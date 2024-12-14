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
#include <cmath>
#include <stdint.h>

#include <boost/endian/buffers.hpp>

#include "../common.hpp"

using std::vector;
using std::byte;

using namespace boost::endian;

#define PAGE_SIZE (4096)
#define MERGE_THRESHOLD_PAGE_SIZE (1024)
#define MAX_DELETED_COUNT ((PAGE_SIZE - sizeof(DeletedHeader)) / sizeof(DeletedSlot))

namespace {
  struct Header {
    big_uint16_buf_t flags;
    big_uint16_buf_t byteSize;
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
  };

  struct InternalSlot {
    big_uint16_buf_t offset;
    big_uint16_buf_t ksize;
    big_uint48_buf_t gePtr; // greater or equal
    // less-than pointer deleted
  };

  struct DeletedHeader {
    Header header;
    big_uint16_buf_t count;
    big_uint48_buf_t next;
  };

  struct DeletedSlot {
    big_uint48_buf_t ptr;
  };
};

typedef uint64_t pageptr_t;

typedef uint16_t pagesize_t;

enum class PageType: uint8_t {
  // Meta = 0x0,
  Internal = 0x1,
  Leaf = 0x2,
  Overflow = 0x3,
  Deleted = 0x4,
};

class Page {
 protected:
  vector<byte> data;

  // for byteSize field (needed for pager)
  pagesize_t getByteSize();
  void setPageType(pagesize_t size);
 public:
  friend class TransactionalPager;
  friend class InternalPage;
  friend class LeafPage;
  friend class DeletedPage;

  Page();
  Page(vector<byte>& data);
  Page(unsafe_buf<byte>& data);
  static Page createInternal();
  static Page createLeaf();
  static Page createDeleted();

  PageType getPageType();
  void setPageType(PageType type);
  
  size_t byteSize();
  bool isOversized();
  bool isUndersized();
};


class InternalPage {
 private:
  int32_t leBsearchInternal(InternalSlot* slots, pagesize_t itemCount, const unsafe_buf<byte>& key);
  void insertInternalSlot(pagesize_t insertIn, const unsafe_buf<byte>& key, pageptr_t page);
 public:
  Page& page;

  InternalPage(Page& page): page(page) {};

  pagesize_t countInternal();
  
  // returned values are valid only during object's (page) lifetime
  unsafe_buf<byte> getKeyInternal(pagesize_t index);
  pageptr_t getPageptr(pagesize_t index); // -1 means lPtr

  void setKeyInternal(pagesize_t index, const vector<byte>& key, pageptr_t page);
  void setKeyInternal(pagesize_t index, const unsafe_buf<byte>& key, pageptr_t page);
  void setGEptr(pagesize_t index, pageptr_t page);

  int32_t searchInternal(const vector<byte>& key); // -1 means key not found
  int32_t searchInternal(const unsafe_buf<byte>& key); // -1 means key not found

  void putInternal(const vector<byte>& key, pageptr_t page);
  void putInternal(const unsafe_buf<byte>& key, pageptr_t page);

  void delInternal(pagesize_t index);
  void delRangeInternal(pagesize_t start, pagesize_t end); // [start, end)
};

class LeafPage {
 private:
  int32_t exactBsearchLeaf(LeafSlot* slots, pagesize_t itemCount, const unsafe_buf<byte>& key);
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

class DeletedPage {
 public:
  Page& page;

  DeletedPage(Page& page): page(page) {};

  pageptr_t getNext();
  void setNext(pageptr_t next);

  pagesize_t getCount();
  pageptr_t getPtr(pagesize_t index);
  void putPtr(pagesize_t ptr);
};
