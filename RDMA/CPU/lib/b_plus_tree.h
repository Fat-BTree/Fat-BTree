#ifndef YCSB_C_B_PLUS_TREE_H_
#define YCSB_C_B_PLUS_TREE_H_

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
// #include <vector>
#include <mutex>
#include <queue>

#include "lib/fbt_utils.h"
#include "lib/fixed_size_container.h"

using namespace std;

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

  int depth() {
    int ret = 0;
    for (auto p = parent; p != NULL; p = p->parent)
      ++ret;
    return ret;
  }
};

template <uint32_t M> class BPlusTree {
public:
  static const int MAX_NODE_SIZE = 20000000;

  Node pool[MAX_NODE_SIZE];

  int nb_access = 0;
  int nb_query = 0;

  void insert(KEY_TYPE &in_key, VAL_TYPE &in_val) {
    key_map[in_key] += 1;
    // lock_guard<mutex> lock(mutex_);
    Node *cur = root;

    while (!cur->isLeaf) {
      auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
      cur = cur->child[it - cur->key.begin()];
    }

    Node *node_v = &pool[nb_node++];
    node_v->isLeaf = false;
    node_v->isValue = true;
    node_v->parent = cur;
    node_v->val = in_val;

    auto p = lower_bound(cur->key.begin(), cur->key.end(), in_key) -
             cur->key.begin();
    cur->key.insert(cur->key.begin() + p, in_key);
    cur->child.insert(cur->child.begin() + p, node_v);

    // cout << "insert: " << in_key << endl;
    // for (int i = 0; i < in_val.size(); i++) {
    //   cout << "insert: " << in_val[i].first << " " << in_val[i].second <<
    //   endl;
    // }

    overflow(cur);
  }

  // void update(KEY_TYPE in_key, VAL_TYPE in_val, Node *cur = NULL)
  // {
  //   if (cur == NULL)
  //     cur = root;
  //   ++cur->access_count;

  //   while (!cur->isLeaf)
  //   {
  //     auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
  //     cur = cur->child[it - cur->key.begin()];
  //     ++cur->access_count;
  //   }

  //   auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
  //   if (it != cur->key.end())
  //     update_field_table(in_val, cur->val[it - cur->key.begin()]);
  // }

  VAL_TYPE &lookup(KEY_TYPE &in_key, Node *cur = NULL) {
    nb_query += 1;

    // if (cur != NULL) {
    //   cout << in_key << " " << cur << endl;
    // }

    if (cur == NULL)
      cur = root;
    ++cur->access_count;

    while (!cur->isValue) {
      nb_access += 1;

      auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
      cur = cur->child[it - cur->key.begin()];
      ++cur->access_count;
    }

    // auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);

    // if (it == cur->key.end() || *it != in_key) {
    //   if (it == cur->key.end()) {
    //     cout << in_key << " not found" << endl;
    //   } else {
    //     cout << in_key << " not match " << *it << endl;
    //     cout << key_map[in_key] << endl;
    //   }
    // }

    // assert((it != cur->key.end()) && !(*it != in_key));
    // return VAL_TYPE();

    // cout << "insert: " << cur->key[it - cur->key.begin()] << endl;
    // for (int i = 0; i < cur->val[it - cur->key.begin()].size(); i++) {
    //   cout << "insert: " << cur->val[it - cur->key.begin()][i].first << " "
    //        << cur->val[it - cur->key.begin()][i].second << endl;
    // }

    // return cur->val[it - cur->key.begin()];
    return cur->val;
  }

  void load2switch(string filename, uint32_t max_load_num,
                   uint32_t compress_depth) {
    ofstream fout(filename, ios::app);
    vector<vector<Node *>> cached_nodes(depth - 1);
    vector<vector<KEY_TYPE>> cached_keys(compress_depth + 1);

    /* For layer < compress_depth, nodes are compressed into a "fat" node and
    printed */
    cached_nodes[0].push_back(root);

    for (int i = 1; i <= compress_depth; ++i)
      for (int j = 0; j < cached_nodes[i - 1].size(); ++j) {
        cached_keys[i].insert(cached_keys[i].end(),
                              cached_nodes[i - 1][j]->key.begin(),
                              cached_nodes[i - 1][j]->key.end());
        if (j < cached_keys[i - 1].size())
          cached_keys[i].push_back(cached_keys[i - 1][j]);
        cached_nodes[i].insert(cached_nodes[i].end(),
                               cached_nodes[i - 1][j]->child.begin(),
                               cached_nodes[i - 1][j]->child.end());
      }

    print_info(root, 0, cached_keys[compress_depth],
               cached_nodes[compress_depth], fout);
    cout << "Layer 0-" << compress_depth - 1
         << ": access_count = " << root->access_count << endl;

    /* For layer > compress_depth, only frequent-accessed nodes are printed
     */
    sort(cached_nodes[compress_depth].begin(),
         cached_nodes[compress_depth].end(), [](Node *n1, Node *n2) {
           return n1->access_count > n2->access_count;
         });
    if (cached_nodes[compress_depth].size() > max_load_num)
      cached_nodes[compress_depth].resize(max_load_num);

    for (int i = compress_depth + 1; i < depth - 1; ++i) {
      for (auto p : cached_nodes[i - 1])
        for (auto ch : p->child)
          cached_nodes[i].push_back(ch);

      sort(cached_nodes[i].begin(), cached_nodes[i].end(),
           [](Node *n1, Node *n2) {
             return n1->access_count > n2->access_count;
           });
      if (cached_nodes[i].size() > max_load_num)
        cached_nodes[i].resize(max_load_num);
    }

    for (int i = compress_depth; i < depth - 1; ++i) {
      int access_count = 0;
      for (auto n : cached_nodes[i]) {
        print_info(n, i - compress_depth + 1, n->key.to_vector(),
                   n->child.to_vector(), fout);
        access_count += n->access_count;
      }
      cout << "Layer " << i << ": access_count = " << access_count << endl;
    }

    fout.close();
  }

  void load2switch_greedy(string filename, uint32_t root_load_num,
                          uint32_t root_load_depth, uint32_t regular_load_num,
                          uint32_t regular_load_depth = UINT32_MAX) {
    ofstream fout(filename, ios::app);
    vector<vector<Node *>> cached_nodes(depth);
    vector<KEY_TYPE> cached_keys;

    /*
     * For layer < root_load_depth,
     * we greedily select the nodes with the largest access_count, and load them
     * to stage 1
     */
    auto cmp_access = [](Node *&n1, Node *&n2) -> bool {
      return n1->access_count < n2->access_count;
    };
    priority_queue<Node *, vector<Node *>, decltype(cmp_access)> node_pq(
        cmp_access);
    unordered_map<Node *, KEY_TYPE> node_map;

    cout << depth << " " << root_load_depth << endl;

    node_map[root] = KEY_TYPE();
    node_pq.push(root);
    while (node_map.size() + M <= root_load_num && !node_pq.empty()) {
      auto tmp = node_pq.top();
      node_pq.pop();

      for (int i = 0; i < tmp->child.size(); ++i) {
        node_map[tmp->child[i]] = (i == 0 ? node_map[tmp] : tmp->key[i - 1]);
        if (!tmp->child[i]->isLeaf && tmp->child[i]->depth() < root_load_depth)
          node_pq.push(tmp->child[i]);
      }

      node_map.erase(tmp);
    }

    vector<pair<KEY_TYPE, Node *>> node_vec;
    for (auto pk : node_map)
      node_vec.push_back(make_pair(pk.second, pk.first));

    sort(node_vec.begin(), node_vec.end());

    cached_nodes[1].push_back(node_vec[0].second);
    for (int i = 1; i < node_vec.size(); ++i) {
      cached_keys.push_back(node_vec[i].first);
      cached_nodes[1].push_back(node_vec[i].second);
    }
    print_info(root, 0, cached_keys, cached_nodes[1], fout);
    cout << "Layer 0: access_count = " << root->access_count << endl;
    /* The access_count represents the sum in stage 1. Note that some node may
     * be loaded in other stage */

    /* For layer > root_load_depth, only frequent-accessed nodes are printed */
    sort(
        cached_nodes[1].begin(), cached_nodes[1].end(),
        [](Node *n1, Node *n2) { return n1->access_count > n2->access_count; });
    if (cached_nodes[1].size() > regular_load_num)
      cached_nodes[1].resize(regular_load_num);

    int access_count = 0;
    for (auto n : cached_nodes[1]) {
      print_info(n, 1, n->key.to_vector(), n->child.to_vector(), fout);
      access_count += n->access_count;
    }
    cout << "Layer 1: access_count = " << access_count << endl;

    for (int i = 2; i <= min(depth - root_load_depth, regular_load_depth);
         ++i) {
      for (auto p : cached_nodes[i - 1])
        for (auto ch : p->child)
          cached_nodes[i].push_back(ch);

      sort(cached_nodes[i].begin(), cached_nodes[i].end(),
           [](Node *n1, Node *n2) {
             return n1->access_count > n2->access_count;
           });
      if (cached_nodes[i].size() > regular_load_num)
        cached_nodes[i].resize(regular_load_num);

      int access_count = 0;
      for (auto n : cached_nodes[i]) {
        print_info(n, i, n->key.to_vector(), n->child.to_vector(), fout);
        access_count += n->access_count;
      }
      cout << "Layer " << i << ": access_count = " << access_count << endl;
    }

    fout.close();
  }

protected:
  Node *root = &pool[0];
  uint32_t nb_node = 1;
  uint32_t depth = 1;
  // mutex mutex_;

  map<KEY_TYPE, int> key_map;

  void overflow(Node *cur) {
    if (cur->key.size() >= M) {
      if (cur == root || cur->isLoaded2Switch) {
        if (cur == root)
          ++depth;

        /*
         * When we need to split an unmodifiable node cur (means that cur is
         * loaded to the switch), we build a modifiable new node, copy_node, as
         * the child of cur, and move all <key, child/value> of cur to
         * copy_node. Then we split copy_node instead of cur in the following
         * operations. This operation ensures that the address of cur loaded to
         * the switch is still valid.
         */
        Node *copy_node = &pool[nb_node++];
        copy_node->isLeaf = cur->isLeaf;
        copy_node->parent = cur;
        copy_node->key = cur->key;
        copy_node->val = cur->val;
        copy_node->child = cur->child;
        copy_node->isLoaded2Switch = false;

        for (auto it = copy_node->child.begin(); it != copy_node->child.end();
             ++it)
          (*it)->parent = copy_node;

        cur->isLeaf = false;
        cur->key.clear();
        cur->val.clear();
        cur->child.clear();
        cur->child.push_back(copy_node);

        cur = copy_node;
      }

      Node *new_node = &pool[nb_node++];
      new_node->parent = cur->parent;
      new_node->isLeaf = cur->isLeaf;

      if (cur->isLeaf) {
        new_node->key.insert(new_node->key.begin(), cur->key.begin() + M / 2,
                             cur->key.end());
        cur->key.erase(cur->key.begin() + M / 2, cur->key.end());
        new_node->child.insert(new_node->child.begin(),
                               cur->child.begin() + M / 2, cur->child.end());
        cur->child.erase(cur->child.begin() + M / 2, cur->child.end());

        auto p = lower_bound(cur->parent->key.begin(), cur->parent->key.end(),
                             new_node->key[0]) -
                 cur->parent->key.begin();
        cur->parent->key.insert(cur->parent->key.begin() + p, cur->key.back());
        cur->parent->child.insert(cur->parent->child.begin() + p + 1, new_node);
      } else {
        new_node->key.insert(new_node->key.begin(),
                             cur->key.begin() + M / 2 + 1, cur->key.end());
        auto new_key = cur->key[M / 2];
        cur->key.erase(cur->key.begin() + M / 2, cur->key.end());
        new_node->child.insert(new_node->child.begin(),
                               cur->child.begin() + M / 2 + 1,
                               cur->child.end());
        cur->child.erase(cur->child.begin() + M / 2 + 1, cur->child.end());
        for (auto ch : new_node->child)
          ch->parent = new_node;

        auto p = lower_bound(cur->parent->key.begin(), cur->parent->key.end(),
                             new_key) -
                 cur->parent->key.begin();
        cur->parent->key.insert(cur->parent->key.begin() + p, new_key);
        cur->parent->child.insert(cur->parent->child.begin() + p + 1, new_node);
      }

      overflow(cur->parent);
    }
  }

  void print_info(Node *cur, uint32_t virtual_depth,
                  const vector<KEY_TYPE> &keys, const vector<Node *> &childs,
                  ofstream &fout) {
    cur->isLoaded2Switch = true;
    for (auto ch : childs)
      ch->isLoaded2Switch = true;

    if (cur == root)
      fout << "Node: 0x0 ";
    else
      fout << "Node: " << cur << " ";
    fout << "(depth = " << virtual_depth << ")" << endl;
    fout << "keys: <0,0> ";
    for (auto k : keys) {
      uint64_t fbt_key = remove_key_index_prefix(k.to_string());
      uint32_t key_hi = fbt_key >> 32 & 0xffffffff;
      uint32_t key_lo = fbt_key & 0xffffffff;
      fout << "<" << key_hi << "," << key_lo << "> ";
    }
    fout << endl;
    fout << "child: ";
    for (auto c : childs)
      fout << c << " ";
    fout << endl;
  }
};

#endif // YCSB_C_B_PLUS_TREE_DB_H_
