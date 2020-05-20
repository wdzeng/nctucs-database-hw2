#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "utils.h"
#include <chrono>
//Uncomment the line below to include your header file
#include "index.h"


using namespace std;

int main()
{
    vector<int> key;
    vector<int> value;
    vector<int> query_keys;
    vector<pair<int,int>> query_pairs;
    int num_rows = 0;
    int num_key_query = 0;
    int num_range_query = 0;
    read_input_file(num_rows, key, value);
    read_key_query_file(num_key_query, query_keys);
    read_range_query_file(num_range_query, query_pairs);
    
    //Uncomment to test your index
    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    //Build index when index constructor is called
    Index myIndex(num_rows, key, value);
    chrono::steady_clock::time_point built_index = chrono::steady_clock::now();
    //Query by key
    myIndex.key_query(query_keys);
    chrono::steady_clock::time_point key_query = chrono::steady_clock::now();
    //Query by range of key
    myIndex.range_query(query_pairs);
    chrono::steady_clock::time_point range_query = chrono::steady_clock::now();
    //Free memory
    //myIndex.clear_index();
    
    auto time_to_build_index = chrono::duration_cast<chrono::microseconds>(built_index - start).count();
    auto time_to_query_key = chrono::duration_cast<chrono::microseconds>(key_query - built_index).count();
    auto time_to_query_range = chrono::duration_cast<chrono::microseconds>(range_query - key_query).count();
    record_time_used(int(time_to_build_index), int(time_to_query_key), int(time_to_query_range));
    return 0;
}
