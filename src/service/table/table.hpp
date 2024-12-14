#pragma once

#include "src/service/table/metatable.hpp"
#include "src/engine/bptree/bptree.hpp"

class Table {
 private:
  Pager& pager;
  Metatable& metatable;
  string tableId;
  Bptree bptree;
 public:
  Table(Pager& pager, Metatable& metatable, string tableId, pageptr_t rootId): 
    pager(pager), metatable(metatable), tableId(tableId), bptree(Bptree(pager, rootId)) {}

  static Table createNewTable(Pager& pager, Metatable& metatable, string tableId, vector<Field> fields) {
    auto bptree = Bptree::createTree(pager);
    TableMetadata newTableMetadata = {
      rootId: bptree.getRootId(),
      name: tableId,
      fields: fields
    };
    metatable.insert(tableId, newTableMetadata);
    return Table(pager, metatable, tableId, bptree.getRootId());
  }

  pageptr_t getRootId() { return bptree.getRootId(); }

  void insert(vector<byte> key, vector<byte> value) {
    bptree.insert(key, value);
    metatable.setTableRoot(tableId, bptree.getRootId());
  }

  void remove(vector<byte> key) {
    bptree.remove(key);
    metatable.setTableRoot(tableId, bptree.getRootId());
  }

  optional<vector<byte>> search(vector<byte> key) const {
    auto valOpt = bptree.search(key);

    if (valOpt.has_value()) {
      return valOpt.value();
    }
    else {
      return nullopt;
    }
  }
};