#pragma once

#include <iostream>
#include <string>

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
public:
  NFSClient(std::shared_ptr<Channel> channel);

  int NFSPROC_NULL(void);
  int NFSPROC_GETATTR(const char *, struct stat *);
  int NFSPROC_MKNOD(const char *, mode_t, dev_t);
  int NFSPROC_OPEN(const char *, const struct fuse_file_info *, int *);
  int NFSPROC_RELEASE(const char *, const struct fuse_file_info *);
  int NFSPROC_READ(const char *, char *, size_t, off_t, const struct fuse_file_info *, ssize_t *);
  int NFSPROC_WRITE(const char *, const char *, size_t, off_t, const struct fuse_file_info *, ssize_t *);
  int NFSPROC_FGETATTR(const char *, struct stat *, const struct fuse_file_info *);
  int NFSPROC_READDIR(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);

private:
  std::unique_ptr<NFS::Stub> stub_;
};