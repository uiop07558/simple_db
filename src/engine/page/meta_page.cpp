#include "meta_page.hpp"

pageptr_t MetaPage::getCursize() {
  assert(this->byteSize() >= sizeof(MetaPageData));
  MetaPageData* header = reinterpret_cast<MetaPageData*>(this->data.data() + 0);
  return header->curSize.value();
}

inline pageptr_t MetaPage::getMetaTableRoot() {
  assert(this->byteSize() >= sizeof(MetaPageData));
  MetaPageData* header = reinterpret_cast<MetaPageData*>(this->data.data() + 0);
  return header->metaTableRoot.value();
}

inline pageptr_t MetaPage::getFreeListHead() {
  assert(this->byteSize() >= sizeof(MetaPageData));
  MetaPageData* header = reinterpret_cast<MetaPageData*>(this->data.data() + 0);
  return header->freeListHead.value();
}

inline pageptr_t MetaPage::getFreeListTail() {
  assert(this->byteSize() >= sizeof(MetaPageData));
  MetaPageData* header = reinterpret_cast<MetaPageData*>(this->data.data() + 0);
  return header->freeListTail.value();
}

void MetaPage::setCursize(pageptr_t ptr) {
  assert(this->byteSize() >= sizeof(MetaPageData));
  MetaPageData* header = reinterpret_cast<MetaPageData*>(this->data.data() + 0);
  header->curSize = ptr;
}

inline void MetaPage::setMetaTableRoot(pageptr_t ptr) {
  assert(this->byteSize() >= sizeof(MetaPageData));
  MetaPageData* header = reinterpret_cast<MetaPageData*>(this->data.data() + 0);
  header->metaTableRoot = ptr;
}

inline void MetaPage::setFreeListHead(pageptr_t ptr) {
  assert(this->byteSize() >= sizeof(MetaPageData));
  MetaPageData* header = reinterpret_cast<MetaPageData*>(this->data.data() + 0);
  header->freeListHead = ptr;
}

inline void MetaPage::setFreeListTail(pageptr_t ptr) {
  assert(this->byteSize() >= sizeof(MetaPageData));
  MetaPageData* header = reinterpret_cast<MetaPageData*>(this->data.data() + 0);
  header->freeListTail = ptr;
}
