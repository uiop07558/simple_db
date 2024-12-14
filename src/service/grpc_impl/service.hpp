#pragma once

#include <exception>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "src/engine/pager/transactional_pager.hpp"
#include "src/service/table/table.hpp"
#include "src/service/table/metatable.hpp"
#include "src/service/grpc/definition.grpc.pb.h"
#include "src/service/grpc/definition.pb.h"

using std::exception;

using namespace grpc;
// using namespace objectstore;

class ObjectStoreImpl final: public objectstore::ObjectStore::Service {
 private:
  TransactionalPager& thePager;
 public:
  Status CreateTable(ServerContext* ctx, const objectstore::CreateTableRequest* request, objectstore::CreateTableResponse* response) override {
    try {
      txid_t txid = thePager.startTransaction(true, "__meta");
      auto pager = thePager.getLocal(txid);

      Metatable metatable(pager);
      
      Table newTable = Table::createNewTable(pager, metatable, request->tableName(), request->fields());
      thePager.commit(txid);

      return Status(StatusCode::OK, "ok");
    }
    catch (const exception& e) {
      return Status(StatusCode::Internal, e.what());
    }
  }

  
  Status GetTableInfo(ServerContext* ctx, const GetTableInfoRequest* request, GetTableInfoResponse* response) override {
    try {
      txid_t txid = thePager.startTransaction(false, "__meta");
      auto pager = thePager.getLocal(txid);

      Metatable metatable(pager);

      TableMetadata metadata = metatable.search(request->tableName()).value();

      for (int i = 0; i < metadata.fields.size(); i++) {
        objectstore::FieldDef fieldDef;
        fieldDef.set_name(metadata.fields[i].name);
        response->mutable_fields()->Add(fieldDef);
      }

      return Status(StatusCode::OK, "ok");
    }
    catch (const exception& e) {
      return Status(StatusCode::INTERNAL, e.what());
    }
  }
};
