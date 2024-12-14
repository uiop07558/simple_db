#pragma once

#include <cstdint>

#include <vector>
#include <cstddef>
#include <stdint.h>

#include <boost/endian/buffers.hpp>

#include "../common.hpp"

using std::vector;
using std::byte;

using namespace boost::endian;

namespace {
  struct MetaPageData {
    uint8_t sig[2];
    big_uint16_buf_t version;
    big_uint48_buf_t curSize;
    big_uint48_buf_t metaTableRoot;
    big_uint48_buf_t freeListHead;
    big_uint48_buf_t freeListTail;
  };
};

class MetaPage {
 private: 
  vector<byte> data;
  pagesize_t getByteSize() { return sizeof(MetaPageData); }
  
  void setFreeListHead(pageptr_t ptr);
  void setFreeListTail(pageptr_t ptr);
  void setCursize(pageptr_t ptr);
 public:
  friend class TransactionalPager;

  MetaPage() {
    this->data = vector<byte>();
    auto metaData = MetaPageData {
      sig: {'d', 'b'},
    };
    metaData.version = 1;
    metaData.curSize = 0;
    metaData.metaTableRoot = 0;
    metaData.freeListHead = 0;
    metaData.freeListTail = 0;

    this->data.insert(this->data.end(), reinterpret_cast<byte*>(&metaData), reinterpret_cast<byte*>(&metaData) + sizeof(MetaPageData));
  }

  MetaPage(vector<byte>& data) {
    this->data = data;
  }

  MetaPage(unsafe_buf<byte>& data) {
    this->data = vector<byte>();
    this->data.insert(this->data.end(), data.ptr, data.ptr + data.len);
  }

  inline size_t byteSize() { return data.size(); }

  pageptr_t getCursize();
  pageptr_t getMetaTableRoot();
  pageptr_t getFreeListHead();
  pageptr_t getFreeListTail();

  void setMetaTableRoot(pageptr_t ptr);
};
