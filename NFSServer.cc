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
#include <signal.h>

#include "NFSServer.h"
#include "helpers.h"
#include <algorithm>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using std::chrono::system_clock;

namespace fs = std::filesystem;

fs::path NFSImpl::fullpath(const std::string &fuse_path)
{
  auto path = fs::path(m_server_storage_path + fuse_path).make_preferred();
  return fs::weakly_canonical(path);
}

NFSImpl::NFSImpl(const std::string &path): 
m_server_storage_path(path),
m_rpc_logger(RPCLogger("/tmp/rpclog.db"))
{}

std::string NFSImpl::serialize(const Message &msg)
{
  return msg.SerializeAsString();
}

template <typename T> T NFSImpl::deserialize(const string &str)
{
  T obj;
  obj.ParseFromString(str);
  return obj;
}

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

  nfs::Stat *stat = new nfs::Stat;
  copystat2Stat(*statbuf, stat);
  res.set_allocated_stat(stat);

  delete statbuf;

  *response = res;
  return Status::OK;
}

int NFSImpl::do_MKNOD(const nfs::MKNODargs *request)
{
  auto fp = fullpath(request->pathname());
  mode_t mode = request->mode();
  dev_t dev = request->dev();
  return mknod(fp.c_str(), mode, dev);
}

Status NFSImpl::NFSPROC_MKNOD(ServerContext *context, const nfs::MKNODargs *request, nfs::MKNODres *response)
{
  nfs::MKNODres res;

  // recovery
  string old_res;
  if (m_rpc_logger.get_log(request->rpc_id(), &old_res))
  {
    res = deserialize<nfs::MKNODres>(old_res);
    *response = res;
    return Status::OK;
  }

  if (do_MKNOD(request) == -1) res.set_syscall_errno(-errno);
  m_rpc_logger.set_log(request->rpc_id(), serialize(res));

  *response = res;
  return Status::OK;
}

int NFSImpl::do_MKDIR(const nfs::MKDIRargs *request)
{
  auto fp = fullpath(request->pathname());
  mode_t mode = request->mode();
  return mkdir(fp.c_str(), mode);
}

Status NFSImpl::NFSPROC_MKDIR(ServerContext *context, const nfs::MKDIRargs *request, nfs::MKDIRres *response)
{
  nfs::MKDIRres res;

  // recovery
  string old_res;
  if (m_rpc_logger.get_log(request->rpc_id(), &old_res))
  {
    res = deserialize<nfs::MKDIRres>(old_res);
    *response = res;
    return Status::OK;
  }

  if (do_MKDIR(request) == -1) res.set_syscall_errno(-errno);

  *response = res;
  return Status::OK;
}

int NFSImpl::do_RMDIR(const nfs::RMDIRargs *request)
{
  auto fp = fullpath(request->pathname());
  return rmdir(fp.c_str());
}

Status NFSImpl::NFSPROC_RMDIR(ServerContext *context, const nfs::RMDIRargs *request, nfs::RMDIRres *response)
{
  nfs::RMDIRres res;

  // recovery
  string old_res;
  if (m_rpc_logger.get_log(request->rpc_id(), &old_res))
  {
    res = deserialize<nfs::RMDIRres>(old_res);
    *response = res;
    return Status::OK;
  }


  if (do_RMDIR(request) == -1) res.set_syscall_errno(-errno);

  *response = res;
  return Status::OK;
}

int NFSImpl::do_RENAME(const nfs::RENAMEargs *request)
{
  auto fp = fullpath(request->oldpathname());
  auto np = fullpath(request->newpathname());
  return rename(fp.c_str(), np.c_str());
}

Status NFSImpl::NFSPROC_RENAME(ServerContext *context, const nfs::RENAMEargs *request, nfs::RENAMEres *response)
{
  nfs::RENAMEres res;

  // recovery
  string old_res;
  if (m_rpc_logger.get_log(request->rpc_id(), &old_res))
  {
    res = deserialize<nfs::RENAMEres>(old_res);
    *response = res;
    return Status::OK;
  }

  if (do_RENAME(request) == -1) res.set_syscall_errno(-errno);

  *response = res;
  return Status::OK;
}

int NFSImpl::do_OPEN(const nfs::OPENargs *request)
{
  auto fp = fullpath(request->pathname());
  int oflag = request->oflag();
  return open(fp.c_str(), oflag);
}

Status NFSImpl::NFSPROC_OPEN(ServerContext *context, const nfs::OPENargs *request, nfs::OPENres *response)
{
  nfs::OPENres res;

  // recovery
  string old_res;
  if (m_rpc_logger.get_log(request->rpc_id(), &old_res))
  {
    res = deserialize<nfs::OPENres>(old_res);
    *response = res;
    return Status::OK;
  }

  int retval = do_OPEN(request);
  if (retval == -1) res.set_syscall_errno(-errno);
  m_rpc_logger.set_log(request->rpc_id(), "1");

  res.set_syscall_value(retval);
  *response = res;
  return Status::OK;

}

int NFSImpl::do_RELEASE(const nfs::RELEASEargs *request)
{
  int fh = request->fh(); 
  return close(fh);
}

Status NFSImpl::NFSPROC_RELEASE(ServerContext *context, const nfs::RELEASEargs *request, nfs::RELEASEres *response)
{
  nfs::RELEASEres res;
  
  // recovery  
  string old_res;
  if (m_rpc_logger.get_log(request->rpc_id(), &old_res))
  {
    res = deserialize<nfs::RELEASEres>(old_res);
    *response = res;
    return Status::OK;
  }

  if (do_RELEASE(request) == -1) res.set_syscall_errno(-errno);
  m_rpc_logger.set_log(request->rpc_id(), "1");

  *response = res;
  return Status::OK;
}

Status NFSImpl::NFSPROC_READ(ServerContext *context, const nfs::READargs *request, nfs::READres *response)
{
  nfs::READres res;
  const int fh = request->fh();
  const size_t size = request->size();
  const off_t offset = request->offset();

  if (lseek(fh, offset, SEEK_SET) == -1)
  {
    res.set_syscall_errno(-errno);
    *response = res;
    return Status::OK;
  }
  char buf[size+1];
  bzero(buf, 0);
  ssize_t retval = read(fh, buf, size);
  if (retval == -1) {
    res.set_syscall_errno(-errno);
    *response = res;
    return Status::OK;
  }
  res.set_syscall_value(retval);
  res.set_data(buf);
  *response = res;
  return Status::OK;
}

long NFSImpl::do_WRITE(const nfs::WRITEargs *request)
{
  const int fh = request->fh();
  const size_t size = request->size();
  const off_t offset = request->offset();
  const char *data = request->data().c_str();

  long retval;
  retval = lseek(fh, offset, SEEK_SET);
  if (retval == -1) return -1;

  retval = write(fh, data, size);
  if (retval == -1) return -1;

  return retval;
}

Status NFSImpl::NFSPROC_WRITE(ServerContext *context, const nfs::WRITEargs *request, nfs::WRITEres *response)
{
  nfs::WRITEres res;

  long retval = do_WRITE(request);
  m_rpc_logger.set_log(request->rpc_id(), "1");

  if (retval == -1) {
    res.set_syscall_errno(-errno);
    *response = res;
    return Status::OK;
  }
  res.set_syscall_value(retval);
  *response = res;
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

  nfs::Stat *stat = new nfs::Stat;
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

Status NFSImpl::RECOVERY(ServerContext *context, ServerReaderWriter<nfs::RECOVERYres, nfs::RECOVERYargs> *stream)
{ 
  return Status::OK;
}
