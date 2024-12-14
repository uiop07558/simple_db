#include <cstring>
#include <cassert>
#include <utility>
#include <cmath>

#include "page.hpp"

using std::to_underlying;

#define assertPageType(pageType) assert(pageType == PageType::Internal || pageType == PageType::Leaf || pageType == PageType::Overflow || pageType == PageType::Deleted)

#define PAGE_TYPE_BIT_DIST 12

#define offsetZeroInternal(itemCount) (sizeof(InternalHeader) + itemCount * sizeof(InternalSlot))
#define offsetToAddrInternal(itemCount, offset) (offsetZeroInternal(itemCount) + offset)

#define offsetZeroLeaf(itemCount) (sizeof(LeafHeader) + itemCount * sizeof(LeafSlot))
#define offsetToAddrLeaf(itemCount, offset) (offsetZeroLeaf(itemCount) + offset)

Page::Page() {
  this->data = vector<byte>();
}

Page::Page(vector<byte>& data) {
  this->data = data;
}

Page::Page(unsafe_buf<byte>& data) {
  this->data = vector<byte>();
  this->data.insert(this->data.end(), data.ptr, data.ptr + data.len);
}

Page Page::createInternal() {
  auto page = Page();
  page.data.resize(sizeof(InternalHeader));
  InternalHeader* header = reinterpret_cast<InternalHeader*>(page.data.data() + 0);
  page.setPageType(PageType::Internal);
  header->itemCount = 0;

  return page;
}

Page Page::createLeaf() {
  auto page = Page();
  page.data.resize(sizeof(LeafHeader));
  LeafHeader* header = reinterpret_cast<LeafHeader*>(page.data.data() + 0);
  page.setPageType(PageType::Leaf);
  header->itemCount = 0;

  return page;
}

Page Page::createDeleted() {
  auto page = Page();
  page.data.resize(sizeof(DeletedHeader));
  DeletedHeader* header = reinterpret_cast<DeletedHeader*>(page.data.data() + 0);
  page.setPageType(PageType::Deleted);
  header->next = 0;
  header->count = 0;

  return page;
}

inline PageType Page::getPageType() {
  assert(this->byteSize() >= sizeof(Header));

  Header* header = reinterpret_cast<Header*>(this->data.data() + 0);

  uint16_t flags = header->flags.value();
  PageType pageType = PageType(flags >> PAGE_TYPE_BIT_DIST);

  assertPageType(pageType);

  return pageType;
}

inline void Page::setPageType(PageType type) {
  assertPageType(type);
  assert(this->byteSize() >= sizeof(Header));

  uint16_t flags = to_underlying<PageType>(type);
  flags = flags << PAGE_TYPE_BIT_DIST;

  Header* header = reinterpret_cast<Header*>(this->data.data() + 0);
  header->flags = flags;
}

inline pagesize_t Page::getByteSize() {
  assert(this->byteSize() >= sizeof(Header));

  Header* header = reinterpret_cast<Header*>(this->data.data() + 0);
  pagesize_t size = header->byteSize.value();
  return size;
}

inline void Page::setPageType(pagesize_t size) {
  assert(this->byteSize() >= sizeof(Header));

  Header* header = reinterpret_cast<Header*>(this->data.data() + 0);
  header->byteSize = size;
}


inline size_t Page::byteSize() {
  return this->data.size();
}

inline bool Page::isOversized() {
  return this->byteSize() > PAGE_SIZE;
}

inline bool Page::isUndersized() {
  return this->byteSize() < MERGE_THRESHOLD_PAGE_SIZE;
}

int32_t InternalPage::leBsearchInternal(InternalSlot* slots, pagesize_t itemCount, const unsafe_buf<byte>& key) {
  pagesize_t left = 0;
  pagesize_t right = itemCount;

  unsafe_buf<byte> keyBufArg = key;

  int32_t pos = -1;
  while (left < right) { // less or equal bsearch
    pagesize_t mid = floor(left + ((right - left) >> 1));
    pagesize_t ksizeMid = slots[mid].ksize.value();
    pagesize_t offsetMid = slots[mid].offset.value();
    unsafe_buf<byte> keyBufMid = {
      ptr: this->page.data.data() + offsetToAddrInternal(itemCount, offsetMid),
      len: ksizeMid,
    };

    int comp = unsafe_buf<byte>::compare(keyBufArg, keyBufMid);
    if (comp == 0) {
      pos = mid;
      break;
    }
    else if (comp > 0) { // arg > mid
      left = mid + 1;
    }
    else {
      right = mid;
    }
  }
  if (pos == -1) {
    pos = (int32_t) left - 1;
  }

  return pos;
}

inline pagesize_t InternalPage::countInternal()
{
  assert(this->page.getPageType() == PageType::Internal);
  assert(this->page.byteSize() >= sizeof(InternalHeader));

  InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);
  assert(this->page.byteSize() >= sizeof(InternalHeader) + header->itemCount.value() * sizeof(InternalSlot));
  return header->itemCount.value();
}

inline unsafe_buf<byte> InternalPage::getKeyInternal(pagesize_t index) {
  assert(this->countInternal() > index);

  InternalSlot* slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
  
  assert(this->page.byteSize() >= offsetToAddrInternal(this->countInternal(), slot->offset.value()) + slot->ksize.value());

  unsafe_buf<byte> key = {
    ptr: &this->page.data[offsetToAddrInternal(this->countInternal(), slot->offset.value())],
    len: slot->ksize.value(),
  };

  return key;
}

inline pageptr_t InternalPage::getPageptr(pagesize_t index){
  assert(this->countInternal() > index);
  assert(index >= 0);

  InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data());

  InternalSlot* slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
  assert(this->page.byteSize() >= offsetToAddrInternal(header->itemCount.value(), slot->offset.value()) + slot->ksize.value());
  return slot->gePtr.value();
}

inline void InternalPage::setKeyInternal(pagesize_t index, const vector<byte>& key, pageptr_t page) {
  this->setKeyInternal(index, unsafe_buf<byte>::createFromVector(key), page);
}

inline void InternalPage::setKeyInternal(pagesize_t index, const unsafe_buf<byte>& key, pageptr_t page) {
  assert(this->countInternal() > index);

  InternalSlot* slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
  if (slot->ksize.value() == key.size()) {
    memcpy(this->page.data.data() + offsetToAddrInternal(this->countInternal(), slot->offset.value()), key.data(), key.size());
  }
  else {
    pagesize_t kAddr = offsetToAddrInternal(this->countInternal(), slot->offset.value());
    this->page.data.erase(this->page.data.begin() + kAddr, this->page.data.begin() + kAddr + slot->ksize.value());

    slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
    slot->offset = this->page.data.size() - offsetZeroInternal(this->countInternal());

    this->page.data.insert(this->page.data.end(), key.data(), key.data() + key.size());
    slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
  }

  slot->gePtr = page;
}

inline void InternalPage::setGEptr(pagesize_t index, pageptr_t page) {
  assert(this->countInternal() > index);

  InternalSlot* slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
  slot->gePtr = page;
}

inline int32_t InternalPage::searchInternal(const vector<byte> &key) {
  return this->searchInternal(unsafe_buf<byte>::createFromVector(key));
}

inline int32_t InternalPage::searchInternal(const unsafe_buf<byte> &key) {
  assert(this->page.getPageType() == PageType::Internal);
  assert(this->page.byteSize() >= sizeof(InternalHeader));
  InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);
  pagesize_t itemCount = header->itemCount.value();

  assert(this->page.byteSize() >= sizeof(InternalHeader) + itemCount * sizeof(InternalSlot));
  InternalSlot* slots = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader));

  int32_t pos = this->leBsearchInternal(slots, itemCount, key);

  return pos;
}


inline void InternalPage::putInternal(const vector<byte>& key, pageptr_t page) {
  this->putInternal(unsafe_buf<byte>::createFromVector(key), page);
}

inline void InternalPage::insertInternalSlot(pagesize_t insertIn, const unsafe_buf<byte>& key, pageptr_t page) {
  assert(this->page.getPageType() == PageType::Internal);
  assert(this->page.byteSize() >= sizeof(InternalHeader));
  InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);

  assert(this->page.byteSize() >= sizeof(InternalHeader) + header->itemCount.value() * sizeof(InternalSlot));
  InternalSlot* slots = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader));

  header->itemCount = header->itemCount.value() + 1;

  InternalSlot newSlot;
  pagesize_t newSlotOffset = sizeof(InternalHeader) + insertIn * sizeof(InternalSlot);
  this->page.data.insert(this->page.data.begin() + newSlotOffset, reinterpret_cast<byte*>(&newSlot), reinterpret_cast<byte*>(&newSlot) + sizeof(InternalSlot));

  slots = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader));
  slots[insertIn].ksize = key.size();
  slots[insertIn].gePtr = page;
  slots[insertIn].offset = this->page.data.size() - offsetZeroInternal(this->countInternal());
  this->page.data.insert(this->page.data.end(), key.data(), key.data() + key.size());
}

inline void InternalPage::putInternal(const unsafe_buf<byte>& key, pageptr_t page) {
  assert(this->page.getPageType() == PageType::Internal);
  assert(this->page.byteSize() >= sizeof(InternalHeader));
  InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);
  pagesize_t itemCount = header->itemCount.value();

  assert(this->page.byteSize() >= sizeof(InternalHeader) + itemCount * sizeof(InternalSlot));
  if (header->itemCount.value() == 0) {
    this->insertInternalSlot(0, key, page);
    return;
  }
  InternalSlot* slots = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader));

  int32_t insertIn = this->leBsearchInternal(slots, itemCount, key);

  insertIn += 1;

  assert(insertIn >= 0);
  assert(insertIn <= header->itemCount.value());

  this->insertInternalSlot(insertIn, key, page);
}

inline void InternalPage::delInternal(pagesize_t index) {
  assert(this->countInternal() > index);

  InternalSlot* slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
  pagesize_t delOffset = slot->offset.value();
  pagesize_t delSize = slot->ksize.value();
  auto startKey = this->page.data.begin() + offsetToAddrInternal(this->countInternal(), slot->offset.value());
  auto endKey = startKey + slot->ksize.value();
  this->page.data.erase(startKey, endKey);

  auto startSlot = this->page.data.begin() + sizeof(InternalHeader) + index * sizeof(InternalSlot);
  auto endSlot = this->page.data.begin() + sizeof(InternalHeader) + (index+1) * sizeof(InternalSlot);
  this->page.data.erase(startSlot, endSlot);

  InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);
  header->itemCount = header->itemCount.value() - 1;

  InternalSlot* slots = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader));
  for (pagesize_t i = 0; i < this->countInternal(); i++) {
    if (slots[i].offset.value() >= delOffset) {
      slots[i].offset = slots[i].offset.value() - delSize;
    }
  }
}

void InternalPage::delRangeInternal(pagesize_t start, pagesize_t end) {
  assert(this->countInternal() >= end);
  assert(end > start);

  for (pagesize_t i = end - 1; i >= start; i--) {
    this->delInternal(i);
  }
}

int32_t LeafPage::exactBsearchLeaf(LeafSlot* slots, pagesize_t itemCount, const unsafe_buf<byte>& key) {
  int32_t left = 0;
  int32_t right = itemCount - 1;

  unsafe_buf<byte> keyBufArg = key;

  int32_t pos = -1;
  while (left <= right) {
    pagesize_t mid = floor(left + ((right - left) >> 1));
    pagesize_t ksizeMid = slots[mid].ksize.value();
    pagesize_t offsetMid = slots[mid].offset.value();
    unsafe_buf<byte> keyBufMid = {
      ptr: this->page.data.data() + offsetToAddrLeaf(itemCount, offsetMid),
      len: ksizeMid,
    };

    int comp = unsafe_buf<byte>::compare(keyBufArg, keyBufMid);
    if (comp < 0) { // arg < mid
      right = mid - 1;
    }
    else if (comp > 0) {
      left = mid + 1;
    }
    else {
      pos = mid;
      break;
    }
  }

  return pos;
}

int32_t LeafPage::leBsearchLeaf(LeafSlot* slots, pagesize_t itemCount, const unsafe_buf<byte>& key, bool& exact) {
  exact = false;
  pagesize_t left = 0;
  pagesize_t right = itemCount;

  unsafe_buf<byte> keyBufArg = key;

  int32_t pos = -1;
  while (left < right) { // less or equal bsearch
    pagesize_t mid = floor(left + ((right - left) >> 1));
    pagesize_t ksizeMid = slots[mid].ksize.value();
    pagesize_t offsetMid = slots[mid].offset.value();
    unsafe_buf<byte> keyBufMid = {
      ptr: this->page.data.data() + offsetToAddrLeaf(itemCount, offsetMid),
      len: ksizeMid,
    };

    int comp = unsafe_buf<byte>::compare(keyBufArg, keyBufMid);
    if (comp == 0) {
      pos = mid;
      exact = true;
      break;
    }
    else if (comp > 0) { // arg > mid
      left = mid + 1;
    }
    else {
      right = mid;
    }
  }
  if (pos == -1) {
    pos = (int32_t) left - 1;
  }

  return pos;
}

inline void LeafPage::putLeaf(const vector<byte> &key, const vector<byte> &value) {
  this->putLeaf(unsafe_buf<byte>::createFromVector(key), unsafe_buf<byte>::createFromVector(value));
}

inline void LeafPage::putLeaf(const unsafe_buf<byte> &key, const unsafe_buf<byte> &value) {
  assert(this->page.getPageType() == PageType::Leaf);
  assert(this->page.byteSize() >= sizeof(LeafHeader));
  LeafHeader* header = reinterpret_cast<LeafHeader*>(this->page.data.data() + 0);

  assert(this->page.byteSize() >= sizeof(LeafHeader) + header->itemCount.value() * sizeof(LeafSlot));
  LeafSlot* slots = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader));

  bool exact = false;
  int32_t insertIn = this->leBsearchLeaf(slots, header->itemCount.value(), key, exact);
  if (exact) {
    this->setKeyLeaf(insertIn, key, value);
    return;
  }
  
  insertIn += 1;
  
  header->itemCount = header->itemCount.value() + 1;

  LeafSlot newSlot;
  pagesize_t newSlotOffset = sizeof(LeafHeader) + insertIn * sizeof(LeafSlot);
  this->page.data.insert(this->page.data.begin() + newSlotOffset, reinterpret_cast<byte*>(&newSlot), reinterpret_cast<byte*>(&newSlot) + sizeof(LeafSlot));

  slots = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader));
  slots[insertIn].ksize = key.len;
  slots[insertIn].vsize = value.len;
  slots[insertIn].flags = 0;
  slots[insertIn].offset = this->page.data.size() - offsetZeroLeaf(this->countLeaf());
  this->page.data.insert(this->page.data.end(), key.ptr, key.ptr + key.len);
  this->page.data.insert(this->page.data.end(), value.ptr, value.ptr + value.len);
}

inline pagesize_t LeafPage::countLeaf() {
  assert(this->page.getPageType() == PageType::Leaf);
  assert(this->page.byteSize() >= sizeof(LeafHeader));

  LeafHeader* header = reinterpret_cast<LeafHeader*>(this->page.data.data() + 0);
  assert(this->page.byteSize() >= sizeof(LeafHeader) + header->itemCount.value() * sizeof(LeafSlot));
  return header->itemCount.value();
}

inline unsafe_buf<byte> LeafPage::getKeyLeaf(pagesize_t index) {
  assert(this->countLeaf() > index);

  LeafSlot* slot = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader) + index * sizeof(LeafSlot));
  
  assert(this->page.byteSize() >= offsetToAddrLeaf(this->countLeaf(), slot->offset.value()) + slot->ksize.value());

  unsafe_buf<byte> key = {
    ptr: &this->page.data[offsetToAddrLeaf(this->countLeaf(), slot->offset.value())],
    len: slot->ksize.value(),
  };

  return key;
}

inline unsafe_buf<byte> LeafPage::getValue(pagesize_t index) {
  assert(this->countLeaf() > index);

  LeafSlot* slot = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader) + index * sizeof(LeafSlot));
  
  assert(this->page.byteSize() >= offsetToAddrLeaf(this->countLeaf(), slot->offset.value()) + slot->ksize.value() + slot->vsize.value());

  unsafe_buf<byte> key = {
    ptr: &this->page.data[offsetToAddrLeaf(this->countLeaf(), slot->offset.value()) + slot->ksize.value()],
    len: slot->vsize.value(),
  };

  return key;
}

inline void LeafPage::setKeyLeaf(pagesize_t index, const vector<byte>& key, const vector<byte>& value) {
  this->setKeyLeaf(index, unsafe_buf<byte>::createFromVector(key), unsafe_buf<byte>::createFromVector(value));
}

inline void LeafPage::setKeyLeaf(pagesize_t index, const unsafe_buf<byte>& key, const unsafe_buf<byte>& value) {
  assert(this->countLeaf() > index);

  LeafSlot* slot = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader) + index * sizeof(LeafSlot));
  pagesize_t kvSizeOld = slot->ksize.value() + slot->vsize.value();
  pagesize_t kvSizeNew = key.size() + value.size();

  pagesize_t kvAddr = offsetToAddrLeaf(this->countLeaf(), slot->offset.value());  
  if (kvSizeNew == kvSizeOld) {
    memcpy(this->page.data.data() + kvAddr, key.data(), key.size());
    memcpy(&this->page.data[kvAddr + slot->ksize.value()], value.data(), value.size());
  }
  else {
    this->page.data.erase(this->page.data.begin() + kvAddr, this->page.data.begin() + kvAddr + kvSizeOld);

    slot = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader) + index * sizeof(LeafSlot));
    slot->offset = this->page.data.size();
    
    this->page.data.insert(this->page.data.end(), key.data(), key.data() + key.size());
    this->page.data.insert(this->page.data.end(), value.data(), value.data() + value.size());
  }
}

inline int32_t LeafPage::searchLeaf(const vector<byte> &key) {
  return this->searchLeaf(unsafe_buf<byte>::createFromVector(key));
}

inline int32_t LeafPage::searchLeaf(const unsafe_buf<byte> &key) {
  assert(this->page.getPageType() == PageType::Leaf);
  assert(this->page.byteSize() >= sizeof(LeafHeader));
  LeafHeader* header = reinterpret_cast<LeafHeader*>(this->page.data.data() + 0);
  pagesize_t itemCount = header->itemCount.value();

  assert(this->page.byteSize() >= sizeof(LeafHeader) + itemCount * sizeof(LeafSlot));
  LeafSlot* slots = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader));

  return this->exactBsearchLeaf(slots, itemCount, key);
}

inline void LeafPage::delLeaf(pagesize_t index) {
  assert(this->countLeaf() > index);

  LeafSlot* slot = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader) + index * sizeof(LeafSlot));
  pagesize_t delOffset = slot->offset.value();
  pagesize_t delSize = slot->ksize.value() + slot->vsize.value();
  auto startKey = this->page.data.begin() + offsetToAddrLeaf(this->countLeaf(), delOffset);
  auto endKey = startKey + delSize;
  this->page.data.erase(startKey, endKey);

  auto startSlot = this->page.data.begin() + sizeof(LeafHeader) + index * sizeof(LeafSlot);
  auto endSlot = this->page.data.begin() + sizeof(LeafHeader) + (index+1) * sizeof(LeafSlot);
  this->page.data.erase(startSlot, endSlot);

  LeafHeader* header = reinterpret_cast<LeafHeader*>(this->page.data.data() + 0);
  header->itemCount = header->itemCount.value() - 1;

  LeafSlot* slots = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader));
  for (pagesize_t i = 0; i < this->countLeaf(); i++) {
    if (slots[i].offset.value() >= delOffset) {
      slots[i].offset = slots[i].offset.value() - delSize;
    }
  }
}

inline void LeafPage::delRangeLeaf(pagesize_t start, pagesize_t end) {
  assert(this->countLeaf() >= end);
  assert(end > start);

  for (pagesize_t i = end - 1; i >= start; i--) {
    this->delLeaf(i);
  }
}

pageptr_t DeletedPage::getNext() {
  assert(this->page.getPageType() == PageType::Deleted);
  assert(this->page.byteSize() >= sizeof(DeletedHeader));

  DeletedHeader* header = reinterpret_cast<DeletedHeader*>(this->page.data.data() + 0);
  pageptr_t next = header->next.value();
  return next;
}

void DeletedPage::setNext(pageptr_t next) {
  assert(this->page.getPageType() == PageType::Deleted);
  assert(this->page.byteSize() >= sizeof(DeletedHeader));

  DeletedHeader* header = reinterpret_cast<DeletedHeader*>(this->page.data.data() + 0);
  header->next = next;
}

pagesize_t DeletedPage::getCount(){
  assert(this->page.getPageType() == PageType::Deleted);
  assert(this->page.byteSize() >= sizeof(DeletedHeader));

  DeletedHeader* header = reinterpret_cast<DeletedHeader*>(this->page.data.data() + 0);
  pagesize_t cnt = header->count.value();
  return cnt;
}

pageptr_t DeletedPage::getPtr(pagesize_t index) {
  assert(this->page.byteSize() >= sizeof(DeletedHeader) + getCount() * sizeof(DeletedSlot));
  assert(index < getCount());

  DeletedSlot* slot = reinterpret_cast<DeletedSlot*>(this->page.data.data() + sizeof(DeletedHeader) + index * sizeof(DeletedSlot));
  pageptr_t ptr = slot->ptr.value();
  return ptr;
}

void DeletedPage::putPtr(pagesize_t newPtr) {
  assert(this->page.byteSize() >= sizeof(DeletedHeader) + getCount() * sizeof(DeletedSlot));

  DeletedSlot newSlot;
  newSlot.ptr = newPtr;
  page.data.insert(page.data.end(), reinterpret_cast<byte*>(&newSlot), reinterpret_cast<byte*>(&newSlot) + sizeof(DeletedSlot));
  DeletedHeader* header = reinterpret_cast<DeletedHeader*>(this->page.data.data() + 0);
  header->count = header->count.value() + 1;
}
