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
#include <dirent.h>
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

using std::chrono::system_clock;

#define READ_CHUNK_SIZE (1<<20)

namespace fs = std::filesystem;

fs::path NFSImpl::fullpath(const std::string &fuse_path)
{
  auto path = fs::path(m_serverStoragePath + fuse_path).make_preferred();
  return fs::weakly_canonical(path);
}

NFSImpl::NFSImpl(const std::string &path) : m_serverStoragePath(path) {}

Status NFSImpl::NFSPROC_NULL(ServerContext *context, const nfs::NULLargs *request, nfs::NULLres *response)
{
  nfs::NULLres res;
  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_GETATTR(ServerContext *context, const nfs::GETATTRargs *request, nfs::GETATTRres *response)
{
  nfs::GETATTRres res;
  auto fp = fullpath(request->pathname());
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

Status NFSImpl::NFSPROC_MKNOD(ServerContext *context, const nfs::MKNODargs *request, nfs::MKNODres *response)
{
  nfs::MKNODres res;
  auto fp = fullpath(request->pathname());
  mode_t mode = request->mode();
  dev_t dev = request->dev();

  if (mknod(fp.c_str(), mode, dev) == -1) res.set_syscall_errno(-errno);

  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_OPEN(ServerContext *context, const nfs::OPENargs *request, nfs::OPENres *response)
{
  nfs::OPENres res;
  auto fp = fullpath(request->pathname());
  int oflag = request->oflag();
  int retval = open(fp.c_str(), oflag);

  if (retval == -1) res.set_syscall_errno(-errno);
  
  res.set_syscall_value(retval);
  *response = res;
  return Status::OK;

}

Status NFSImpl::NFSPROC_RELEASE(ServerContext *context, const nfs::RELEASEargs *request, nfs::RELEASEres *response)
{
  nfs::RELEASEres res;
  int fh = request->fh();
  
  if (close(fh) == -1) res.set_syscall_errno(-errno);

  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_READ(ServerContext *context, const nfs::READargs *request, ServerWriter<nfs::READres> *writer)
{
  nfs::READres res;
  int fh = request->fh();
  size_t size = request->size();
  off_t offset = request->offset();
  int retval;
  char *buffer = new char[READ_CHUNK_SIZE];
  int num_chunk = ((int)size / (int)READ_CHUNK_SIZE) + 1;
  for (int chunk_idx = 0; chunk_idx < num_chunk; ++chunk_idx)
  {
    if (lseek(fh, offset + chunk_idx*READ_CHUNK_SIZE, SEEK_SET) == -1)
    {
      res.set_syscall_errno(-errno);
      break;
    }
    if (retval = read(fh, buffer, READ_CHUNK_SIZE) == -1) {
      res.set_syscall_errno(-errno);
      break;
    }
    res.set_syscall_value(retval);
    res.set_data(buffer);
    res.set_chunk_idx(chunk_idx);
    writer->Write(res);
  }
  delete buffer;
  return Status::OK;
}

Status NFSImpl::NFSPROC_FGETATTR(ServerContext *context, const nfs::FGETATTRargs *request, nfs::FGETATTRres *response)
{
  nfs::FGETATTRres res;
  struct stat *statbuf = new struct stat;
  int fh = request->fh();

  if (fstat(fh, statbuf) == -1)
  {
    res.set_syscall_errno(-errno);
    *response = res;
    return Status::OK;
  }

  nfs::Stat *stat = new Stat;
  copystat2Stat(*statbuf, stat);
  res.set_allocated_stat(stat);

  delete statbuf;

  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_READDIR(ServerContext *context, const nfs::READDIRargs *request, nfs::READDIRres *response)
{
  nfs::READDIRres res;

  DIR *dp;
  struct dirent *de;

  auto fp = fullpath(request->pathname());

  dp = opendir(fp.c_str());
  if (dp == NULL)
  {
    res.set_syscall_errno(-errno);
    return Status::OK;
  }

  while ((de = readdir(dp)) != NULL) {
      res.add_stat();
      nfs::Stat *st = res.add_stat();
      st->set_st_ino(de->d_ino);
      st->set_st_mode(DTTOIF(de->d_type));
      
      std::string *fname = res.add_filename();
      fname->assign(de->d_name);
  }

  closedir(dp);
  *response = res;
  return Status::OK;
}