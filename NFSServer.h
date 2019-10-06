#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#ifdef BAZEL_BUILD
#include "examples/protos/nfs.grpc.pb.h"
#else
#include "nfs.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using nfs::NFS;
using nfs::NULLargs;
using nfs::NULLres;

using nfs::MKNODargs;
using nfs::MKNODres;

using std::chrono::system_clock;

class NFSImpl final : public NFS::Service
{
  std::mutex mu_;
  const std::string m_serverStoragePath;
  const char *getFullPath(const std::string &);
protected:
public:
  NFSImpl(const std::string &path);
  Status NFSPROC_NULL(ServerContext *, const NULLargs *, NULLres *) override;
  Status NFSPROC_MKNOD(ServerContext *, const MKNODargs *, MKNODres *) override;
};


