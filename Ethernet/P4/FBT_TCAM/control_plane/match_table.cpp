#include <bits/stdc++.h>

using namespace std;

vector<uint64_t> keylist;

class range {
public:
  uint64_t l;
  uint64_t r;

  range(uint64_t l, uint64_t r) : l(l), r(r) {}
};

vector<range> rangelist;

vector<uint64_t> table2;
vector<uint64_t> table3;
vector<uint64_t> table4;

map<uint64_t, uint64_t> pfidx;

int main() {
  ifstream keyinfo("keyinfo.txt");

  uint64_t key;

  keylist.push_back(UINT64_MAX);
  while (keyinfo >>  key) {
    keylist.push_back(key);
    if (keyinfo.get() == '\n') {
      break;
    }
  }
  for (int i = 1; i < keylist.size(); i++) {
    rangelist.push_back(range(keylist[i - 1] + 1, keylist[i]));
  }
  rangelist.push_back(range(keylist[keylist.size() - 1] + 1, UINT64_MAX));

  for (int i = 0; i < rangelist.size(); i++) {
    cout << " 0x" << setfill('0') << setw(16) << hex << rangelist[i].l << endl;
    cout << " 0x" << setfill('0') << setw(16) << hex << rangelist[i].r << endl;
    cout << endl;
  }

  int curidx = rangelist.size();
  cout << "# 1" << endl;
  int l = -1, r = -1, j = 0;
  for (uint64_t i = 0; i <= UINT16_MAX; i++) {
    uint64_t tmp_l = (i << 48);
    uint64_t tmp_r = tmp_l + ((1ULL << 48) - 1);

    if (rangelist[j].r < tmp_l) {
      if (l != -1 && r != -1) {
        cout << l << " " << r << " " << 0 << " " << j << endl;
        l = -1;
        r = -1;
      }

      while (rangelist[j].r < tmp_l) {
        j += 1;
      }
    }

    if (tmp_l >= rangelist[j].l && tmp_r <= rangelist[j].r) {
      if (l == -1 && r == -1) {
        l = i;
        r = i;
      } else {
        r = i;
      }
    } 
	else {
      if (l != -1 && r != -1) {
        cout << l << " " << r << " " << 0 << " " << j << endl;
        l = -1;
        r = -1;
      }

      pfidx[i] = curidx++;
      cout << i << " " << i << " " << 1 << " " << pfidx[i] << endl;
      table2.push_back(i);
    }
  }
  if (l != -1 && r != -1) {
    cout << l << " " << r << " " << 0 << " " << j << endl;
    l = -1;
    r = -1;
  }

  cout << endl;
  cout << endl;
  cout << "# 2" << endl;

  for (uint64_t pf : table2) {
    int l = -1, r = -1, j = 0;
    for (uint64_t i = 0; i <= UINT16_MAX; i++) {
      uint64_t tmp_l = (pf << 48) + (i << 32);
      uint64_t tmp_r = tmp_l + ((1ULL << 32) - 1);

      if (rangelist[j].r < tmp_l) {
        if (l != -1 && r != -1) {
          cout << pfidx[pf] << l << " " << r << " " << 0 << " " << j
               << endl;
          l = -1;
          r = -1;
        }

        while (rangelist[j].r < tmp_l) {
          j += 1;
        }
      }

      if (tmp_l >= rangelist[j].l && tmp_r <= rangelist[j].r) {
        if (l == -1 && r == -1) {
          l = i;
          r = i;
        } else {
          r = i;
        }
      } else {
        if (l != -1 && r != -1) {
          cout << pfidx[pf] << " " << l << " " << r << " " << 0 << " " << j
               << endl;
          l = -1;
          r = -1;
        }

        pfidx[(pf << 16) + i] = curidx++;
        cout << pfidx[pf] << " " << i << " " << i << " " << 1 << " "
             << pfidx[(pf << 16) + i] << endl;
        table3.push_back((pf << 16) + i);
      }
    }
    if (l != -1 && r != -1) {
      cout << pfidx[pf] << " " << l << " " << r << " " << 0 << " " << j
           << endl;
      l = -1;
      r = -1;
    }
  }

  cout << endl;
  cout << endl;
  cout << "# 3" << endl;

  for (uint64_t pf : table3) {
    int l = -1, r = -1, j = 0;
    for (uint64_t i = 0; i <= UINT16_MAX; i++) {
      uint64_t tmp_l = (pf << 32) + (i << 16);
      uint64_t tmp_r = tmp_l + ((1ULL << 16) - 1);

      if (rangelist[j].r < tmp_l) {
        if (l != -1 && r != -1) {
          cout << pfidx[pf] << l << " " << r << " " << 0 << " " << j
               << endl;
          l = -1;
          r = -1;
        }

        while (rangelist[j].r < tmp_l) {
          j += 1;
        }
      }

      if (tmp_l >= rangelist[j].l && tmp_r <= rangelist[j].r) {
        if (l == -1 && r == -1) {
          l = i;
          r = i;
        } else {
          r = i;
        }
      } else {
        if (l != -1 && r != -1) {
          cout << pfidx[pf] << " " << l << " " << r << " " << 0 << " " << j
               << endl;
          l = -1;
          r = -1;
        }

        pfidx[(pf << 16) + i] = curidx++;
        cout << pfidx[pf] << " " << i << " " << i << " " << 1 << " "
             << pfidx[(pf << 16) + i] << endl;
        table4.push_back((pf << 16) + i);
      }
    }
    if (l != -1 && r != -1) {
      cout << pfidx[pf] << " " << l << " " << r << " " << 0 << " " << j
           << endl;
      l = -1;
      r = -1;
    }
  }

  cout << endl;
  cout << endl;
  cout << "# 4" << endl;

  for (uint64_t pf : table4) {
    int l = -1, r = -1, j = 0;
    for (uint64_t i = 0; i <= UINT16_MAX; i++) {
      uint64_t tmp_l = (pf << 16) + (i << 0);
      uint64_t tmp_r = tmp_l + ((1ULL << 0) - 1);

      if (rangelist[j].r < tmp_l) {
        if (l != -1 && r != -1) {
          cout << pfidx[pf] << " " << l << " " << r << " " << 0 << " " << j
               << endl;
          l = -1;
          r = -1;
        }

        while (rangelist[j].r < tmp_l) {
          j += 1;
        }
      }

      if (tmp_l >= rangelist[j].l && tmp_r <= rangelist[j].r) {
        if (l == -1 && r == -1) {
          l = i;
          r = i;
        } else {
          r = i;
        }
      } else {
        if (l != -1 && r != -1) {
          cout << pfidx[pf] << " " << l << " " << r << " " << 0 << " " << j
               << endl;
          l = -1;
          r = -1;
        }

        pfidx[(pf << 16) + i] = curidx++;
        cout << pfidx[pf] << " " << i << " " << i << " " << 1 << " "
             << pfidx[(pf << 16) + i] << endl;
      }
    }
    if (l != -1 && r != -1) {
      cout << pfidx[pf] << " " << l << " " << r << " " << 0 << " " << j
           << endl;
      l = -1;
      r = -1;
    }
  }
}
