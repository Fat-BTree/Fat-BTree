#ifndef YCSB_C_B_PLUS_TREE_DB_H_
#define YCSB_C_B_PLUS_TREE_DB_H_

#include "core/db.h"
#include "driver/driver.h"
#include "driver/rdma_driver.h"
// #include "lib/b_plus_tree.h"

#include <cassert>
#include <iostream>
#include <sched.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// #define CORRECTNESS

using namespace std;

/* copy from b_plus_tree.h, must be a way to include directly*/
const int M = 16;
class Node {
public:
  bool isLeaf = true;
  bool isValue = false;
  Node *parent = NULL;

  fixed_size_array<KEY_TYPE, M> key;
  fixed_size_array<Node *, M> child;
  // fixed_size_array<VAL_TYPE, M> val; // only leaf records val
  VAL_TYPE val; // only leaf records val

  uint32_t access_count = 0;
  bool isLoaded2Switch = false;
};
/* end copy*/

namespace ycsbc {
class BPlusTreeDB : public DB {
public:
  Node remote_node;

  BPlusTreeDB(int nb_threads = 1) : DB() {
    int argc = 5;
    char **argv;
    string *args;

    argv = new char *[argc];
    args = new string[argc];

    args[0] = "frontend";
    args[1] = "--file-prefix";
    args[2] = "frontend";
    args[3] = "-l";
    args[4] = "0-16";

    for (int i = 0; i < argc; i++) {
      argv[i] = (char *)args[i].c_str();
    }

    driver.init(argc, argv, nb_threads);
  }

  void Init();
  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields, std::vector<KVPair> &result);
  int RDMARead(const std::string &table, const std::string &key,
               const std::vector<std::string> *fields,
               std::vector<KVPair> &result);
  int Scan(const std::string &table, const std::string &key, int len,
           const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result);
  int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values);
  int Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values);
  int Delete(const std::string &table, const std::string &key);
  int Load2Switch();
  int PrintStatisticsInfo();

protected:
  int64_t root_idx;
  dpdk_driver driver;
  rdma_client rclient;

  mutex mutex_;
  int init_count = 0;
  vector<double> latency;
  vector<double> read_count;

#ifdef CORRECTNESS
  unordered_map<string, vector<KVPair>> ground_truth;
#endif
};

} // namespace ycsbc

#endif // YCSB_C_B_PLUS_TREE_DB_H_
