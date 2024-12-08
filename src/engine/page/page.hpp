/*
Page format:

Uses big-endian

Flags:
+-----------+----------+
| Node type | Reserved |
+-----------+----------+
| 4 bits    | 12 bits  |
+-----------+----------+


Leaf node:
+---------+------------+------------------------+---------------+
|  Flags  | Item count | Item slot (x item cnt) |     Data      |
+---------+------------+------------------------+---------------+
| 2 bytes | 2 bytes    | 7 bytes (x item cnt)   | Variable size |
|         |            |                        |               |
+---------+------------+------------------------+---------------+

Leaf item slot:
+---------+---------+---------+-----------+
| Offset  |  Ksize  |  Vsize  |   Flags   |
+---------+---------+---------+-----------+
| 2 bytes | 2 bytes | 2 bytes | 1 byte    |
+---------+---------+---------+-----------+

KV pair:
+-------+-------------------------------+-------+
|  Key  | (Optional) Value overflow ptr | Value |
+-------+-------------------------------+-------+
| Ksize | 4 bytes                       | Vsize |
+-------+-------------------------------+-------+

Slot flags:
+-----------+----------+
| Voverflow | Reserved |
+-----------+----------+
| 1 bit     | 7 bits   |
+-----------+----------+


Internal node:
+---------+------------+---------+-----------------------+---------------+
|  Flags  | Item count |  Lptr   | Key slot (x item cnt) |     Keys      |
+---------+------------+---------+-----------------------+---------------+
| 2 bytes | 2 bytes    | 6 bytes | 10 bytes (x item cnt) | Variable size |
|         |            |         |                       |               |
+---------+------------+---------+-----------------------+---------------+


Internal item slot:
+---------+---------+---------+
| Offset  |  Ksize  |  GEptr  |
+---------+---------+---------+
| 2 bytes | 2 bytes | 6 bytes |
+---------+---------+---------+


*/
#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>

#include <boost/endian/buffers.hpp>

#include "../common.hpp"

using std::vector;
using std::byte;

using namespace boost::endian;

#define MAX_PAGE_SIZE 4096
#define MERGE_THRESHOLD_PAGE_SIZE 1024

namespace {
  struct Header {
    big_uint16_buf_t flags;
  };

  struct LeafHeader {
    Header header;
    big_uint16_buf_t itemCount;
  };

  struct LeafSlot {
    big_uint16_buf_t offset;
    big_uint16_buf_t ksize;
    big_uint16_buf_t vsize;
    big_uint8_buf_t flags;
  };

  struct InternalHeader {
    Header header;
    big_uint16_buf_t itemCount;
    big_uint48_buf_t lPtr; // less
  };

  struct InternalSlot {
    big_uint16_buf_t offset;
    big_uint16_buf_t ksize;
    big_uint48_buf_t gePtr; // greater or equal
  };
};

typedef uint64_t pageptr_t;

typedef uint16_t pagesize_t;

enum class PageType: uint8_t {
  Meta = 0x0,
  Internal = 0x1,
  Leaf = 0x2,
  Overflow = 0x3
};

class Page {
 private:
  vector<byte> data;
 public:
  friend class Pager;
  friend class InternalPage;
  friend class LeafPage;

  Page();
  Page(vector<byte>& data);
  Page(unsafe_buf<byte>& data);
  static Page createInternal();
  static Page createLeaf();

  PageType getPageType();
  void setPageType(PageType type);
  
  size_t byteSize();
  bool isOversized();
  bool isUndersized();
};


class InternalPage {
 private:
  // int32_t leBsearchInternal(InternalSlot* slots, pagesize_t itemCount, const vector<byte>& key);
  int32_t leBsearchInternal(InternalSlot* slots, pagesize_t itemCount, const unsafe_buf<byte>& key);
  void insertInternalSlot(pagesize_t insertIn, const unsafe_buf<byte>& key, pageptr_t page);
 public:
  Page& page;

  InternalPage(Page& page): page(page) {};

  pagesize_t countInternal();
  
  // returned values are valid only during object's (page) lifetime
  unsafe_buf<byte> getKeyInternal(pagesize_t index);
  pageptr_t getPageptr(int32_t index); // -1 means lPtr

  void setKeyInternal(pagesize_t index, const vector<byte>& key, pageptr_t page);
  void setKeyInternal(pagesize_t index, const unsafe_buf<byte>& key, pageptr_t page);
  void setGEptr(pagesize_t index, pageptr_t page);
  void setLptr(pageptr_t page);

  int32_t searchInternal(const vector<byte>& key); // -1 means key is less than anything else (get lPtr)
  int32_t searchInternal(const unsafe_buf<byte>& key); // -1 means key is less than anything else (get lPtr)

  void putInternal(const vector<byte>& key, pageptr_t page);
  void putInternal(const unsafe_buf<byte>& key, pageptr_t page);

  void delInternal(int32_t index);
  void delRangeInternal(int32_t start, int32_t end); // [start, end)
};

class LeafPage {
 private:
  // int32_t exactBsearchLeaf(LeafSlot* slots, pagesize_t itemCount, const vector<byte>& key);
  int32_t exactBsearchLeaf(LeafSlot* slots, pagesize_t itemCount, const unsafe_buf<byte>& key);
  // int32_t leBsearchLeaf(LeafSlot* slots, pagesize_t itemCount, const vector<byte>& key);
  int32_t leBsearchLeaf(LeafSlot* slots, pagesize_t itemCount, const unsafe_buf<byte>& key, bool& exact);
 public:
  Page& page;

  LeafPage(Page& page): page(page) {};

  pagesize_t countLeaf();

  // returned values are valid only during object's (page) lifetime
  unsafe_buf<byte> getKeyLeaf(pagesize_t index);
  unsafe_buf<byte> getValue(pagesize_t index);

  void setKeyLeaf(pagesize_t index, const vector<byte>& key, const vector<byte>& value);
  void setKeyLeaf(pagesize_t index, const unsafe_buf<byte>& key, const unsafe_buf<byte>& value);

  int32_t searchLeaf(const vector<byte>& key); // -1 means key not found
  int32_t searchLeaf(const unsafe_buf<byte>& key); // -1 means key not found

  void putLeaf(const vector<byte>& key, const vector<byte>& value);
  void putLeaf(const unsafe_buf<byte>& key, const unsafe_buf<byte>& value);

  void delLeaf(pagesize_t index);
  void delRangeLeaf(pagesize_t start, pagesize_t end); // [start, end)
};
