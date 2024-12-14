#pragma once

#include <exception>
#include <stdexcept>
#include <nlohmann/json.hpp>

#include "src/engine/bptree/bptree.hpp"
#include "src/engine/pager/transactional_pager.hpp"
#include "src/service/table/metadata.hpp"

using std::out_of_range;

using nlohmann::json;

class Metatable {
 private:
  Pager& pager;
  Bptree bptree;
 public:
  Metatable(Pager& pager): pager(pager), bptree(Bptree(pager, pager.getMetaPage().getMetaTableRoot())) {}

  void insert(string id, TableMetadata value) {
    json ser = value;
    vector<byte> valueArr = json::to_cbor(ser);
    json serId = id;
    vector<byte> keyArr = json::to_cbor(serId);

    bptree.insert(keyArr, valueArr);
    MetaPage meta = pager.getMetaPage();
    meta.setMetaTableRoot(bptree.getRootId());
    pager.saveMetaPage(meta);
  }

  void remove(string id) {
    json serId = id;
    vector<byte> keyArr = json::to_cbor(serId);

    bptree.remove(keyArr);
    MetaPage meta = pager.getMetaPage();
    meta.setMetaTableRoot(bptree.getRootId());
    pager.saveMetaPage(meta);
  }

  optional<TableMetadata> search(string id) const {
    json serId = id;
    vector<byte> keyArr = json::to_cbor(serId);

    auto valArrOpt = bptree.search(keyArr);
    if (valArrOpt.has_value()) {
      json serMeta = json::from_cbor(valArrOpt.value());
      TableMetadata metaData = serMeta.template get<TableMetadata>();
      return metaData;
    }
    else {
      return nullopt;
    }
  }

  pageptr_t getTableRoot(string id) {
    auto metadataOpt = search(id);
    if (!metadataOpt.has_value()) {
      throw out_of_range("table metadata not found");
    }

    TableMetadata metadata = metadataOpt.value();
    return metadata.rootId;
  }

  void setTableRoot(string id, pageptr_t rootId) {
    auto metadataOpt = search(id);
    if (!metadataOpt.has_value()) {
      throw out_of_range("table metadata not found");
    }

    TableMetadata metadata = metadataOpt.value();
    metadata.rootId = rootId;
    insert(id, metadata);
  }
};
