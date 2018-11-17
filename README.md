# K-means-Custering
K means clustering of tweets using Jaccard Distance
The folder has the following structure:

./

├── InitialSeeds.txt

├── k_means.cpp

├── nlohmann

│   └── json.hpp

├── output.txt

├── ReadME.md

├── Tweets.json




The InitialSeeds.txt is the initial seed file. 

Tweets.json is tweet file used

k_means.cpp is the actual source file

tweets-k-means is the executable generated after comiplation [It can be any file the user specifies]

nlohmann/json.hpp is an open source header with Json parsing utility

output.txt is the result file that contains the clustering results. Each line represents a cluster. Also includes the overall SSE value. [It can be any file the user specifies]

Command to Compile the code:
--------------------------------------------------------
$ g++ k_means.cpp -o tweets-k-means


Command to Execute the code
--------------------------------------------------------
tweets-k-means <numberOfClusters> <initialSeedsFile><TweetsDataFile> <outputFile>

For example: 
$  ./tweets-k-means 25 InitialSeeds.txt Tweets.json ouput.txt
