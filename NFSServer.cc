#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "NFSServer.h"

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

const char *NFSImpl::getFullPath(const std::string &suffix)
{
  return (m_serverStoragePath + suffix).c_str();
}

NFSImpl::NFSImpl(const std::string &path) : m_serverStoragePath(path) {}

Status NFSImpl::NFSPROC_NULL(ServerContext *context, const NULLargs *request, NULLres *response)
{
  std::cerr << "NULL" << std::endl;
  nfs::NULLres res;
  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_MKNOD(ServerContext *context, const MKNODargs *request, MKNODres *response)
{
  std::cerr << "MKNOD" << std::endl;
  nfs::MKNODres res;
  res.set_ret(0);

  const char *pathname = getFullPath(request->pathname().c_str());
  mode_t mode = request->mode();
  dev_t dev = request->dev();

  if (!~mknod(pathname, mode, dev))
  {
    res.set_ret(-errno);
  }

  *response = res;
  return Status::OK;
}