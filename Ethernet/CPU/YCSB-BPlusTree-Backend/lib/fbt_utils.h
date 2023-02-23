#ifndef FBT_UTILS_H_
#define FBT_UTILS_H_

#include <vector>
#include <string>
#include <sstream>

using namespace std;

uint64_t remove_key_index_prefix(string key) // key_index : usertable + user + uint64 -> uint64
{
  key = key.substr(13);
  stringstream ss(key);
  uint64_t ret;
  ss >> ret;
  return ret;
}

void update_field_table(const vector<pair<string, string>> &update_field_pairs, vector<pair<string, string>> &field_table)
{
    for (auto update_kv : update_field_pairs)
    {
        bool kv_exist = false;
        for (auto kv : field_table)
            if (kv.first == update_kv.first)
            {
                kv.second = update_kv.second;
                kv_exist = true;
                break;
            }
        if (!kv_exist)
            field_table.push_back(update_kv);
    }
}

#endif // FBT_UTILS_H_