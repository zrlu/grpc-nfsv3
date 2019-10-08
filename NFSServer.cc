#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>
#include <thread>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "NFSServer.h"
#include "helpers.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using nfs::NFS;
using nfs::Stat;
using nfs::TimeSpec;
using nfs::NULLargs;
using nfs::NULLres;
using nfs::GETATTRargs;
using nfs::GETATTRres;
using nfs::MKNODargs;
using nfs::MKNODres;
using nfs::OPENargs;
using nfs::OPENres;
using nfs::READargs;
using nfs::READres;
using nfs::WRITEargs;
using nfs::WRITEres;

using std::chrono::system_clock;

namespace fs = std::filesystem;

fs::path NFSImpl::fullpath(const std::string &suffix)
{
    auto fp = fs::path(m_serverStoragePath) / fs::path(suffix);
    return fs::weakly_canonical(fp);
}

NFSImpl::NFSImpl(const std::string &path) : m_serverStoragePath(path) {}

Status NFSImpl::NFSPROC_NULL(ServerContext *context, const NULLargs *request, NULLres *response)
{
  nfs::NULLres res;
  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_GETATTR(ServerContext *context, const GETATTRargs *request, GETATTRres *response)
{
  nfs::GETATTRres res;
  auto fp = fullpath(request->pathname());
  std::cerr << "GETATTR " << fp << std::endl;
  struct stat *statbuf = new struct stat;

  if (stat(fp.c_str(), statbuf) == -1)
  {
    res.set_syscall_errno(-errno);
    *response = res;
    return Status::OK;
  }

  Stat *stat = new Stat;
  copystat2Stat(*statbuf, stat);
  res.set_allocated_stat(stat);

  delete statbuf;

  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_MKNOD(ServerContext *context, const MKNODargs *request, MKNODres *response)
{
  nfs::MKNODres res;
  auto fp = fullpath(request->pathname());
  mode_t mode = request->mode();
  dev_t dev = request->dev();

  if (mknod(fp.c_str(), mode, dev) == -1) res.set_syscall_errno(-errno);

  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_OPEN(ServerContext *context, const OPENargs *request, OPENres *response)
{
  std::cerr << "OPEN" << std::endl;
  nfs::OPENres res;
  auto fp = fullpath(request->pathname());
  int oflag = request->oflag();
  int retval = open(fp.c_str(), oflag);

  if (retval == -1) res.set_syscall_errno(-errno);

  res.set_syscall_value(retval);
  *response = res;
  return Status::OK;

}

Status NFSImpl::NFSPROC_RELEASE(ServerContext *context, const RELEASEargs *request, RELEASEres *response)
{
  nfs::RELEASEres res;
  int fh = request->fh();
  
  if (close(fh) == -1) res.set_syscall_errno(-errno);

  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_READ(ServerContext *context, const READargs *request, ServerWriter<READres> *writer)
{
  std::cerr << "READ" << std::endl;
  nfs::READres res;
  int fh = request->fh();
  size_t size = request->size();
  off_t offset = request->offset();
  char *buffer = new char[size]; 
  bzero(buffer, size);

  int retval = lseek(fh, offset, SEEK_SET);

  if (retval == -1) {
    res.set_syscall_errno(-errno);
    goto NFSPROC_READ_reply;
  }

  retval = read(fh, buffer, size);

  if (retval == -1) res.set_syscall_errno(-errno);
  res.set_syscall_value(retval);
  res.set_data(buffer);

NFSPROC_READ_reply:

  writer->Write(res);
  
  delete buffer;
  return Status::OK;
}
