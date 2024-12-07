#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <vector>

using std::byte;
using std::min;
using std::vector;

template <typename T> struct unsafe_buf {
  const T* ptr;
  size_t len;

  inline const T* data() const { return this->ptr; }
  inline size_t size() const { return this->len; }

  inline vector<T> toVector() const {
    vector<T> vec;
    vec.insert(vec.end(), this->ptr, this->ptr + this->len);
    return vec;
  }

  static inline unsafe_buf<T> createFromVector(const vector<T>& vec) {
    return {
      ptr: vec.data(),
      len: vec.size(),
    };
  }

  static inline int compare(unsafe_buf<T>& first, unsafe_buf<T>& second) {
    size_t minSize = min(first.len * sizeof(T), second.len * sizeof(T));

    int comp = memcmp(first.ptr, second.ptr, minSize);
    if (comp == 0) {
      if (first.len * sizeof(T) == second.len * sizeof(T)) {
        return 0;
      }
      else if (first.len * sizeof(T) == minSize * sizeof(T)) {
        return 1;
      }
      else {
        assert(second.len * sizeof(T) == minSize);
        return -1;
      }
    }
    else {
      return comp;
    }
  }
};
