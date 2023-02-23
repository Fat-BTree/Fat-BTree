#include "driver/driver.h"
#include "lib/b_plus_tree.h"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

// #define CORRECTNESS
// #define STORE_INSERT_MSG

using namespace std;

BPlusTree<16> b_plus_tree;
dpdk_driver driver;
vector<string> node_info_txt = { "node_info_0.txt", "node_info_1.txt", "node_info_2.txt", "node_info_3.txt" };
const int root_load_num = 150;
const int max_load_num = 800;
const int compress_depth = 2;

void exit_handler(int signo) {
  cout << endl << "nb_lookup: " << b_plus_tree.nb_query << endl;
  cout << endl << "nb_access: " << b_plus_tree.nb_access << endl;
  exit(0);
}

#ifdef STORE_INSERT_MSG
ofstream msg_out("insert_msg.txt");
#endif

void handle_insert(char *msg, uint16_t dst_qid) {
  std::string key_index;
  std::vector<std::pair<std::string, std::string>> values;

  msg += extract_key_index(msg, key_index);
  msg += extract_values(msg, values);

#ifdef STORE_INSERT_MSG
  msg_out << key_index << endl;
  for (auto kv : values)
    msg_out << kv.first << " " << kv.second << endl;
#endif

  b_plus_tree.insert(key_index, values);

  char *ret = new char[1472];
  unsigned msg_len = 0;

  msg_len += inject_msg_type(ret + msg_len, 0x0528, 0);

  driver.send_pkt(ret, msg_len, dst_qid, 0);

  delete[] ret;
}

void handle_update(char *msg, uint16_t dst_qid) {
  int64_t fbt_idx;
  std::string key_index;
  std::vector<std::pair<std::string, std::string>> values;

  msg += extract_fbt_hdr(msg, fbt_idx);
  msg += extract_key_index(msg, key_index);
  msg += extract_values(msg, values);

  b_plus_tree.update(key_index, values, fbt_idx);

  char *ret = new char[1472];
  unsigned msg_len = 0;

  msg_len += inject_msg_type(ret + msg_len, 0x0530, 0);

  driver.send_pkt(ret, msg_len, dst_qid, 0);

  delete[] ret;
}

void handle_query(char *msg, uint16_t dst_qid, int mode) {
  int64_t fbt_idx;
  std::string key_index;
  msg += extract_fbt_hdr(msg, fbt_idx);
  msg += extract_key_index(msg, key_index);

  if (mode == 2) {
    cout << key_index << " " << hex << fbt_idx << endl;
  }

  if (mode == 0) {
    fbt_idx = 0; 
  }

  // std::vector<std::pair<std::string, std::string>> values;
  std::vector<std::pair<std::string, std::string>> &values =
      b_plus_tree.lookup(key_index, fbt_idx);

#ifdef CORRECTNESS
  std::vector<std::pair<std::string, std::string>> basic_values =
      b_plus_tree.lookup(key_index);

  if (values != basic_values) {
    cout << key_index << endl;
    cout << "fbt_idx" << hex << fbt_idx << endl;
    cout << "ours results:" << endl;
    for (auto kv : values)
      cout << "<" << kv.first << "," << kv.second << ">" << endl;
    cout << "basic results:" << endl;
    for (auto kv : basic_values)
      cout << "<" << kv.first << "," << kv.second << ">" << endl;
    cout << endl;
  }
#endif

  char *ret = new char[1472];
  unsigned msg_len = 0;

  msg_len += inject_msg_type(ret + msg_len, 0x0529, 0);
  msg_len += inject_values(ret + msg_len, values);

  driver.send_pkt(ret, msg_len, dst_qid, 0);

  delete[] ret;

  // usleep(10);
}

void handle_load2switch() {
  // b_plus_tree.load2switch(node_info_txt, max_load_num, compress_depth);
  // cout << "load2switch" << endl;

  b_plus_tree.load2switch_greedy(node_info_txt, root_load_num, max_load_num, compress_depth, 1ULL << 46);
  cout << "load2switch" << endl;

  // string str;
  // while (true)
  // {
  //   cin >> str;
  //    if (str == "done")
  //      break;
  // }

  char *ret = new char[1472];
  unsigned msg_len = 0;

  msg_len += inject_msg_type(ret + msg_len, 0x601, 0);
  driver.send_pkt(ret, msg_len, 0, 0);
}

int main(int argc, char *argv[]) {

  struct sigaction new_sa, old_sa;

  sigemptyset(&new_sa.sa_mask);
  new_sa.sa_flags = 0;
  new_sa.sa_handler = exit_handler;
  sigaction(SIGINT, &new_sa, &old_sa);

  int mode = atoi(argv[argc - 1]);
	int port = atoi(argv[argc - 2]);

  driver.init(argc - 1, argv, 1, port);

  char *msg = new char[1472];
  unsigned msg_type;
  unsigned msg_len;
  uint16_t recv_no;

  while (true) {
    msg_len = 0;

    driver.recv_pkt(msg, 1472, recv_no, 0);

    // for (int i = 0; i < 100; i++)
    // {
    //   printf("%02x ", (unsigned)msg[i] & 0xff);
    //   if (i % 4 == 3)
    //     cout << endl;
    // }
    // cout << endl;

    msg_len += extract_msg_type(msg, msg_type);

    // cout << "msg_type: " << hex << msg_type << endl;

    switch (msg_type) {
    case 0x0526:
      handle_insert(msg + msg_len, recv_no);
      break;
    case 0x0527:
      handle_query(msg + msg_len, recv_no, mode);
      break;
    case 0x0531:
      handle_update(msg + msg_len, recv_no);
      break;
    case 0x0600:
      handle_load2switch();
      b_plus_tree.nb_query = 0;
      b_plus_tree.nb_access = 0;
      break;
    default:
      printf("Warning: Undefined msg type %x\n", msg_type);
      break;
    }
  }
  delete[] msg;
}
