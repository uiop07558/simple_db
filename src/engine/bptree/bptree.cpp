#include <utility>

#include "bptree.hpp"
#include "../pager/pager.hpp"
#include "../page/page.hpp"

using std::nullopt;
using std::swap;

Bptree::Bptree(Pager& pager, pageptr_t rootId): pager(pager), rootId(rootId) {}

void Bptree::insert(const vector<byte> &key, const vector<byte> &value) {
  pageptr_t newId = 0;
  bool isSplit = false;
  vector<byte> splitKey;
  pageptr_t splitId = 0;

  this->insertRecursive(this->rootId, key, value, newId, isSplit, splitKey, splitId);

  if (isSplit) {
    auto newRootPage = Page::createInternal();
    InternalPage newRoot(newRootPage);

    newRoot.putInternal(splitKey, splitId);
    newRoot.setLptr(newId);

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
    if (rootCount == 0) {
      newRootId = root.getPageptr(-1);
      this->pager.delPage(newId);
      newId = newRootId;
    }
    else if (rootCount == 1 && root.getPageptr(-1) == 0) {
      for (pagesize_t i = 0; i < rootCount; i++) {
        newRootId = root.getPageptr(i);
        if (newRootId != 0) {
          break;
        }
      }
      assert(newRootId != 0);
      this->pager.delPage(newId);
      newId = newRootId;
    }
  }

  this->rootId = newId;

  return;
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
    pageptr_t& newId, bool& isSplit, vector<byte>& splitKey, pageptr_t& splitId) {
  auto page = this->pager.getPage(pageId);
  
  switch (page.getPageType()) {
    case PageType::Leaf: {
      LeafPage leaf(page);
      leaf.putLeaf(key, value);

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

      insertRecursive(insertId, key, value, childNewId, isChildSplit, childSplitKey, childSplitId);

      insertToIdx != -1 ? internal.setGEptr(insertToIdx, childNewId) : internal.setLptr(childNewId);
      if (isChildSplit) {
        internal.putInternal(childSplitKey, childSplitId);
      }

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

      childIndex != -1 ? internal.setGEptr(childIndex, childNewId) : internal.setLptr(childNewId);

      Page childPage = this->pager.getPage(childNewId);
      bool haveSiblings = (childIndex == -1 && internal.countInternal() >= 1) || (childIndex >= 0 && internal.countInternal() >= 2)
        || (childIndex >= 0 && internal.getPageptr(-1) != 0);

      if (childPage.isUndersized() && haveSiblings) {
        int32_t siblingIndex = 0;
        if (childIndex == internal.countInternal() - 1) {
          assert(childIndex != -1);
          siblingIndex = childIndex - 1;
        }
        else if (childIndex == 1 && internal.getPageptr(-1) != 0) {
          siblingIndex = -1;
        }
        else {
          siblingIndex = childIndex + 1;
        }

        pageptr_t siblingId = internal.getPageptr(siblingIndex);  
        Page siblingPage = this->pager.getPage(siblingId);

        if (childPage.byteSize() + siblingPage.byteSize() < MAX_PAGE_SIZE) { // merging
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

              if (sibling.getPageptr(-1) != 0) {
                unsafe_buf<byte> k = internal.getKeyInternal(siblingIndex);
                pageptr_t p = sibling.getPageptr(-1);
                child.putInternal(k, p);
              }

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

          if (siblingIndex < childIndex) {
            mergeKey = internal.getKeyInternal(siblingIndex).toVector();

            internal.delInternal(childIndex);
            internal.delInternal(siblingIndex);
          }
          else {
            assert(childIndex != siblingIndex);
            mergeKey = internal.getKeyInternal(childIndex).toVector();

            internal.delInternal(siblingIndex);
            internal.delInternal(childIndex);
          }

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
