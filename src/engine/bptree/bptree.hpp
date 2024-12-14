#pragma once

#include <optional>
#include <stack>
#include <functional>

#include "../pager/pager.hpp"
#include "../page/page.hpp"

using std::optional;
using std::stack;
using std::pair;
using std::reference_wrapper;
using std::swap;

class Bptree;
class BptreeIterator;

class BptreeIterator {
 public:
  BptreeIterator(Bptree& bptree);

  bool hasNext() const;
  pair<vector<byte>, vector<byte>> next();

 private:
  Bptree& bptree;

  stack<pair<pagesize_t, pageptr_t>, vector<pair<pagesize_t, pageptr_t>>> pageStack;
};

class Bptree {
 public:
  Bptree(Pager& pager, pageptr_t rootId);

  // Bptree(const Bptree&) = default;
  // Bptree operator=(const Bptree& tree) {
  //   return Bptree(tree)
  // }
  // Bptree(Bptree&&) = default;
  // Bptree operator=(Bptree&& tree) {
  //   return Bptree(tree);
  // }
  // ~Bptree() = default;

  static Bptree createTree(Pager& pager);

  pageptr_t getRootId() { return rootId; }

  void insert(const vector<byte>& key, const vector<byte>& value);
  void remove(const vector<byte>& key);
  optional<vector<byte>> search(const vector<byte>& key) const;

  BptreeIterator iterate() {
    return BptreeIterator(*this);
  }

  friend class BptreeIterator;
 private:
  pageptr_t rootId;

  bool isSplit;

  Pager& pager;

  void insertRecursive(pageptr_t pageId, const vector<byte>& key, const vector<byte>& value,
    pageptr_t& newId, bool& isSplit, vector<byte>& splitKey, vector<byte>& oldRootKey, pageptr_t& splitId);
  optional<vector<byte>> searchRecursive(pageptr_t pageId, const std::vector<byte>& key) const;
  void deleteRecursive(pageptr_t pageId, const std::vector<byte>& key, pageptr_t& newId);
};
