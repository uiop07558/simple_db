// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: definition.proto
#ifndef GRPC_definition_2eproto__INCLUDED
#define GRPC_definition_2eproto__INCLUDED

#include "definition.pb.h"

#include <functional>
#include <grpcpp/generic/async_generic_service.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/client_context.h>
#include <grpcpp/completion_queue.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/rpc_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/support/stub_options.h>
#include <grpcpp/support/sync_stream.h>

namespace objectstore {

class ObjectStore final {
 public:
  static constexpr char const* service_full_name() {
    return "objectstore.ObjectStore";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status CreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::objectstore::CreateTableResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::CreateTableResponse>> AsyncCreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::CreateTableResponse>>(AsyncCreateTableRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::CreateTableResponse>> PrepareAsyncCreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::CreateTableResponse>>(PrepareAsyncCreateTableRaw(context, request, cq));
    }
    virtual ::grpc::Status GetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::objectstore::GetTableInfoResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::GetTableInfoResponse>> AsyncGetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::GetTableInfoResponse>>(AsyncGetTableInfoRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::GetTableInfoResponse>> PrepareAsyncGetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::GetTableInfoResponse>>(PrepareAsyncGetTableInfoRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>> Transaction(::grpc::ClientContext* context) {
      return std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>>(TransactionRaw(context));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>> AsyncTransaction(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>>(AsyncTransactionRaw(context, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>> PrepareAsyncTransaction(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>>(PrepareAsyncTransactionRaw(context, cq));
    }
    class async_interface {
     public:
      virtual ~async_interface() {}
      virtual void CreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest* request, ::objectstore::CreateTableResponse* response, std::function<void(::grpc::Status)>) = 0;
      virtual void CreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest* request, ::objectstore::CreateTableResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      virtual void GetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest* request, ::objectstore::GetTableInfoResponse* response, std::function<void(::grpc::Status)>) = 0;
      virtual void GetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest* request, ::objectstore::GetTableInfoResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      virtual void Transaction(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::objectstore::TransactionRequest,::objectstore::TransactionResponse>* reactor) = 0;
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::CreateTableResponse>* AsyncCreateTableRaw(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::CreateTableResponse>* PrepareAsyncCreateTableRaw(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::GetTableInfoResponse>* AsyncGetTableInfoRaw(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::objectstore::GetTableInfoResponse>* PrepareAsyncGetTableInfoRaw(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientReaderWriterInterface< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>* TransactionRaw(::grpc::ClientContext* context) = 0;
    virtual ::grpc::ClientAsyncReaderWriterInterface< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>* AsyncTransactionRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) = 0;
    virtual ::grpc::ClientAsyncReaderWriterInterface< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>* PrepareAsyncTransactionRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    ::grpc::Status CreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::objectstore::CreateTableResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::objectstore::CreateTableResponse>> AsyncCreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::objectstore::CreateTableResponse>>(AsyncCreateTableRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::objectstore::CreateTableResponse>> PrepareAsyncCreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::objectstore::CreateTableResponse>>(PrepareAsyncCreateTableRaw(context, request, cq));
    }
    ::grpc::Status GetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::objectstore::GetTableInfoResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::objectstore::GetTableInfoResponse>> AsyncGetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::objectstore::GetTableInfoResponse>>(AsyncGetTableInfoRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::objectstore::GetTableInfoResponse>> PrepareAsyncGetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::objectstore::GetTableInfoResponse>>(PrepareAsyncGetTableInfoRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientReaderWriter< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>> Transaction(::grpc::ClientContext* context) {
      return std::unique_ptr< ::grpc::ClientReaderWriter< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>>(TransactionRaw(context));
    }
    std::unique_ptr<  ::grpc::ClientAsyncReaderWriter< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>> AsyncTransaction(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriter< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>>(AsyncTransactionRaw(context, cq, tag));
    }
    std::unique_ptr<  ::grpc::ClientAsyncReaderWriter< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>> PrepareAsyncTransaction(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriter< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>>(PrepareAsyncTransactionRaw(context, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void CreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest* request, ::objectstore::CreateTableResponse* response, std::function<void(::grpc::Status)>) override;
      void CreateTable(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest* request, ::objectstore::CreateTableResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
      void GetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest* request, ::objectstore::GetTableInfoResponse* response, std::function<void(::grpc::Status)>) override;
      void GetTableInfo(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest* request, ::objectstore::GetTableInfoResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
      void Transaction(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::objectstore::TransactionRequest,::objectstore::TransactionResponse>* reactor) override;
     private:
      friend class Stub;
      explicit async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class async* async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::objectstore::CreateTableResponse>* AsyncCreateTableRaw(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::objectstore::CreateTableResponse>* PrepareAsyncCreateTableRaw(::grpc::ClientContext* context, const ::objectstore::CreateTableRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::objectstore::GetTableInfoResponse>* AsyncGetTableInfoRaw(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::objectstore::GetTableInfoResponse>* PrepareAsyncGetTableInfoRaw(::grpc::ClientContext* context, const ::objectstore::GetTableInfoRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientReaderWriter< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>* TransactionRaw(::grpc::ClientContext* context) override;
    ::grpc::ClientAsyncReaderWriter< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>* AsyncTransactionRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) override;
    ::grpc::ClientAsyncReaderWriter< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>* PrepareAsyncTransactionRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_CreateTable_;
    const ::grpc::internal::RpcMethod rpcmethod_GetTableInfo_;
    const ::grpc::internal::RpcMethod rpcmethod_Transaction_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status CreateTable(::grpc::ServerContext* context, const ::objectstore::CreateTableRequest* request, ::objectstore::CreateTableResponse* response);
    virtual ::grpc::Status GetTableInfo(::grpc::ServerContext* context, const ::objectstore::GetTableInfoRequest* request, ::objectstore::GetTableInfoResponse* response);
    virtual ::grpc::Status Transaction(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::objectstore::TransactionResponse, ::objectstore::TransactionRequest>* stream);
  };
  template <class BaseClass>
  class WithAsyncMethod_CreateTable : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_CreateTable() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_CreateTable() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status CreateTable(::grpc::ServerContext* /*context*/, const ::objectstore::CreateTableRequest* /*request*/, ::objectstore::CreateTableResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestCreateTable(::grpc::ServerContext* context, ::objectstore::CreateTableRequest* request, ::grpc::ServerAsyncResponseWriter< ::objectstore::CreateTableResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_GetTableInfo : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_GetTableInfo() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_GetTableInfo() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetTableInfo(::grpc::ServerContext* /*context*/, const ::objectstore::GetTableInfoRequest* /*request*/, ::objectstore::GetTableInfoResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetTableInfo(::grpc::ServerContext* context, ::objectstore::GetTableInfoRequest* request, ::grpc::ServerAsyncResponseWriter< ::objectstore::GetTableInfoResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_Transaction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_Transaction() {
      ::grpc::Service::MarkMethodAsync(2);
    }
    ~WithAsyncMethod_Transaction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Transaction(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::objectstore::TransactionResponse, ::objectstore::TransactionRequest>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestTransaction(::grpc::ServerContext* context, ::grpc::ServerAsyncReaderWriter< ::objectstore::TransactionResponse, ::objectstore::TransactionRequest>* stream, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncBidiStreaming(2, context, stream, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_CreateTable<WithAsyncMethod_GetTableInfo<WithAsyncMethod_Transaction<Service > > > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_CreateTable : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_CreateTable() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::objectstore::CreateTableRequest, ::objectstore::CreateTableResponse>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::objectstore::CreateTableRequest* request, ::objectstore::CreateTableResponse* response) { return this->CreateTable(context, request, response); }));}
    void SetMessageAllocatorFor_CreateTable(
        ::grpc::MessageAllocator< ::objectstore::CreateTableRequest, ::objectstore::CreateTableResponse>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::objectstore::CreateTableRequest, ::objectstore::CreateTableResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_CreateTable() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status CreateTable(::grpc::ServerContext* /*context*/, const ::objectstore::CreateTableRequest* /*request*/, ::objectstore::CreateTableResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* CreateTable(
      ::grpc::CallbackServerContext* /*context*/, const ::objectstore::CreateTableRequest* /*request*/, ::objectstore::CreateTableResponse* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_GetTableInfo : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_GetTableInfo() {
      ::grpc::Service::MarkMethodCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::objectstore::GetTableInfoRequest, ::objectstore::GetTableInfoResponse>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::objectstore::GetTableInfoRequest* request, ::objectstore::GetTableInfoResponse* response) { return this->GetTableInfo(context, request, response); }));}
    void SetMessageAllocatorFor_GetTableInfo(
        ::grpc::MessageAllocator< ::objectstore::GetTableInfoRequest, ::objectstore::GetTableInfoResponse>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(1);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::objectstore::GetTableInfoRequest, ::objectstore::GetTableInfoResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_GetTableInfo() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetTableInfo(::grpc::ServerContext* /*context*/, const ::objectstore::GetTableInfoRequest* /*request*/, ::objectstore::GetTableInfoResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* GetTableInfo(
      ::grpc::CallbackServerContext* /*context*/, const ::objectstore::GetTableInfoRequest* /*request*/, ::objectstore::GetTableInfoResponse* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_Transaction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_Transaction() {
      ::grpc::Service::MarkMethodCallback(2,
          new ::grpc::internal::CallbackBidiHandler< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>(
            [this](
                   ::grpc::CallbackServerContext* context) { return this->Transaction(context); }));
    }
    ~WithCallbackMethod_Transaction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Transaction(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::objectstore::TransactionResponse, ::objectstore::TransactionRequest>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerBidiReactor< ::objectstore::TransactionRequest, ::objectstore::TransactionResponse>* Transaction(
      ::grpc::CallbackServerContext* /*context*/)
      { return nullptr; }
  };
  typedef WithCallbackMethod_CreateTable<WithCallbackMethod_GetTableInfo<WithCallbackMethod_Transaction<Service > > > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_CreateTable : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_CreateTable() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_CreateTable() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status CreateTable(::grpc::ServerContext* /*context*/, const ::objectstore::CreateTableRequest* /*request*/, ::objectstore::CreateTableResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_GetTableInfo : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_GetTableInfo() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_GetTableInfo() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetTableInfo(::grpc::ServerContext* /*context*/, const ::objectstore::GetTableInfoRequest* /*request*/, ::objectstore::GetTableInfoResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_Transaction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_Transaction() {
      ::grpc::Service::MarkMethodGeneric(2);
    }
    ~WithGenericMethod_Transaction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Transaction(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::objectstore::TransactionResponse, ::objectstore::TransactionRequest>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_CreateTable : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_CreateTable() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_CreateTable() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status CreateTable(::grpc::ServerContext* /*context*/, const ::objectstore::CreateTableRequest* /*request*/, ::objectstore::CreateTableResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestCreateTable(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_GetTableInfo : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_GetTableInfo() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_GetTableInfo() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetTableInfo(::grpc::ServerContext* /*context*/, const ::objectstore::GetTableInfoRequest* /*request*/, ::objectstore::GetTableInfoResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetTableInfo(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_Transaction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_Transaction() {
      ::grpc::Service::MarkMethodRaw(2);
    }
    ~WithRawMethod_Transaction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Transaction(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::objectstore::TransactionResponse, ::objectstore::TransactionRequest>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestTransaction(::grpc::ServerContext* context, ::grpc::ServerAsyncReaderWriter< ::grpc::ByteBuffer, ::grpc::ByteBuffer>* stream, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncBidiStreaming(2, context, stream, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_CreateTable : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_CreateTable() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->CreateTable(context, request, response); }));
    }
    ~WithRawCallbackMethod_CreateTable() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status CreateTable(::grpc::ServerContext* /*context*/, const ::objectstore::CreateTableRequest* /*request*/, ::objectstore::CreateTableResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* CreateTable(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_GetTableInfo : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_GetTableInfo() {
      ::grpc::Service::MarkMethodRawCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->GetTableInfo(context, request, response); }));
    }
    ~WithRawCallbackMethod_GetTableInfo() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetTableInfo(::grpc::ServerContext* /*context*/, const ::objectstore::GetTableInfoRequest* /*request*/, ::objectstore::GetTableInfoResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* GetTableInfo(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_Transaction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_Transaction() {
      ::grpc::Service::MarkMethodRawCallback(2,
          new ::grpc::internal::CallbackBidiHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context) { return this->Transaction(context); }));
    }
    ~WithRawCallbackMethod_Transaction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Transaction(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::objectstore::TransactionResponse, ::objectstore::TransactionRequest>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerBidiReactor< ::grpc::ByteBuffer, ::grpc::ByteBuffer>* Transaction(
      ::grpc::CallbackServerContext* /*context*/)
      { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_CreateTable : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_CreateTable() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::objectstore::CreateTableRequest, ::objectstore::CreateTableResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::objectstore::CreateTableRequest, ::objectstore::CreateTableResponse>* streamer) {
                       return this->StreamedCreateTable(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_CreateTable() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status CreateTable(::grpc::ServerContext* /*context*/, const ::objectstore::CreateTableRequest* /*request*/, ::objectstore::CreateTableResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedCreateTable(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::objectstore::CreateTableRequest,::objectstore::CreateTableResponse>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_GetTableInfo : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_GetTableInfo() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler<
          ::objectstore::GetTableInfoRequest, ::objectstore::GetTableInfoResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::objectstore::GetTableInfoRequest, ::objectstore::GetTableInfoResponse>* streamer) {
                       return this->StreamedGetTableInfo(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_GetTableInfo() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status GetTableInfo(::grpc::ServerContext* /*context*/, const ::objectstore::GetTableInfoRequest* /*request*/, ::objectstore::GetTableInfoResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedGetTableInfo(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::objectstore::GetTableInfoRequest,::objectstore::GetTableInfoResponse>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_CreateTable<WithStreamedUnaryMethod_GetTableInfo<Service > > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_CreateTable<WithStreamedUnaryMethod_GetTableInfo<Service > > StreamedService;
};

}  // namespace objectstore


#endif  // GRPC_definition_2eproto__INCLUDED
