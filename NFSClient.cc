#include <fuse.h>

#include "NFSClient.h"
#include "helpers.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using grpc::StatusCode;
using nfs::NFS;
using nfs::Stat;
using nfs::NULLargs;
using nfs::NULLres;
using nfs::GETATTRargs;
using nfs::GETATTRres;
using nfs::MKNODargs;
using nfs::MKNODres;
using nfs::OPENargs;
using nfs::OPENres;
using nfs::RELEASEargs;
using nfs::RELEASEres;
using nfs::READargs;
using nfs::READres;
using nfs::WRITEargs;
using nfs::WRITEres;

NFSClient::NFSClient(std::shared_ptr<Channel> channel) : stub_(NFS::NewStub(channel)) {}

int NFSClient::NFSPROC_NULL(void)
{
  ClientContext context;
  nfs::NULLargs args;
  nfs::NULLres res;
  Status status = stub_->NFSPROC_NULL(&context, args, &res);
  std::cerr << status.error_message() << std::endl;
  return status.error_code();
}

int NFSClient::NFSPROC_GETATTR(const char *pathname, struct stat *statbuf) {
  ClientContext context;
  nfs::GETATTRargs args;
  nfs::GETATTRres res;
  args.set_pathname(pathname);
  Status status = stub_->NFSPROC_GETATTR(&context, args, &res);
  std::cerr << "rpc: " << status.error_code() << std::endl;
  std::cerr << "errno: " << res.syscall_errno() << std::endl;
  std::cerr << res.ShortDebugString() << std::endl;
  const Stat stat = res.stat();
  copyStat2stat(stat, statbuf);
  std::cerr << status.error_message() << status.error_details() << std::endl;
  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_MKNOD(const char *pathname, mode_t mode, dev_t dev)
{
  ClientContext context;
  nfs::MKNODargs args;
  nfs::MKNODres res;
  args.set_pathname(pathname);
  args.set_mode(mode);
  args.set_dev(dev);
  Status status = stub_->NFSPROC_MKNOD(&context, args, &res);
  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_OPEN(const char *pathname, const struct fuse_file_info *fi, int *ret)
{
  ClientContext context;
  nfs::OPENargs args;
  nfs::OPENres res;
  args.set_pathname(pathname);
  args.set_oflag(fi->flags);
  Status status = stub_->NFSPROC_OPEN(&context, args, &res);
  *ret = res.syscall_value();
  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_RELEASE(const char *pathname, const struct fuse_file_info *fi)
{
  ClientContext context;
  nfs::RELEASEargs args;
  nfs::RELEASEres res;
  args.set_fh(fi->fh);
  Status status = stub_->NFSPROC_RELEASE(&context, args, &res);
  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_READ(const char *pathname, char *buffer, size_t size, off_t offset, const struct fuse_file_info *fi, int *ret)
{
  ClientContext context;
  nfs::READargs args;
  nfs::READres res;
  args.set_fh(fi->fh);
  args.set_size(size);
  args.set_offset(offset);
  std::shared_ptr<ClientReader<nfs::READres>> stream(stub_->NFSPROC_READ(&context, args));
  // std::cerr << "before read" << std::endl;
  while (stream->Read(&res)) {};
  // std::cerr << "read done" << std::endl;
  Status status = stream->Finish();
  if (status.ok())
  {
    res.data().copy(buffer, size, 0);
    *ret = res.syscall_value();
    std::cerr << res.ShortDebugString() << std::endl;
  }
  return status.error_code() | res.syscall_errno();
}