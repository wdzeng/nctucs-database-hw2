#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void read_input_file(int& num_rows, vector<int>& key, vector<int>& value);
void read_key_query_file(int& num_ke_query, vector<int>& query_kes);
void read_range_query_file(int& num_range_query, vector<pair<int,int>>& query_pairs);
void record_time_used(int time_to_build_index, int time_to_query_key, int time_to_query_range);
#endif