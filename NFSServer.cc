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

using std::chrono::system_clock;

namespace fs = std::filesystem;

const char *NFSImpl::getFullPath(const std::string &suffix)
{
    fs::path fp = fs::path(m_serverStoragePath) / fs::path(suffix);
    return fp.c_str();
}

NFSImpl::NFSImpl(const std::string &path) : m_serverStoragePath(path) {}

Status NFSImpl::NFSPROC_NULL(ServerContext *context, const NULLargs *request, NULLres *response)
{
  std::cerr << "NULL" << std::endl;
  nfs::NULLres res;
  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_GETATTR(ServerContext *context, const GETATTRargs *request, GETATTRres *response)
{
  std::cerr << "GETATTR" << std::endl;
  nfs::GETATTRres res;
  
  const char *pathname = getFullPath(request->pathname().c_str());

  struct stat statbuf;

  if (!~stat(pathname, &statbuf))
  {
    res.set_syscall_errno(-errno);
  }
  Stat *stat = new Stat;
  copystat2Stat(&statbuf, stat);
  *response = res;
  res.set_allocated_stat(stat);
  return Status::OK;
}

Status NFSImpl::NFSPROC_MKNOD(ServerContext *context, const MKNODargs *request, MKNODres *response)
{
  std::cerr << "MKNOD" << std::endl;
  nfs::MKNODres res;

  const char *pathname = getFullPath(request->pathname().c_str());
  mode_t mode = request->mode();
  dev_t dev = request->dev();

  if (!~mknod(pathname, mode, dev))
  {
    res.set_syscall_errno(-errno);
  }

  *response = res;
  return Status::OK;
}