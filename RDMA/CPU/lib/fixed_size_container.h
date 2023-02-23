#ifndef FIXED_SIZE_CONTAINER_H_
#define FIXED_SIZE_CONTAINER_H_

#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

template <typename TYPE, unsigned MAX_SIZE> class fixed_size_array {
public:
  typedef TYPE *iterator;

  fixed_size_array() { clear(); }

  fixed_size_array(const vector<TYPE> &x) {
    memmove(begin(), &x[0], x.size() * sizeof(TYPE));
    _size = x.size();
  }

  // fixed_size_array &operator=(const fixed_size_array &x) {
  //   _size = x.size();
  //   memmove(begin(), x.begin(), x.size() * sizeof(TYPE));
  // }

  iterator begin() { return _val; }

  iterator end() { return _val + _size; }

  size_t size() const { return _size; }

  void resize(size_t _resize) { _size = _resize; }

  TYPE &operator[](size_t n) { return _val[n]; }

  TYPE getItem(size_t n) const { return _val[n]; }

  TYPE &back() { return _val[_size - 1]; }

  void push_back(const TYPE &val) { _val[_size++] = val; }

  void insert(iterator pos, const TYPE &val) {
    memmove(pos + 1, pos, (end() - pos) * sizeof(TYPE));
    *pos = val;
    ++_size;
  }

  void insert(iterator pos, iterator first, iterator last) {
    memmove(pos + (last - first), pos, (end() - pos) * sizeof(TYPE));
    memmove(pos, first, (last - first) * sizeof(TYPE));
    _size += last - first;
  }

  void erase(iterator first, iterator last) {
    memmove(first, last, (end() - last) * sizeof(TYPE));
    _size -= last - first;
  }

  void clear() { _size = 0; }

  bool operator==(const fixed_size_array &rhs) const {
    if (size() != rhs.size()) {
      return false;
    }
    for (auto i = 0; i < size(); ++i)
      // if (this->[i] != rhs[i])
      if (_val[i] != rhs.getItem(i)) {
        return false;
      }
    return true;
  }

  bool operator!=(const fixed_size_array &rhs) const {
    if (size() != rhs.size()) {
      return true;
    }
    for (auto i = 0; i < size(); ++i)
      // if (this->[i] != rhs[i])
      if (_val[i] != rhs.getItem(i)) {
        return true;
      }
    return false;
  }

  bool operator<(const fixed_size_array &rhs) const {
    for (int i = 0; i < min(size(), rhs.size()); i++) {
      if (_val[i] < rhs.getItem(i)) {
        return true;
      } else if (_val[i] > rhs.getItem(i)) {
        return false;
      }
    }
    if (size() < rhs.size()) {
      return true;
    } else {
      return false;
    }
  }

  ostream &operator<<(ostream &out) const {
    for (int i = 0; i < _size; i++) {
      out << _val[i];
    }
    return out;
  }

  vector<TYPE> to_vector()
  {
    vector<TYPE> ret(_val, _val + _size);
    return ret;
  }

protected:
  size_t _size;
  TYPE _val[MAX_SIZE];
};

template <typename TYPE, unsigned MAX_SIZE>
ostream &operator<<(ostream &out, const fixed_size_array<TYPE, MAX_SIZE> &arr) {
  arr << out;
  return out;
}

template <unsigned MAX_SIZE>
class fixed_size_string : public fixed_size_array<char, MAX_SIZE> {
public:
  fixed_size_string() {}

  fixed_size_string(const string &x) {
    memmove(this->begin(), &x[0], x.size());
    this->_size = x.size();
  }

  string to_string()
  {
    string ret(this->_val, this->_val + this->_size);
    return ret;
  }
};

static const int KEY_LEN = 33;
static const int FIELD_LEN = 6;
static const int VALUE_LEN = 100;
static const int FIELD_NUM = 10;

typedef fixed_size_string<KEY_LEN> KEY_TYPE;
typedef fixed_size_array<
    pair<fixed_size_string<FIELD_LEN>, fixed_size_string<VALUE_LEN>>, FIELD_NUM>
    VAL_TYPE;

// template <unsigned MAX_SIZE>
// using fixed_size_string = fixed_size_array<char, MAX_SIZE>;

#endif // FIXED_SIZE_CONTAINER_H_
