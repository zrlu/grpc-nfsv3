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

#define CHUNK_SIZE (1<<20)

#define ENABLE_DEBUG_RESPONSE 0
#if ENABLE_DEBUG_RESPONSE
#define DEBUG_RESPONSE(res) std::cerr << __func__ << ": " << res.ShortDebugString() << std::endl;
#else
#define DEBUG_RESPONSE(res)
#endif


NFSClient::NFSClient(std::shared_ptr<Channel> channel) : stub_(NFS::NewStub(channel)) {}

int NFSClient::NFSPROC_NULL(void)
{
  ClientContext context;
  nfs::NULLargs args;
  nfs::NULLres res;
  Status status = stub_->NFSPROC_NULL(&context, args, &res);
  DEBUG_RESPONSE(res);
  return status.error_code();
}

int NFSClient::NFSPROC_GETATTR(const char *pathname, struct stat *statbuf) {
  ClientContext context;
  nfs::GETATTRargs args;
  nfs::GETATTRres res;
  args.set_pathname(pathname);
  Status status = stub_->NFSPROC_GETATTR(&context, args, &res);
  DEBUG_RESPONSE(res);
  const nfs::Stat stat = res.stat();
  copyStat2stat(stat, statbuf);
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
  DEBUG_RESPONSE(res);
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
  DEBUG_RESPONSE(res);
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
  DEBUG_RESPONSE(res);
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

  int total_size_copied = 0;
  std::shared_ptr<ClientReader<nfs::READres>> stream(stub_->NFSPROC_READ(&context, args));
  while (stream->Read(&res)) {
    DEBUG_RESPONSE(res);
    if (res.syscall_errno() < 0) break;
    const int read_chunk_size = res.syscall_value();
    size_t size_copied = res.data().copy(buffer, read_chunk_size);
    total_size_copied += size_copied;
  };
  
  Status status = stream->Finish();
  if (status.ok() && res.syscall_errno() == 0) *ret = total_size_copied;
  if (res.syscall_errno() < 0) *ret = -1;
  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_FGETATTR(const char *pathname, struct stat *statbuf, const struct fuse_file_info *fi)
{
  ClientContext context;
  nfs::FGETATTRargs args;
  nfs::FGETATTRres res;
  args.set_fh(fi->fh);
  Status status = stub_->NFSPROC_FGETATTR(&context, args, &res);
  DEBUG_RESPONSE(res);
  const nfs::Stat stat = res.stat();
  copyStat2stat(stat, statbuf);
  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_READDIR(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  ClientContext context;
  nfs::READDIRargs args;
  nfs::READDIRres res;
  args.set_pathname(path);
  Status status = stub_->NFSPROC_READDIR(&context, args, &res);
  DEBUG_RESPONSE(res);
  int code = status.error_code() | res.syscall_errno();
  if (code) return code;

  int size = res.filename_size();
  for (int index = 0; index < size; ++index)
  {
    nfs::Stat stat_obj = res.stat(index);
    const std::string filename = res.filename(index);
    struct stat st;
    copyStat2stat(stat_obj, &st);
    if (filler(buf, filename.c_str(), &st, 0)) break;
  }
  return 0;
}