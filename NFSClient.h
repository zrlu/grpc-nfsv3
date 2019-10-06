#include <iostream>
#include <string>

#include <sys/types.h>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#ifdef BAZEL_BUILD
#include "examples/protos/nfs.grpc.pb.h"
#else
#include "nfs.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using nfs::NULLargs;
using nfs::NULLres;

using nfs::MKNODargs;
using nfs::MKNODres;

using nfs::NFS;

class NFSClient
{
public:
  NFSClient(std::shared_ptr<Channel> channel);

  void NFSPROC_NULL();
  int NFSPROC_MKNOD(const char *, mode_t, dev_t);

private:
  std::unique_ptr<NFS::Stub> stub_;
};