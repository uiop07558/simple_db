#include <utility>

#include "bptree.hpp"
#include "../pager/pager.hpp"
#include "../page/page.hpp"

using std::nullopt;
using std::swap;
using std::max;
using std::min;

Bptree::Bptree(Pager& pager, pageptr_t rootId): rootId(rootId), pager(pager) {}

Bptree Bptree::createTree(Pager& pager) {
  Page leafPage = Page::createLeaf();
  pageptr_t rootId = pager.addPage(leafPage);
  return Bptree(pager, rootId);
}

void Bptree::insert(const vector<byte> &key, const vector<byte> &value) {
  pageptr_t newId = 0;
  bool isSplit = false;
  vector<byte> oldRootKey;
  vector<byte> splitKey;
  pageptr_t splitId = 0;

  this->insertRecursive(this->rootId, key, value, newId, isSplit, splitKey, oldRootKey, splitId);

  if (isSplit) {
    Page newRootPage = Page::createInternal();
    InternalPage newRoot(newRootPage);

    newRoot.putInternal(splitKey, splitId);
    newRoot.putInternal(oldRootKey, newId);

    this->pager.delPage(this->rootId);
    this->rootId = this->pager.addPage(newRoot.page);
  }
  else {
    this->rootId = newId;
  }
}

void Bptree::remove(const vector<byte> &key) {
  pageptr_t newId = 0;

  deleteRecursive(rootId, key, newId);
  if (newId == 0) {
    return;
  }

  Page page = this->pager.getPage(newId);
  if (page.getPageType() == PageType::Internal) {
    InternalPage root(page);
    pagesize_t rootCount = root.countInternal();
    pageptr_t newRootId = 0;
    if (rootCount == 1) {
      newRootId = root.getPageptr(0);
      assert(newRootId != 0);
      this->pager.delPage(newId);
      newId = newRootId;
    }
  }
  this->rootId = newId;
}

optional<vector<byte>> Bptree::search(const vector<byte>& key) const {
  return this->searchRecursive(this->rootId, key);
}

optional<vector<byte>> Bptree::searchRecursive(pageptr_t pageId, const std::vector<byte>& key) const {
  Page page = pager.getPage(pageId);
  
  switch (page.getPageType()) {
    case PageType::Leaf: {
      LeafPage leaf(page);
      int32_t index = leaf.searchLeaf(key);
      if (index != -1) {
        vector<byte> value = leaf.getValue(index).toVector();
        return value;
      }
      return nullopt;
    }
    case PageType::Internal: {
      InternalPage internal(page);
      int32_t childIndex = internal.searchInternal(key);
      if (childIndex == -1) {
        return nullopt;
      }
      pageptr_t childId = internal.getPageptr(childIndex);

      return searchRecursive(childId, key);
    }
    default: {
      assert(false && "deleteRecursive() got page of wrong type");
      return nullopt;
    }
  }
}

void Bptree::insertRecursive(pageptr_t pageId, const vector<byte>& key, const vector<byte> &value, 
    pageptr_t& newId, bool& isSplit, vector<byte>& splitKey, vector<byte>& oldRootKey, pageptr_t& splitId) {
  auto page = this->pager.getPage(pageId);
  
  switch (page.getPageType()) {
    case PageType::Leaf: {
      LeafPage leaf(page);
      leaf.putLeaf(key, value);
      oldRootKey = leaf.getKeyLeaf(0).toVector();

      isSplit = false;
      if (leaf.page.isOversized()) {
        auto newPage = Page::createLeaf();
        LeafPage newLeaf(newPage);
        pagesize_t oldCount = leaf.countLeaf();
        assert(oldCount >= 2);

        pagesize_t midIndex = oldCount >> 1;
        for (pagesize_t i = midIndex; i < oldCount; i++) {
          unsafe_buf<byte> k = leaf.getKeyLeaf(i);
          unsafe_buf<byte> v = leaf.getValue(i);
          newLeaf.putLeaf(k, v);
        }

        leaf.delRangeLeaf(midIndex, oldCount);

        assert(newLeaf.countLeaf() >= 0);
        splitKey = newLeaf.getKeyLeaf(0).toVector();
        isSplit = true;
        splitId = this->pager.addPage(newLeaf.page);
      }
      this->pager.delPage(pageId);
      newId = this->pager.addPage(leaf.page);

      break;
    };
    case PageType::Internal: {
      InternalPage internal(page);
      int32_t insertToIdx = internal.searchInternal(key);
      pageptr_t insertId = internal.getPageptr(insertToIdx);

      pageptr_t childNewId = 0;
      bool isChildSplit = false;
      vector<byte> childSplitKey;
      pageptr_t childSplitId = 0;

      insertRecursive(insertId, key, value, childNewId, isChildSplit, childSplitKey, oldRootKey, childSplitId);

      assert(insertToIdx >= 0);
      internal.setGEptr(insertToIdx, childNewId);
      if (isChildSplit) {
        internal.putInternal(childSplitKey, childSplitId);
      }
      oldRootKey = internal.getKeyInternal(0).toVector();

      isSplit = false;
      if (internal.page.isOversized()) {
        auto newPage = Page::createInternal();
        InternalPage newInternal(newPage);
        pagesize_t oldCount = internal.countInternal();
        assert(oldCount >= 2);
        
        pagesize_t midIndex = oldCount >> 1;

        unsafe_buf<byte> k = internal.getKeyInternal(midIndex);
        pageptr_t p = internal.getPageptr(midIndex);
        newInternal.putInternal(k, p);

        isSplit = true;
        splitKey = k.toVector();
        for (pagesize_t i = midIndex + 1; i < oldCount; i++) {
          unsafe_buf<byte> k = internal.getKeyInternal(i);
          pageptr_t p = internal.getPageptr(i);
          newInternal.putInternal(k, p);
        }

        internal.delRangeInternal(midIndex, oldCount);
        
        splitId = this->pager.addPage(newInternal.page);
      }
      this->pager.delPage(pageId);
      newId = this->pager.addPage(internal.page);

      break;
    }
    default: {
      assert(false && "insertRecursive() got page of wrong type");
      return;
    }
  }
}

void Bptree::deleteRecursive(pageptr_t pageId, const std::vector<byte>& key, pageptr_t& newId) {
  Page page = this->pager.getPage(pageId);

  switch (page.getPageType()) {
    case PageType::Leaf: {
      LeafPage leaf(page);
      int32_t index = leaf.searchLeaf(key);
      if (index != -1) {
        leaf.delLeaf(index);
        this->pager.delPage(pageId);
        newId = pager.addPage(leaf.page);
      }
      break;
    }
    case PageType::Internal: {
      InternalPage internal(page);
      int32_t childIndex = internal.searchInternal(key);
      pageptr_t childId = internal.getPageptr(childIndex);

      pageptr_t childNewId = 0;
      deleteRecursive(childId, key, childNewId);

      assert(childIndex >= 0);
      internal.setGEptr(childIndex, childNewId);

      Page childPage = this->pager.getPage(childNewId);
      bool haveSiblings = childIndex >= 0 && internal.countInternal() >= 2;

      if (childPage.isUndersized() && haveSiblings) {
        int32_t siblingIndex = 0;
        if (childIndex == internal.countInternal() - 1) {
          assert(childIndex != -1);
          siblingIndex = childIndex - 1;
        }
        else {
          siblingIndex = childIndex + 1;
        }

        pageptr_t siblingId = internal.getPageptr(siblingIndex);  
        Page siblingPage = this->pager.getPage(siblingId);

        if (childPage.byteSize() + siblingPage.byteSize() < PAGE_SIZE) { // merging
          switch (childPage.getPageType()) {
            case PageType::Leaf: {
              LeafPage child(childPage);
              LeafPage sibling(siblingPage);

              for (pagesize_t i = 0; i < sibling.countLeaf(); i++) {
                unsafe_buf<byte> k = sibling.getKeyLeaf(i);
                unsafe_buf<byte> v = sibling.getValue(i);
                child.putLeaf(k, v);
              }

              break;
            }
            case PageType::Internal: {
              InternalPage child(childPage);
              InternalPage sibling(siblingPage);

              for (pagesize_t i = 0; i < sibling.countInternal(); i++) {
                unsafe_buf<byte> k = sibling.getKeyInternal(i);
                pageptr_t p = sibling.getPageptr(i);
                child.putInternal(k, p);
              }

              break;
            }
            default: {
              assert(false && "deleteRecursive() got page of wrong type");
            }
          }

          vector<byte> mergeKey;
          mergeKey = internal.getKeyInternal(min(siblingIndex, childIndex)).toVector();
          
          internal.delInternal(max(siblingIndex, childIndex));
          internal.delInternal(min(siblingIndex, childIndex));

          this->pager.delPage(childNewId);
          this->pager.delPage(siblingId);
          pageptr_t childNewIdAfterMerge = this->pager.addPage(childPage);
          internal.putInternal(mergeKey, childNewIdAfterMerge);
        }
      }
      
      this->pager.delPage(pageId);
      newId = this->pager.addPage(internal.page);
      break;
    }
    default: {
      assert(false && "deleteRecursive() got page of wrong type");
      return;
    }
  }
}

BptreeIterator::BptreeIterator(Bptree &bptree): bptree(bptree), endReached(false) {
  pageptr_t pageId = this->bptree.rootId;
  while (true) {
    pageStack.emplace(0, pageId);
    Page page = this->bptree.pager.getPage(pageId);
    if (page.getPageType() == PageType::Leaf) {
      break;
    }
    InternalPage internal(page);
    pageId = internal.getPageptr(0);
  }
}

inline bool BptreeIterator::hasNext() const {
  return !pageStack.empty();
}

pair<vector<byte>, vector<byte>> BptreeIterator::next() {
  assert("iterator's end reached" && !this->endReached);

  auto [currentIndex, currentPageId] = this->pageStack.top();
  this->pageStack.pop();

  Page page = this->bptree.pager.getPage(currentPageId);
  LeafPage leaf(page);

  assert(currentIndex < leaf.countLeaf());

  vector<byte> key = leaf.getKeyLeaf(currentIndex).toVector();
  vector<byte> value = leaf.getValue(currentIndex).toVector();

  if (currentIndex + 1 < leaf.countLeaf()) {
    pageStack.emplace(currentIndex + 1, currentPageId);
  }
  else {
    while (!this->pageStack.empty()) {
      auto [parentIndex, parentPageId] = this->pageStack.top();
      this->pageStack.pop();

      Page parentPage = this->bptree.pager.getPage(parentPageId);
      if (parentPage.getPageType() == PageType::Internal) {
        InternalPage internal(parentPage);

        if (parentIndex + 1 < internal.countInternal()) {
          pageStack.emplace(parentIndex + 1, parentPageId);
          pageptr_t nextPageId = internal.getPageptr(parentIndex + 1);

          while (true) {
            pageStack.emplace(0, nextPageId);
            Page page = this->bptree.pager.getPage(nextPageId);
            if (page.getPageType() == PageType::Leaf) {
              break;
            }
            InternalPage internal(page);
            nextPageId = internal.getPageptr(0);
          }

          break;
        }
      }
    }

    if (this->pageStack.empty()) {
      this->endReached = true;
    }
  }

  return {key, value};
}
