#pragma once

#include <filesystem>

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

#include "RPCLogger.h"
#include "Logger.h"
#include <google/protobuf/message.h>
#include <queue>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using std::chrono::system_clock;

namespace fs = std::filesystem;

class NFSImpl final : public nfs::NFS::Service
{
  std::mutex mu_;
  const std::string m_server_storage_path;
  fs::path fullpath(const std::string &);
  RPCLogger m_rpc_logger;
  Logger m_open_fh;

  std::map<rpcid_t, nfs::WRITEargs> m_write_buffer;

  // the do_* functions are also used in recovery
  int do_MKNOD(const nfs::MKNODargs *);
  int do_MKDIR(const nfs::MKDIRargs *);
  int do_UNLINK(const nfs::UNLINKargs *);
  int do_RMDIR(const nfs::RMDIRargs *);
  int do_RENAME(const nfs::RENAMEargs *);
  int do_TRUNCATE(const nfs::TRUNCATEargs *);
  int do_OPEN(const nfs::OPENargs *);
  int do_RELEASE(const nfs::RELEASEargs *);
  long do_WRITE(const nfs::WRITEargs *);

  std::string serialize(const Message &msg);
  template <typename T> T deserialize(const string &str);

protected:
public:
  NFSImpl(const std::string &path);
  Status NFSPROC_NULL(ServerContext *, const nfs::NULLargs *, nfs::NULLres *) override;
  Status NFSPROC_GETATTR(ServerContext *, const nfs::GETATTRargs *, nfs::GETATTRres *) override;
  Status NFSPROC_MKNOD(ServerContext *, const nfs::MKNODargs *, nfs::MKNODres *) override;
  Status NFSPROC_MKDIR(ServerContext *, const nfs::MKDIRargs *, nfs::MKDIRres *) override;
  Status NFSPROC_UNLINK(ServerContext *, const nfs::UNLINKargs *, nfs::UNLINKres *) override;
  Status NFSPROC_RMDIR(ServerContext *, const nfs::RMDIRargs *, nfs::RMDIRres *) override;
  Status NFSPROC_RENAME(ServerContext *, const nfs::RENAMEargs *, nfs::RENAMEres *) override;
  Status NFSPROC_TRUNCATE(ServerContext *, const nfs::TRUNCATEargs *, nfs::TRUNCATEres *) override;
  Status NFSPROC_OPEN(ServerContext *, const nfs::OPENargs *, nfs::OPENres *) override;
  Status NFSPROC_RELEASE(ServerContext *, const nfs::RELEASEargs *, nfs::RELEASEres *) override;
  Status NFSPROC_READ(ServerContext *, const nfs::READargs *, nfs::READres *) override;
  Status NFSPROC_WRITE(ServerContext *, const nfs::WRITEargs *, nfs::WRITEres *) override;
  Status NFSPROC_FGETATTR(ServerContext *, const nfs::FGETATTRargs *, nfs::FGETATTRres *) override;
  Status NFSPROC_READDIR(ServerContext *, const nfs::READDIRargs *, nfs::READDIRres *) override;
  Status NFSPROC_COMMIT(ServerContext*, ServerReaderWriter<nfs::COMMITres, nfs::COMMITargs>*) override;
  Status CHECK_MISSING(ServerContext *, const nfs::COMMITargs *, nfs::ResendList *) override;
  Status WRITE_BUFFER_SYNC(ServerContext *, ServerReader<nfs::WRITEargs> *, nfs::SyncResponse *) override;
};


