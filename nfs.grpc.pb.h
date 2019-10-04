// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: nfs.proto
// Original file comments:
// Copyright 2015 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef GRPC_nfs_2eproto__INCLUDED
#define GRPC_nfs_2eproto__INCLUDED

#include "nfs.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/completion_queue.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/impl/codegen/stub_options.h>
#include <grpcpp/impl/codegen/sync_stream.h>

namespace grpc_impl {
class CompletionQueue;
class ServerCompletionQueue;
class ServerContext;
}  // namespace grpc_impl

namespace grpc {
namespace experimental {
template <typename RequestT, typename ResponseT>
class MessageAllocator;
}  // namespace experimental
}  // namespace grpc

namespace nfs {

// Interface exported by the server.
class NFS final {
 public:
  static constexpr char const* service_full_name() {
    return "nfs.NFS";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status NFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::nfs::VOIDRES* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfs::VOIDRES>> AsyncNFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfs::VOIDRES>>(AsyncNFSPROC3_NULLRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfs::VOIDRES>> PrepareAsyncNFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfs::VOIDRES>>(PrepareAsyncNFSPROC3_NULLRaw(context, request, cq));
    }
    class experimental_async_interface {
     public:
      virtual ~experimental_async_interface() {}
      virtual void NFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS* request, ::nfs::VOIDRES* response, std::function<void(::grpc::Status)>) = 0;
      virtual void NFSPROC3_NULL(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::nfs::VOIDRES* response, std::function<void(::grpc::Status)>) = 0;
      virtual void NFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS* request, ::nfs::VOIDRES* response, ::grpc::experimental::ClientUnaryReactor* reactor) = 0;
      virtual void NFSPROC3_NULL(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::nfs::VOIDRES* response, ::grpc::experimental::ClientUnaryReactor* reactor) = 0;
    };
    virtual class experimental_async_interface* experimental_async() { return nullptr; }
  private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::nfs::VOIDRES>* AsyncNFSPROC3_NULLRaw(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::nfs::VOIDRES>* PrepareAsyncNFSPROC3_NULLRaw(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    ::grpc::Status NFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::nfs::VOIDRES* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfs::VOIDRES>> AsyncNFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfs::VOIDRES>>(AsyncNFSPROC3_NULLRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfs::VOIDRES>> PrepareAsyncNFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfs::VOIDRES>>(PrepareAsyncNFSPROC3_NULLRaw(context, request, cq));
    }
    class experimental_async final :
      public StubInterface::experimental_async_interface {
     public:
      void NFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS* request, ::nfs::VOIDRES* response, std::function<void(::grpc::Status)>) override;
      void NFSPROC3_NULL(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::nfs::VOIDRES* response, std::function<void(::grpc::Status)>) override;
      void NFSPROC3_NULL(::grpc::ClientContext* context, const ::nfs::VOIDARGS* request, ::nfs::VOIDRES* response, ::grpc::experimental::ClientUnaryReactor* reactor) override;
      void NFSPROC3_NULL(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::nfs::VOIDRES* response, ::grpc::experimental::ClientUnaryReactor* reactor) override;
     private:
      friend class Stub;
      explicit experimental_async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class experimental_async_interface* experimental_async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class experimental_async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::nfs::VOIDRES>* AsyncNFSPROC3_NULLRaw(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::nfs::VOIDRES>* PrepareAsyncNFSPROC3_NULLRaw(::grpc::ClientContext* context, const ::nfs::VOIDARGS& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_NFSPROC3_NULL_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status NFSPROC3_NULL(::grpc::ServerContext* context, const ::nfs::VOIDARGS* request, ::nfs::VOIDRES* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_NFSPROC3_NULL : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_NFSPROC3_NULL() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_NFSPROC3_NULL() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status NFSPROC3_NULL(::grpc::ServerContext* /*context*/, const ::nfs::VOIDARGS* /*request*/, ::nfs::VOIDRES* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestNFSPROC3_NULL(::grpc::ServerContext* context, ::nfs::VOIDARGS* request, ::grpc::ServerAsyncResponseWriter< ::nfs::VOIDRES>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_NFSPROC3_NULL<Service > AsyncService;
  template <class BaseClass>
  class ExperimentalWithCallbackMethod_NFSPROC3_NULL : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithCallbackMethod_NFSPROC3_NULL() {
      ::grpc::Service::experimental().MarkMethodCallback(0,
        new ::grpc_impl::internal::CallbackUnaryHandler< ::nfs::VOIDARGS, ::nfs::VOIDRES>(
          [this](::grpc::ServerContext* context,
                 const ::nfs::VOIDARGS* request,
                 ::nfs::VOIDRES* response,
                 ::grpc::experimental::ServerCallbackRpcController* controller) {
                   return this->NFSPROC3_NULL(context, request, response, controller);
                 }));
    }
    void SetMessageAllocatorFor_NFSPROC3_NULL(
        ::grpc::experimental::MessageAllocator< ::nfs::VOIDARGS, ::nfs::VOIDRES>* allocator) {
      static_cast<::grpc_impl::internal::CallbackUnaryHandler< ::nfs::VOIDARGS, ::nfs::VOIDRES>*>(
          ::grpc::Service::experimental().GetHandler(0))
              ->SetMessageAllocator(allocator);
    }
    ~ExperimentalWithCallbackMethod_NFSPROC3_NULL() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status NFSPROC3_NULL(::grpc::ServerContext* /*context*/, const ::nfs::VOIDARGS* /*request*/, ::nfs::VOIDRES* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual void NFSPROC3_NULL(::grpc::ServerContext* /*context*/, const ::nfs::VOIDARGS* /*request*/, ::nfs::VOIDRES* /*response*/, ::grpc::experimental::ServerCallbackRpcController* controller) { controller->Finish(::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "")); }
  };
  typedef ExperimentalWithCallbackMethod_NFSPROC3_NULL<Service > ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_NFSPROC3_NULL : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_NFSPROC3_NULL() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_NFSPROC3_NULL() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status NFSPROC3_NULL(::grpc::ServerContext* /*context*/, const ::nfs::VOIDARGS* /*request*/, ::nfs::VOIDRES* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_NFSPROC3_NULL : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_NFSPROC3_NULL() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_NFSPROC3_NULL() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status NFSPROC3_NULL(::grpc::ServerContext* /*context*/, const ::nfs::VOIDARGS* /*request*/, ::nfs::VOIDRES* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestNFSPROC3_NULL(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class ExperimentalWithRawCallbackMethod_NFSPROC3_NULL : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithRawCallbackMethod_NFSPROC3_NULL() {
      ::grpc::Service::experimental().MarkMethodRawCallback(0,
        new ::grpc_impl::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
          [this](::grpc::ServerContext* context,
                 const ::grpc::ByteBuffer* request,
                 ::grpc::ByteBuffer* response,
                 ::grpc::experimental::ServerCallbackRpcController* controller) {
                   this->NFSPROC3_NULL(context, request, response, controller);
                 }));
    }
    ~ExperimentalWithRawCallbackMethod_NFSPROC3_NULL() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status NFSPROC3_NULL(::grpc::ServerContext* /*context*/, const ::nfs::VOIDARGS* /*request*/, ::nfs::VOIDRES* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual void NFSPROC3_NULL(::grpc::ServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/, ::grpc::experimental::ServerCallbackRpcController* controller) { controller->Finish(::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "")); }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_NFSPROC3_NULL : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_NFSPROC3_NULL() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler< ::nfs::VOIDARGS, ::nfs::VOIDRES>(std::bind(&WithStreamedUnaryMethod_NFSPROC3_NULL<BaseClass>::StreamedNFSPROC3_NULL, this, std::placeholders::_1, std::placeholders::_2)));
    }
    ~WithStreamedUnaryMethod_NFSPROC3_NULL() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status NFSPROC3_NULL(::grpc::ServerContext* /*context*/, const ::nfs::VOIDARGS* /*request*/, ::nfs::VOIDRES* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedNFSPROC3_NULL(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::nfs::VOIDARGS,::nfs::VOIDRES>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_NFSPROC3_NULL<Service > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_NFSPROC3_NULL<Service > StreamedService;
};

}  // namespace nfs


#endif  // GRPC_nfs_2eproto__INCLUDED
