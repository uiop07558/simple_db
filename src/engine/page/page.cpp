#include <cstring>
#include <cassert>
#include <utility>
#include <cmath>

#include "page.hpp"

using std::to_underlying;

#define assertPageType(pageType) assert(pageType == PageType::Meta || pageType == PageType::Internal || pageType == PageType::Leaf || pageType == PageType::Overflow)

#define PAGE_TYPE_BIT_DIST 12

Page::Page() {
  this->data = vector<byte>();
}

Page::Page(vector<byte> &data) {
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
  header->lPtr = 0;

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

inline size_t Page::byteSize() {
  return this->data.size();
}

inline bool Page::isOversized() {
  return this->byteSize() > MAX_PAGE_SIZE;
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
      ptr: this->page.data.data() + offsetMid,
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
  
  assert(this->page.byteSize() >= slot->offset.value() + slot->ksize.value());

  unsafe_buf<byte> key = {
    ptr: &this->page.data[slot->offset.value()],
    len: slot->ksize.value(),
  };

  return key;
}

inline pageptr_t InternalPage::getPageptr(int32_t index){
  assert(this->countInternal() > index);
  assert(index >= -1);

  if (index >= 0) {
    InternalSlot* slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
    assert(this->page.byteSize() >= slot->offset.value() + slot->ksize.value());
    return slot->gePtr.value();
  }
  else {
    InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);
    return header->lPtr.value();
  }
}

inline void InternalPage::setKeyInternal(pagesize_t index, const vector<byte>& key, pageptr_t page) {
  this->setKeyInternal(index, unsafe_buf<byte>::createFromVector(key), page);
}

inline void InternalPage::setKeyInternal(pagesize_t index, const unsafe_buf<byte>& key, pageptr_t page) {
  assert(this->countInternal() > index);

  InternalSlot* slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
  if (slot->ksize.value() == key.size()) {
    memcpy(&this->page.data[slot->offset.value()], key.data(), key.size());
  }
  else {
    pagesize_t offset = slot->offset.value();
    this->page.data.erase(this->page.data.begin() + offset, this->page.data.begin() + offset + slot->ksize.value());
    slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));

    slot->offset = this->page.data.size();

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

inline void InternalPage::setLptr(pageptr_t page) {
  assert(this->page.getPageType() == PageType::Internal);
  assert(this->page.byteSize() >= sizeof(InternalHeader));

  InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);
  header->lPtr = page;
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
  if (itemCount == 0) {
    return -1;
  }
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

  InternalSlot newSlot;
  pagesize_t newSlotOffset = sizeof(InternalHeader) + insertIn * sizeof(InternalSlot);
  this->page.data.insert(this->page.data.begin() + newSlotOffset, reinterpret_cast<byte*>(&newSlot), reinterpret_cast<byte*>(&newSlot) + sizeof(InternalSlot));

  slots = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader));
  slots[insertIn].ksize = key.size();
  slots[insertIn].gePtr = page;
  slots[insertIn].offset = this->page.data.size();
  this->page.data.insert(this->page.data.end(), key.data(), key.data() + key.size());

  header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);
  header->itemCount = header->itemCount.value() + 1;
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

  if (insertIn == -1) {
    header->lPtr = page;
    return;
  }

  assert(insertIn >= 0);
  assert(insertIn < header->itemCount.value());

  this->insertInternalSlot(insertIn, key, page);
}

inline void InternalPage::delInternal(pagesize_t index) {
  assert(this->countInternal() > index);

  InternalSlot* slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + index * sizeof(InternalSlot));
  auto startKey = this->page.data.begin() + slot->offset.value();
  auto endKey = startKey + slot->ksize.value();
  this->page.data.erase(startKey, endKey);

  auto startSlot = this->page.data.begin() + sizeof(InternalHeader) + index * sizeof(InternalSlot);
  auto endSlot = this->page.data.begin() + sizeof(InternalHeader) + (index+1) * sizeof(InternalSlot);
  this->page.data.erase(startSlot, endSlot);

  InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);
  header->itemCount = header->itemCount.value() - 1;
}

void InternalPage::delRangeInternal(pagesize_t start, pagesize_t end) {
  assert(this->countInternal() >= end);
  assert(end > start);

  for (pagesize_t i = end - 1; i >= start; i--) {
    InternalSlot* slot = reinterpret_cast<InternalSlot*>(this->page.data.data() + sizeof(InternalHeader) + i * sizeof(InternalSlot));
    auto startKey = this->page.data.begin() + slot->offset.value();
    auto endKey = startKey + slot->ksize.value();
    this->page.data.erase(startKey, endKey);
  }

  auto startSlot = this->page.data.begin() + sizeof(InternalHeader) + start * sizeof(InternalSlot);
  auto endSlot = this->page.data.begin() + sizeof(InternalHeader) + end * sizeof(InternalSlot);
  this->page.data.erase(startSlot, endSlot);

  InternalHeader* header = reinterpret_cast<InternalHeader*>(this->page.data.data() + 0);
  header->itemCount = header->itemCount.value() - (end - start);
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
      ptr: this->page.data.data() + offsetMid,
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
      ptr: this->page.data.data() + offsetMid,
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
  
  if (insertIn < 0) {
    insertIn = 0;
  }

  LeafSlot newSlot;
  pagesize_t newSlotOffset = sizeof(LeafHeader) + insertIn * sizeof(LeafSlot);
  for (pagesize_t i = 0; i < header->itemCount.value(); i++) {
    slots[i].offset = slots[i].offset.value() + sizeof(LeafSlot);
  }
  this->page.data.insert(this->page.data.begin() + newSlotOffset, reinterpret_cast<byte*>(&newSlot), reinterpret_cast<byte*>(&newSlot) + sizeof(LeafSlot));

  slots = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader));
  slots[insertIn].ksize = key.len;
  slots[insertIn].vsize = value.len;
  slots[insertIn].flags = 0;
  slots[insertIn].offset = this->page.data.size();
  this->page.data.insert(this->page.data.end(), key.ptr, key.ptr + key.len);
  this->page.data.insert(this->page.data.end(), value.ptr, value.ptr + value.len);

  header = reinterpret_cast<LeafHeader*>(this->page.data.data() + 0);
  header->itemCount = header->itemCount.value() + 1;
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
  
  assert(this->page.byteSize() >= slot->offset.value() + slot->ksize.value());

  unsafe_buf<byte> key = {
    ptr: &this->page.data[slot->offset.value()],
    len: slot->ksize.value(),
  };

  return key;
}

inline unsafe_buf<byte> LeafPage::getValue(pagesize_t index) {
  assert(this->countLeaf() > index);

  LeafSlot* slot = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader) + index * sizeof(LeafSlot));
  
  assert(this->page.byteSize() >= slot->offset.value() + slot->ksize.value() + slot->vsize.value());

  unsafe_buf<byte> key = {
    ptr: &this->page.data[slot->offset.value() + slot->ksize.value()],
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

  pagesize_t offset = slot->offset.value();  
  if (kvSizeNew == kvSizeOld) {
    memcpy(this->page.data.data() + offset, key.data(), key.size());
    memcpy(&this->page.data[offset + slot->ksize.value()], value.data(), value.size());
  }
  else {
    this->page.data.erase(this->page.data.begin() + offset, this->page.data.begin() + offset + kvSizeOld);

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
  auto startKey = this->page.data.begin() + slot->offset.value();
  auto endKey = startKey + slot->ksize.value() + slot->vsize.value();
  this->page.data.erase(startKey, endKey);

  auto startSlot = this->page.data.begin() + sizeof(LeafHeader) + index * sizeof(LeafSlot);
  auto endSlot = this->page.data.begin() + sizeof(LeafHeader) + (index+1) * sizeof(LeafSlot);
  this->page.data.erase(startSlot, endSlot);

  LeafHeader* header = reinterpret_cast<LeafHeader*>(this->page.data.data() + 0);
  header->itemCount = header->itemCount.value() - 1;
}

inline void LeafPage::delRangeLeaf(pagesize_t start, pagesize_t end) {
  assert(this->countLeaf() >= end);
  assert(end > start);

  for (pagesize_t i = end - 1; i >= start; i--) {
    LeafSlot* slot = reinterpret_cast<LeafSlot*>(this->page.data.data() + sizeof(LeafHeader) + i * sizeof(LeafSlot));
    auto startKey = this->page.data.begin() + slot->offset.value();
    auto endKey = startKey + slot->ksize.value() + slot->vsize.value();
    this->page.data.erase(startKey, endKey);
  }

  auto startSlot = this->page.data.begin() + sizeof(LeafHeader) + start * sizeof(LeafSlot);
  auto endSlot = this->page.data.begin() + sizeof(LeafHeader) + end * sizeof(LeafSlot);
  this->page.data.erase(startSlot, endSlot);

  LeafHeader* header = reinterpret_cast<LeafHeader*>(this->page.data.data() + 0);
  header->itemCount = header->itemCount.value() - (end - start);
}