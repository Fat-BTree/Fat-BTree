#include "db/b_plus_tree_db.h"
#include "lib/fbt_utils.h"
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;
using ycsbc::BPlusTreeDB;
using ycsbc::DB;

void BPlusTreeDB::Init() {
  lock_guard<mutex> lock(mutex_);
  ++init_count;

  driver.register_queue_id();
  driver.register_dest_port();

  if (init_count <= driver.nb_queues)
    driver.flow_init(driver.get_queue_id(), driver.get_dest_port());

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(driver.get_dest_port() + 1, &cpuset);
  if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
    printf("Warning: failed to bind %d-th thread to %d-th CPU core\n",
           driver.get_dest_port(), driver.get_dest_port() + 1);
  }

  cout << "A new thread begins working." << endl;
}

int BPlusTreeDB::RDMARead(const std::string &table, const std::string &key,
                          const std::vector<std::string> *fields,
                          std::vector<KVPair> &result) {

  KEY_TYPE key_index(table + key);

  steady_clock::time_point t1 = steady_clock::now();
  int count = 1;

  int ret =
      rclient.read_remote_node(NULL, remove_key_index_prefix(table + key));

  while (!remote_node.isValue) {
    auto it =
        lower_bound(remote_node.key.begin(), remote_node.key.end(), key_index);
    rclient.read_remote_node(remote_node.child[it - remote_node.key.begin()]);
    ++count;
  }

  for (auto kv : remote_node.val) {
    result.push_back(make_pair(string(kv.first.begin(), kv.first.size()),
                               string(kv.second.begin(), kv.second.size())));
  }

  steady_clock::time_point t2 = steady_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
  latency.push_back(time_span.count());
  read_count.push_back(count);

#ifdef CORRECTNESS
  auto gt_result = ground_truth[table + key];
  if (result != gt_result) {
    cout << key_index << endl;
    cout << "result:" << endl;
    for (auto kv : result)
      cout << "<" << kv.first << "," << kv.second << ">" << endl;
    cout << "ground truth:" << endl;
    for (auto kv : gt_result)
      cout << "<" << kv.first << "," << kv.second << ">" << endl;
    cout << endl;
  } else {
    // cout << "correct." << endl;
  }
#endif

  return DB::kOK;
}

int BPlusTreeDB::Read(const std::string &table, const std::string &key,
                      const std::vector<std::string> *fields,
                      std::vector<KVPair> &result) {

  string key_index(table + key);

  char *msg = new char[1472];
  unsigned msg_len = 0;
  unsigned msg_type;
  uint16_t queue_id = driver.get_queue_id();
  uint16_t dest_port = driver.get_dest_port();

  msg_len += inject_msg_type(msg + msg_len, 0x0527);
  msg_len += inject_fbt_hdr(msg + msg_len, remove_key_index_prefix(key_index));
  msg_len += inject_key_index(msg + msg_len, key_index);
  driver.send_pkt(msg, msg_len, dest_port, queue_id);

  driver.recv_pkt(msg, 1472, dest_port, queue_id);
  msg_len = 0;
  msg_len += extract_msg_type(msg + msg_len, msg_type);
  assert(msg_type == 0x0529);
  msg_len += extract_values(msg + msg_len, result);

#ifdef CORRECTNESS
  auto gt_result = ground_truth[table + key];
  if (result != gt_result) {
    cout << key_index << endl;
    cout << "result:" << endl;
    for (auto kv : result)
      cout << "<" << kv.first << "," << kv.second << ">" << endl;
    cout << "ground truth:" << endl;
    for (auto kv : gt_result)
      cout << "<" << kv.first << "," << kv.second << ">" << endl;
    cout << endl;
  } else {
    // cout << "correct." << endl;
  }
#endif

  return DB::kOK;
}

int BPlusTreeDB::Scan(const std::string &table, const std::string &key, int len,
                      const std::vector<std::string> *fields,
                      std::vector<std::vector<KVPair>> &result) {
  throw "Scan: function not implemented!";
}

int BPlusTreeDB::Update(const std::string &table, const std::string &key,
                        std::vector<KVPair> &values) {
  string key_index(table + key);

  char *msg = new char[1472];
  unsigned msg_len = 0;
  unsigned msg_type;
  uint16_t queue_id = driver.get_queue_id();
  uint16_t dest_port = driver.get_dest_port();

  msg_len += inject_msg_type(msg + msg_len, 0x0531);
  msg_len += inject_fbt_hdr(msg + msg_len, remove_key_index_prefix(key_index));
  msg_len += inject_key_index(msg + msg_len, key_index);
  msg_len += inject_values(msg + msg_len, values);
  driver.send_pkt(msg, msg_len, dest_port, queue_id);

  driver.recv_pkt(msg, 1472, dest_port, queue_id);
  msg_len = 0;
  msg_len += extract_msg_type(msg + msg_len, msg_type);
  assert(msg_type == 0x0530);

#ifdef CORRECTNESS
  lock_guard<mutex> lock(mutex_);
  update_field_table(values, ground_truth[key_index]);
#endif

  return DB::kOK;
}

int BPlusTreeDB::Insert(const std::string &table, const std::string &key,
                        std::vector<KVPair> &values) {
  string key_index(table + key);
  // string recv_key_index;

  char *msg = new char[1472];
  unsigned msg_len = 0;
  unsigned msg_type;
  uint16_t queue_id = driver.get_queue_id();
  uint16_t dest_port = driver.get_dest_port();

#ifdef CORRECTNESS
  // cout << "send packet from queue " << queue_id << ", msg = " << key_index
  //      << endl;
#endif

  msg_len += inject_msg_type(msg + msg_len, 0x0526);
  msg_len += inject_key_index(msg + msg_len, key_index);
  msg_len += inject_values(msg + msg_len, values);
  driver.send_pkt(msg, msg_len, dest_port, queue_id);

  driver.recv_pkt(msg, 1472, dest_port, queue_id);
  msg_len = 0;
  msg_len += extract_msg_type(msg + msg_len, msg_type);
  assert(msg_type == 0x0528);
  // msg_len += extract_key_index(msg + msg_len, recv_key_index);

  // cout << "recv packet from queue " << queue_id << ", msg_type = " <<
  // msg_type << endl;

  // if (key_index != recv_key_index)
  //   cout << "Error: " << key_index << " " << recv_key_index << endl;
  // else
  //   cout << "Correct: " << key_index << " " << recv_key_index << endl;

#ifdef CORRECTNESS
  lock_guard<mutex> lock(mutex_);
  ground_truth[key_index] = values;
#endif

  return DB::kOK;
}

int BPlusTreeDB::Delete(const std::string &table, const std::string &key) {
  throw "Delete: function not implemented!";
}

int BPlusTreeDB::Load2Switch() {
  char *msg = new char[1472];
  unsigned msg_len = 0;
  unsigned msg_type;
  uint16_t recv_no;

  msg_len += inject_msg_type(msg + msg_len, 0x0600);
  driver.send_pkt(msg, msg_len, 0, 0);

  driver.recv_pkt(msg, 1472, recv_no, 0);
  msg_len = 0;
  msg_len += extract_msg_type(msg + msg_len, msg_type);
  assert(msg_type == 0x0601);

  usleep(1000000);

  int rdma_ret = rclient.rdma_client_setup(string("10.0.0.1").c_str(),
                                           &remote_node, sizeof(remote_node));
  assert(rdma_ret == 0);

  string str;
  while (true) {
    cin >> str;
    if (str == "done")
      break;
  }

  return DB::kOK;
}

int BPlusTreeDB::PrintStatisticsInfo() {
  string file1 = "latency.txt";
  string file2 = "read_count.txt";
  double totl = 0, totc = 0;

  ofstream fout1(file1);
  for (auto l : latency)
  {
    fout1 << l << endl;
    totl += l;
  }
  fout1.close();

  ofstream fout2(file2);
  for (auto c : read_count)
  {
    fout2 << c << endl;
    totc += c;
  }
  fout2.close();

  cerr << "# average latency: " << totl / latency.size() << endl;
  cerr << "# average read_count: " << totc / read_count.size() << endl;

  return DB::kOK;
}
