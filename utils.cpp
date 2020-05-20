#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "utils.h"

using namespace std;

void read_input_file(int& num_rows, vector<int>& key, vector<int>& value)
{
    //readfile
    fstream file;
    file.open("data.txt");
    string line;

    while(getline(file, line, '\n'))
    {
        istringstream templine(line);
        string data;
        getline(templine, data,',');
        key.push_back(atoi(data.c_str()));
        getline(templine, data,',');
        value.push_back(atoi(data.c_str()));
        num_rows += 1;
    }
    file.close();
    cout << "Data file reading complete, " << num_rows << " rows loaded."<< endl;
    return;
}

void read_key_query_file(int& num_key_query, vector<int>& query_keys)
{
    //readfile
    fstream file;
    file.open("key_query.txt");
    string line;

    while(getline(file, line, '\n'))
    {
        istringstream templine(line);
        string data;
        getline(templine, data, ',');
        query_keys.push_back(atoi(data.c_str()));
        num_key_query += 1;
    }
    file.close();
    cout << "Key query file reading complete, " << num_key_query << " queries loaded."<< endl;
    return;
}

void read_range_query_file(int& num_range_query, vector<pair<int,int>>& query_pairs)
{
    //readfile
    fstream file;
    file.open("range_query.txt");
    string line;

    while(getline(file, line, '\n'))
    {
        istringstream templine(line);
        string data1, data2;
        getline(templine, data1,',');
        getline(templine, data2,',');
        query_pairs.push_back({atoi(data1.c_str()),atoi(data2.c_str())});
        num_range_query += 1;
    }
    file.close();
    cout << "Range query file reading complete, " << num_range_query << " queries loaded."<< endl;
    return;
}

void record_time_used(int time_to_build_index, int time_to_query_key, int time_to_query_range)
{
    ofstream file ("time_used.txt", std::ios_base::app);
    if(file.is_open())
    {
        file << time_to_build_index << "," << time_to_query_key << "," << time_to_query_range << endl;
        file.close();
    }
}