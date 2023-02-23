#include "db/b_plus_tree_db.h"
#include "lib/fbt_utils.h"
#include <fstream>
#include <thread>

using namespace std;
using ycsbc::BPlusTreeDB;
using ycsbc::DB;

void BPlusTreeDB::Init() {
  lock_guard<mutex> lock(mutex_);
  ++init_count;

  driver.register_queue_id();
  driver.register_dest_port();

	// flow init only first Init call
  if (init_count <= 16)
    driver.flow_init(driver.get_queue_id(), driver.get_dest_port());

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(driver.get_dest_port() + 1, &cpuset);
  if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
    printf("Warning: failed to bind %d-th thread to %d-th CPU core\n",
           driver.get_dest_port(), driver.get_dest_port() + 1);
  }

  cout << "A new thread begins working: " << std::this_thread::get_id() << endl;
}

int BPlusTreeDB::Read(const std::string &table, const std::string &key,
                      const std::vector<std::string> *fields,
                      std::vector<KVPair> &result) {
  string key_index(table + key);

  char msg[1472];
  unsigned msg_len = 0;
  unsigned msg_type = 0;
  uint16_t queue_id = driver.get_queue_id();
  uint16_t dest_port = driver.get_dest_port();

  msg_len += inject_msg_type(msg + msg_len, 0x0527, get_part_id(key_index));
  msg_len += inject_fbt_hdr(msg + msg_len, remove_key_index_prefix(key_index));
  msg_len += inject_key_index(msg + msg_len, key_index);

  driver.send_pkt(msg, msg_len, dest_port, queue_id);

	do {
		driver.recv_pkt(msg, 1472, dest_port, queue_id);

		msg_len = 0;
		msg_len += extract_msg_type(msg + msg_len, msg_type);
		if (msg_type != 0x0529) {
			cout << "msg_type: " << hex << msg_type << endl;
			assert(msg_type == 0);
		}
		msg_len += extract_values(msg + msg_len, result);
	} while (msg_type == 0);

#ifdef CORRECTNESS
  auto gt_result = ground_truth[key_index];
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

  char msg[1472];
  unsigned msg_len = 0;
  unsigned msg_type = 0;
  uint16_t queue_id = driver.get_queue_id();
  uint16_t dest_port = driver.get_dest_port();

  msg_len += inject_msg_type(msg + msg_len, 0x0531, get_part_id(key_index));
  msg_len += inject_fbt_hdr(msg + msg_len, remove_key_index_prefix(key_index));
  msg_len += inject_key_index(msg + msg_len, key_index);
  msg_len += inject_values(msg + msg_len, values);
  driver.send_pkt(msg, msg_len, dest_port, queue_id);

	do {
		driver.recv_pkt(msg, 1472, dest_port, queue_id);
		msg_len = 0;
		msg_len += extract_msg_type(msg + msg_len, msg_type);
		if (msg_type != 0x0530) {
			cout << "msg_type: " << hex << msg_type << endl;
			assert(msg_type == 0);
		}
	} while(msg_type == 0);

#ifdef CORRECTNESS
  lock_guard<mutex> lock(mutex_);
  update_field_table(values, ground_truth[key_index]);
#endif

  return DB::kOK;
}

int BPlusTreeDB::Insert(const std::string &table, const std::string &key,
                        std::vector<KVPair> &values) {
  string key_index(table + key);
  string recv_key_index;

  char msg[1472];
  unsigned msg_len = 0;
  unsigned msg_type = 0;
  uint16_t queue_id = driver.get_queue_id();
  uint16_t dest_port = driver.get_dest_port();

#ifdef CORRECTNESS
  // cout << "send packet from queue " << queue_id << ", msg = " << key_index
  //      << endl;
#endif

  msg_len += inject_msg_type(msg + msg_len, 0x0526, get_part_id(key_index));
  msg_len += inject_key_index(msg + msg_len, key_index);
  msg_len += inject_values(msg + msg_len, values);
  driver.send_pkt(msg, msg_len, dest_port, queue_id);

	do {
		driver.recv_pkt(msg, 1472, dest_port, queue_id);

		msg_len = 0;
		msg_len += extract_msg_type(msg + msg_len, msg_type);
		if (msg_type != 0x0528) {
			cout << "msg_type: " << hex << msg_type << endl;
			assert(msg_type == 0);
		}
	} while(msg_type == 0);

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
  char *msg;
  unsigned msg_len;
  unsigned msg_type;
  uint16_t recv_no;
	int part_id;

	for (part_id = 0; part_id < 4; part_id += 1) {
		char msg[1472];
		msg_len = 0;

		msg_len += inject_msg_type(msg + msg_len, 0x0600, part_id);
		driver.send_pkt(msg, msg_len, 0, 0);

		msg_type = 0;
		do {
			driver.recv_pkt(msg, 1472, recv_no, 0);
			msg_len = 0;
			msg_len += extract_msg_type(msg + msg_len, msg_type);
			if (msg_type != 0x0601) {
				cout << "msg_type: " << hex << msg_type << endl;
				assert(msg_type == 0);
			}
		} while(msg_type == 0);

	}

  cout << "load2switch" << endl;
  string str;
  while (true)
  {
    cin >> str;
     if (str == "done")
       break;
  }

  return DB::kOK;
}
