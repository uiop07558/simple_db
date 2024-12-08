#pragma once

#include <optional>

#include "../pager/pager.hpp"
#include "../page/page.hpp"

using std::optional;

class Bptree {
 public:
  Bptree(Pager& pager, pageptr_t rootId);

  void insert(const vector<byte>& key, const vector<byte>& value);
  void remove(const vector<byte>& key);
  optional<vector<byte>> search(const vector<byte>& key) const;
 private:
  pageptr_t rootId;

  bool isSplit;

  Pager& pager;

  void insertRecursive(pageptr_t pageId, const vector<byte>& key, const vector<byte>& value,
    pageptr_t& newId, bool& isSplit, vector<byte>& splitKey, vector<byte>& oldRootKey, pageptr_t& splitId);
  optional<vector<byte>> searchRecursive(pageptr_t pageId, const std::vector<byte>& key) const;
  void deleteRecursive(pageptr_t pageId, const std::vector<byte>& key, pageptr_t& newId);
};
