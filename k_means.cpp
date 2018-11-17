#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include<iterator>
#include <functional>
#include <cfloat>
#include <limits>
#include <cstdlib>
#include <ostream>

#define min(a,b) ((a) < (b) ? (a) : (b))

using json = nlohmann::json;
using namespace std;

typedef vector<pair<string /* ID */, string  /* Text */>> cluster_t;
map<string /* ID */, cluster_t> clusters; 
map<string /* ID */, string /* Text */> tweets;

void erase_endline(string &str){
    size_t pos;
    while((pos = str.find('\n'))  != string::npos) {
        str.replace(pos, 1, 1, ' ');
    }
}
double calc_jaccard_dist(string a, string b) {
    int num_words_a = 0, num_words_b = 0, matched_words = 0;
    stringstream s1,s2;
    string str;
    map<string, int> temp;
    double ret;
    
    s1<<a;
    while(getline(s1, str, ' ')) {
        temp[str] = 1;
        num_words_a++;
        str.clear();
    }
    str.clear();
    s2<<b;
     while(getline(s2, str, ' ')) {
        if(temp.find(str) != temp.end()) {
            matched_words++;
        }
        num_words_b++;
        str.clear();
    }
    
    ret = (double)matched_words/(num_words_a + num_words_b - matched_words);
    //cout<<"Num MAtched words = "<<matched_words<<" "<<num_words_a<<", "<<num_words_b<<", "<<ret<<endl;
    
    return ((double)1 - ret);
}

void recompute_centroid(vector<string> &centroids){
    map<string, cluster_t>::iterator it;
    //vector<pair<string, double>> distance_list;
    
    centroids.clear();
    for(it = clusters.begin(); it != clusters.end(); it++) {
        int len = it->second.size();
        double min_sum_distance = DBL_MAX;
        string new_centroid_id;
        
        if(!len)
            continue;
        double distance_list[len][len];
        
        for(int i = 0; i < len; i++) {
            double sum_distance = 0;
            for(int j = 0; j < len; j++) {
                double dist;
                
                if(j == i)
                    continue;
                if(j > i) {
                    dist = calc_jaccard_dist((it->second[i]).second, (it->second[j]).second);
                    distance_list[i][j] = dist;
                    distance_list[j][i] = dist;
                } else {
                    dist = distance_list[i][j];
                }
                sum_distance += dist;
            }
            if(sum_distance < min_sum_distance) {
                min_sum_distance = sum_distance;
                new_centroid_id = (it->second[i]).first;
            }
        }
        centroids.push_back(new_centroid_id);
    }
}

void perform_clustering(vector<string> centroids) {
    int num_centroids = centroids.size(), num_tweets = tweets.size();
    
    map<string, string>::iterator it;
    
    clusters.clear();
    for(it = tweets.begin(); it != tweets.end(); it++) {
        string min_id;
        double min_dist = DBL_MAX;
        for(int j = 0 ; j < num_centroids; j++) {
            if(tweets.find(centroids[j]) != tweets.end()) {
                double dist;
                
                dist = calc_jaccard_dist(tweets[centroids[j]], it->second);
                if(dist < min_dist) {
                    min_dist = dist;
                    min_id = centroids[j];
                }
            } else {
                cout<<"ERROR: Centroid does not exist"<<endl;
            }
        }
        clusters[min_id].push_back(make_pair(it->first, it->second));
    }
}

bool check_if_centroids_same(vector<string> s1, vector<string> s2) {
    vector<string>::iterator it;
    size_t hsh = 0;
    
    
    if(s1.size() != s2.size())
        return false;
    for(int i = 0; i < s1.size(); i++) {
        hsh ^= hash<string>{}(s1[i]);
        hsh ^= hash<string>{}(s2[i]);
    }
    
    return ((hsh == 0)? true : false);
}

double calc_SSE() {
    map<string, cluster_t>::iterator it;
    double sse = 0;
    
    for(it = clusters.begin(); it != clusters.end(); it++) {
        string centroid_text = tweets[it->first];
        for(int i = 0; i < it->second.size(); i++) {
            double jacc_dist = calc_jaccard_dist(centroid_text, (it->second[i]).second);
            sse += (jacc_dist * jacc_dist);
        }
    }
    return sse;
}

void print_cluster_data(const char *ofilename) {
    map<string, cluster_t>::iterator it;
    fstream outfile;
    int i = 0;
    
    outfile.open(ofilename, fstream::out);
    outfile<<"Printing Data from "<<clusters.size()<<" clusters"<<endl;
    
    for(it = clusters.begin(); it != clusters.end(); it++) {
        outfile<<"Cluster ID "<<++i<<" [Centroid "<<it->first <<"]:: ";
        for(int i = 0; i < it->second.size(); i++) {
            outfile<<(it->second[i]).first<<", ";
        }
        outfile<<endl<<endl;
    }
    outfile<<"\nThe Sum of Squared Error is "<<calc_SSE()<<endl;
    outfile.close();
}

void run_k_means(vector<string> centroids, const char *ofilename) {
    vector<string> old_centroids;
    int num_iter = 0;
    do {
        old_centroids = centroids;
        perform_clustering(centroids);
        recompute_centroid(centroids);
        num_iter++;
    }while(!check_if_centroids_same(old_centroids, centroids));
    
    print_cluster_data(ofilename);
    cout<<"It took "<<num_iter <<" iterations for the algorithm to converge"<<endl;
}

int main(int argc, const char *argv[]) {

    vector<string> centroids;
    ifstream strm;
    json j;
    int num_tweets = 0, num_clusters = 0;
    
    if(argc < 5) {
        cout<<"Usage::"<<endl<<"    tweets-k-means <numberOfClusters> <initialSeedsFile><TweetsDataFile> <outputFile>"<<endl;
        exit(-1);
    }
    
    num_clusters = atoi(argv[1]);
    strm.open(/* Tweets file */argv[3], ifstream::in);
    while(strm.good()) {
        try{
            strm >> j;
            string tweet_txt;
            
            tweet_txt = j["text"];
            erase_endline(tweet_txt);
            tweets[j["id_str"]] = tweet_txt;
            //cout<<tweet.text<<endl;
            num_tweets++;
        } catch (json::parse_error& e) {
                if(e.id == 101)
                    cout<<"Done reading "<<num_tweets<<" tweets"<<endl;
                else {
                    cout<<"Error in reading JSON file";
                }
        }
    }
    strm.close();
    
    strm.open(/* Initial seeds file */argv[2], ifstream::in);
     while(strm.good()) {
        string seed;
        getline(strm, seed);
        if(seed.back() == ',')
            seed.pop_back();
        centroids.push_back(seed);
     }
    strm.close();
    if(num_clusters != centroids.size()){
        cout<<"Number of Clusters not the same as in seed file"<<endl;
    }
    run_k_means(centroids, argv[4]);
    //cout<<calc_jaccard_dist("the long march","ides of march")<<endl;
    return 0;
}

