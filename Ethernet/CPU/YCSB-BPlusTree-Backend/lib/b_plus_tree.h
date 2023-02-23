#ifndef YCSB_C_B_PLUS_TREE_H_
#define YCSB_C_B_PLUS_TREE_H_

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <mutex>

#include "lib/fbt_utils.h"

using namespace std;

typedef string KEY_TYPE;
typedef vector<pair<string, string>> VAL_TYPE;

class Node
{
public:
  bool isLeaf = true;
  bool isValue = false;

  Node *parent = NULL;
  vector<KEY_TYPE> key;
  // vector<VAL_TYPE> val; // only leaf records val
  VAL_TYPE val;
  vector<Node *> child;

  uint32_t access_count = 0;
  bool isLoaded2Switch = false;

  int depth()
  {
    int ret = 0;
    for (auto p = parent; p != NULL; p = p->parent)
      ++ret;
    return ret;
  }
};

template <uint32_t M>
class BPlusTree
{
public:
  uint32_t nb_query = 0;
  uint32_t nb_access = 0;

	BPlusTree() {
		pool = new Node[MAX_NODE_SIZE];
		root = pool;
	}

  void insert(KEY_TYPE &in_key, VAL_TYPE &in_val)
  {
    Node *value_node = &pool[nb_node++];

    // lock_guard<mutex> lock(mutex_);
    Node *cur = root;

    while (!cur->isLeaf)
    {
      auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
      cur = cur->child[it - cur->key.begin()];
    }

    auto p = lower_bound(cur->key.begin(), cur->key.end(), in_key) -
             cur->key.begin();
    cur->key.insert(cur->key.begin() + p, in_key);
    // cur->val.insert(cur->val.begin() + p, in_val);
    cur->child.insert(cur->child.begin() + p, value_node);

    value_node->isLeaf = false;
    value_node->isValue = true;
		value_node->parent = cur;
		update_field_table(in_val, value_node->val);

    overflow(cur);
  }

  void update(KEY_TYPE &in_key, VAL_TYPE &in_val, uint64_t bias = 0)
  {
		Node *cur = (Node *)((uint64_t)root + (bias & 0xFFFFFFFFULL));
    ++cur->access_count;

    // while (!cur->isLeaf)
    // {
    //   auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
    //   cur = cur->child[it - cur->key.begin()];
    //   ++cur->access_count;
    // }

    // auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
    // if (it != cur->key.end())
    //   update_field_table(in_val, cur->val[it - cur->key.begin()]);

    while (!cur->isValue)
    {
      auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
      cur = cur->child[it - cur->key.begin()];
      ++cur->access_count;
    }

		update_field_table(in_val, cur->val);
  }

  VAL_TYPE &lookup(KEY_TYPE &in_key, uint64_t bias = 0)
  {
		Node *cur = (Node *)((uint64_t)root + (bias & 0xFFFFFFFFULL));
    nb_query += 1;

    if (cur == NULL)
      cur = root;
    ++cur->access_count;

    /*cout << cur << endl;
    for (auto k : cur->key)
	    cout << k <<" ";
    cout <<endl;
    */
    // while (!cur->isLeaf)
    // {
    //   auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
    //   cur = cur->child[it - cur->key.begin()];
    //   ++cur->access_count;
    //   nb_access += 1;
    //   /*cout << cur << endl;
    //   for (auto k : cur->key)
	  //     cout << k << " ";
    //   cout <<endl;
    //   */
    // }

    // /*for (auto k :cur->key)
	  //   cout << k << " ";
    // cout <<endl;
    // cout << "------------------" << endl;
    // */

    // // cout << cur << " " << in_key << endl;
    // auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
    // // cout << (it == cur->key.end()) << endl << endl;

    // if (it == cur->key.end() || *it != in_key)
    //   return VAL_TYPE();
    // return cur->val[it - cur->key.begin()];
		
    while (!cur->isValue)
    {
			nb_access += 1;

      auto it = lower_bound(cur->key.begin(), cur->key.end(), in_key);
      cur = cur->child[it - cur->key.begin()];
      ++cur->access_count;
    }

    return cur->val;
  }

  void load2switch(string filename, uint32_t max_load_num, uint32_t compress_depth)
  {
    ofstream fout(filename);
    vector<vector<Node *>> cached_nodes(depth - 1);
    vector<vector<KEY_TYPE>> cached_keys(compress_depth + 1);

    /* For layer < compress_depth, nodes are compressed into a "fat" node and printed */
    cached_nodes[0].push_back(root);

    for (int i = 1; i <= compress_depth; ++i)
      for (int j = 0; j < cached_nodes[i - 1].size(); ++j)
      {
        cached_keys[i].insert(cached_keys[i].end(), cached_nodes[i - 1][j]->key.begin(), cached_nodes[i - 1][j]->key.end());
        if (j < cached_keys[i - 1].size())
          cached_keys[i].push_back(cached_keys[i - 1][j]);
        cached_nodes[i].insert(cached_nodes[i].end(), cached_nodes[i - 1][j]->child.begin(), cached_nodes[i - 1][j]->child.end());
      }

    print_info(root, 0, cached_keys[compress_depth], cached_nodes[compress_depth], fout);
    cout << "Layer 0-" << compress_depth - 1 << ": access_count = " << root->access_count << endl;

    /* For layer > compress_depth, only frequent-accessed nodes are printed */
    sort(cached_nodes[compress_depth].begin(), cached_nodes[compress_depth].end(), [](Node *n1, Node *n2)
         { return n1->access_count > n2->access_count; });
    if (cached_nodes[compress_depth].size() > max_load_num)
      cached_nodes[compress_depth].resize(max_load_num);

    for (int i = compress_depth + 1; i <= depth - 1; ++i)
    {
      for (auto p : cached_nodes[i - 1])
        for (auto ch : p->child)
          cached_nodes[i].push_back(ch);

      sort(cached_nodes[i].begin(), cached_nodes[i].end(), [](Node *n1, Node *n2)
           { return n1->access_count > n2->access_count; });
      if (cached_nodes[i].size() > max_load_num)
        cached_nodes[i].resize(max_load_num);
    }

    for (int i = compress_depth; i <= depth - 1; ++i)
    {
      int access_count = 0;
      for (auto n : cached_nodes[i])
      {
        print_info(n, i - compress_depth + 1, n->key, n->child, fout);
        access_count += n->access_count;
      }
      cout << "Layer " << i << ": access_count = " << access_count << endl;
    }

    fout.close();
  }

  void load2switch_greedy(vector<string> filename, uint32_t root_load_num, 
			uint32_t max_load_num, uint32_t compress_depth, uint64_t mask)
  {
    ofstream fout(filename[0]);
    // vector<vector<Node *>> cached_nodes(depth - 1);
    vector<vector<Node *>> cached_nodes(max(depth, compress_depth + 1));
    vector<vector<KEY_TYPE>> cached_keys(compress_depth + 1);

    /*
     * For layer < tree_depth - compress_depth,
     * we greedily select the nodes with the largest access_count, and load them to stage 1
     */
    auto cmp_access = [](Node *&n1, Node *&n2) -> bool
    { return n1->access_count < n2->access_count; };
    priority_queue<Node *, vector<Node *>, decltype(cmp_access)> node_pq(cmp_access);
    unordered_map<Node *, KEY_TYPE> node_map;

    node_map[root] = KEY_TYPE();
    node_pq.push(root);
    while (node_map.size() + M <= root_load_num && !node_pq.empty())
    {
      auto tmp = node_pq.top();
      node_pq.pop();

      for (int i = 0; i < tmp->child.size(); ++i)
      {
        node_map[tmp->child[i]] = (i == 0 ? node_map[tmp] : tmp->key[i - 1]);
        if (!tmp->child[i]->isLeaf && tmp->child[i]->depth() < compress_depth)
          node_pq.push(tmp->child[i]);
      }

      node_map.erase(tmp);
    }

    vector<pair<KEY_TYPE, Node *>> node_vec;
    for (auto pk : node_map)
      node_vec.push_back(make_pair(pk.second, pk.first));

    sort(node_vec.begin(), node_vec.end());

    cached_nodes[compress_depth].push_back(node_vec[0].second);
    for (int i = 1; i < node_vec.size(); ++i)
    {
      cached_keys[compress_depth].push_back(node_vec[i].first);
      cached_nodes[compress_depth].push_back(node_vec[i].second);
    }
    print_info(root, 0, cached_keys[compress_depth], cached_nodes[compress_depth], fout, mask);
    cout << "Layer 0-" << compress_depth - 1 << ": access_count = " << root->access_count << endl;

    /* For layer > compress_depth, only frequent-accessed nodes are printed */
    sort(cached_nodes[compress_depth].begin(), cached_nodes[compress_depth].end(), [](Node *n1, Node *n2)
         { return n1->access_count > n2->access_count; });
    if (cached_nodes[compress_depth].size() > max_load_num)
      cached_nodes[compress_depth].resize(max_load_num);

    // for (int i = compress_depth + 1; i <= depth - 1; ++i)
    for (int i = compress_depth + 1; i <= depth - 2; ++i)
    {
      for (auto p : cached_nodes[i - 1])
        for (auto ch : p->child)
          cached_nodes[i].push_back(ch);

      sort(cached_nodes[i].begin(), cached_nodes[i].end(), [](Node *n1, Node *n2)
           { return n1->access_count > n2->access_count; });
      if (cached_nodes[i].size() > max_load_num)
        cached_nodes[i].resize(max_load_num);
    }

    // for (int i = compress_depth; i <= depth - 1; ++i)
    for (int i = compress_depth; i <= depth - 2; ++i)
    {
      if ((i - compress_depth) % 2 == 0 && (i - compress_depth) / 2 + 1 < filename.size())
      {
        fout.close();
        fout.open(filename[(i - compress_depth) / 2 + 1]);
      }

      int access_count = 0;
      for (auto n : cached_nodes[i])
      {
        print_info(n, i - compress_depth + 1, n->key, n->child, fout, mask);
        access_count += n->access_count;
      }
      cout << "Layer " << i << ": access_count = " << access_count << endl;
    }

    fout.close();
  }

protected:
	static const int MAX_NODE_SIZE = 100000000;
	Node *pool;
  Node *root;

	int nb_node = 1;

  uint32_t depth = 1;
  // mutex mutex_;

  void overflow(Node *cur)
  {
    if (cur->key.size() >= M)
    {
      if (cur == root || cur->isLoaded2Switch)
      {
        if (cur == root)
          ++depth;

        /*
         * When we need to split an unmodifiable node cur (means that cur is loaded to the switch),
         * we build a modifiable new node, copy_node, as the child of cur,
         * and move all <key, child/value> of cur to copy_node.
         * Then we split copy_node instead of cur in the following operations.
         * This operation ensures that the address of cur loaded to the switch is still valid.
         */
        Node *copy_node = &pool[nb_node++];
        copy_node->isLeaf = cur->isLeaf;
        copy_node->parent = cur;
        copy_node->key = cur->key;
        // copy_node->val = cur->val;
        copy_node->child = cur->child;
        copy_node->isLoaded2Switch = false;

        for (auto ch : copy_node->child)
          ch->parent = copy_node;

        cur->isLeaf = false;
        cur->key.clear();
        // cur->val.clear();
        cur->child.clear();
        cur->child.push_back(copy_node);

        cur = copy_node;
      }

      Node *new_node = &pool[nb_node++];
      new_node->parent = cur->parent;
      new_node->isLeaf = cur->isLeaf;

      if (cur->isLeaf)
      {
        new_node->key.insert(new_node->key.begin(), cur->key.begin() + M / 2,
                             cur->key.end());
        cur->key.erase(cur->key.begin() + M / 2, cur->key.end());
        // new_node->val.insert(new_node->val.begin(), cur->val.begin() + M / 2,
        //                      cur->val.end());
        // cur->val.erase(cur->val.begin() + M / 2, cur->val.end());
        new_node->child.insert(new_node->child.begin(), cur->child.begin() + M / 2,
                             cur->child.end());
        cur->child.erase(cur->child.begin() + M / 2, cur->child.end());

        auto p = lower_bound(cur->parent->key.begin(), cur->parent->key.end(),
                             new_node->key[0]) -
                 cur->parent->key.begin();
        cur->parent->key.insert(cur->parent->key.begin() + p, cur->key.back());
        cur->parent->child.insert(cur->parent->child.begin() + p + 1, new_node);
      }
      else
      {
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

  void print_info(Node *cur, uint32_t virtual_depth, vector<KEY_TYPE> &keys, vector<Node *> &childs, ofstream &fout)
  {
    cur->isLoaded2Switch = true;
    for (auto ch : childs)
      ch->isLoaded2Switch = true;

    if (cur == root)
      fout << "Node: 0x0 ";
    else
      fout << "Node: 0x" << hex << (uint64_t)cur - (uint64_t)root << dec << " ";
    fout << "(depth = " << virtual_depth << ")" << endl;
    fout << "keys: <0,0> ";
    for (auto k : keys)
    {
      uint64_t fbt_key = remove_key_index_prefix(k);
      uint32_t key_hi = fbt_key >> 32 & 0xffffffff;
      uint32_t key_lo = fbt_key & 0xffffffff;
      fout << "<" << key_hi << "," << key_lo << "> ";
    }
    fout << endl;
    fout << "child: ";
    for (auto c : childs)
      fout << hex << "0x" << (uint64_t)c - (uint64_t)root << dec << " ";
    fout << endl;
  }

  void print_info(Node *cur, uint32_t virtual_depth, vector<KEY_TYPE> &keys,
			vector<Node *> &childs, ofstream &fout, uint64_t mask)
  {
    cur->isLoaded2Switch = true;
    for (auto ch : childs)
      ch->isLoaded2Switch = true;

    if (cur == root)
      fout << "Node: 0x0 ";
    else
      fout << "Node: 0x" << hex << (mask | ((uint64_t)cur - (uint64_t)root)) << dec << " ";
    fout << "(depth = " << virtual_depth << ")" << endl;
    fout << "keys: <0,0> ";
    for (auto k : keys)
    {
      uint64_t fbt_key = remove_key_index_prefix(k);
      uint32_t key_hi = fbt_key >> 32 & 0xffffffff;
      uint32_t key_lo = fbt_key & 0xffffffff;
      fout << "<" << key_hi << "," << key_lo << "> ";
    }
    fout << endl;
    fout << "child: ";
    for (auto c : childs)
      fout << hex << "0x" << (mask | ((uint64_t)c - (uint64_t)root)) << dec << " ";
    fout << endl;
  }
};

#endif // YCSB_C_B_PLUS_TREE_DB_H_
