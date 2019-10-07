#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>

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

using std::chrono::system_clock;

namespace fs = std::filesystem;

fs::path NFSImpl::fullpath(const std::string &suffix)
{
    auto fp = fs::path(m_serverStoragePath) / fs::path(suffix);
    return fs::canonical(fp) ;
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
  struct stat statbuf;

  if (!~stat(fp.c_str(), &statbuf)) res.set_syscall_errno(-errno);

  Stat *stat = new Stat;
  copystat2Stat(statbuf, stat);
  res.set_allocated_stat(stat);
  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_MKNOD(ServerContext *context, const MKNODargs *request, MKNODres *response)
{
  nfs::MKNODres res;
  auto fp = fullpath(request->pathname());
  mode_t mode = request->mode();
  dev_t dev = request->dev();

  if (!~mknod(fp.c_str(), mode, dev)) res.set_syscall_errno(-errno);

  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_OPEN(ServerContext *context, const OPENargs *request, OPENres *response)
{
  nfs::OPENres res;
  auto fp = fullpath(request->pathname());
  int oflag = request->oflag();
  int retval = open(fp.c_str(), oflag);

  if (!~retval) res.set_syscall_errno(-errno);

  res.set_syscall_value(retval);
  *response = res;
  return Status::OK;

}

Status NFSImpl::NFSPROC_RELEASE(ServerContext *context, const RELEASEargs *request, RELEASEres *response)
{
  nfs::RELEASEres res;
  auto fp = fullpath(request->pathname());
  int fh = request->fh();
  
  if (!~close(fh)) res.set_syscall_errno(-errno);

  *response = res;
  return Status::OK;
}
