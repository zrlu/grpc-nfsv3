#pragma once

#include <iostream>
#include <string>
#include <map>
#include <set>

#include <sys/types.h>
#include <fuse.h>

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

#include "RPCManager.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using nfs::NFS;

#define NFSPROC_OK(x) x == 0
#define NFSPROC_RPC_ERROR(x) x > 0
#define NFSPROC_SYSCALL_ERROR(x) x < 0

class NFSClient
{  
  RPCManager m_rpc_mgr;
  std::shared_ptr<grpc::Channel> m_channel;
  std::unique_ptr<NFS::Stub> stub_;
  short m_client_id;

  template <typename T> T* make_rpc();
  bool del_rpc_if_ok(rpcid_t rpcid, const Status &status);
  

public:
  NFSClient(std::shared_ptr<Channel> channel);

  bool WaitForConnection();
  bool WaitForConnection(int64_t sec, int32_t nsec);

  int NFSPROC_NULL(void);
  int NFSPROC_GETATTR(const char *, struct stat *);
  int NFSPROC_MKNOD(const char *, mode_t, dev_t);
  int NFSPROC_MKDIR(const char *, mode_t);
  int NFSPROC_RMDIR(const char *);
  int NFSPROC_RENAME(const char *, const char *);
  int NFSPROC_OPEN(const char *, const struct fuse_file_info *, int *);
  int NFSPROC_RELEASE(const char *, const struct fuse_file_info *);
  int NFSPROC_READ(const char *, char *, size_t, off_t, const struct fuse_file_info *, ssize_t *);
  int NFSPROC_WRITE(const char *, const char *, size_t, off_t, const struct fuse_file_info *, ssize_t *);
  int NFSPROC_FGETATTR(const char *, struct stat *, const struct fuse_file_info *);
  int NFSPROC_READDIR(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);

};