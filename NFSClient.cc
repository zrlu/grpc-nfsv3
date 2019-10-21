#include <fuse.h>

#include "NFSClient.h"
#include "helpers.h"
#include <algorithm>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using grpc::StatusCode;

// #undef CLIENT_ENABLE_DEBUG_MESSAGE
#ifdef CLIENT_ENABLE_DEBUG_MESSAGE
#define DEBUG_RESPONSE(_res) std::cerr << "CLIENT <==  " << __func__ << ": " << _res.ShortDebugString().substr(0, 300) << std::endl;
#define DEBUG_REQUEST(_args) std::cerr << "CLIENT  ==> " << __func__ << ": " << _args->ShortDebugString().substr(0, 300) << std::endl;
#define DEBUG_REQUEST_(_args) std::cerr << "CLIENT  ==> " << __func__ << ": " << _args.ShortDebugString().substr(0, 300) << std::endl;

#else
#define DEBUG_RESPONSE(res)
#define DEBUG_REQUEST(res)
#define DEBUG_REQUEST_(res)
#endif

extern rpcid_t current_rpcid;
extern bool recovery_mode;

NFSClient::NFSClient(std::shared_ptr<Channel> channel, int client_id) : 
  m_channel(channel), 
  m_client_id(client_id),
  stub_(NFS::NewStub(channel))
  {}

bool NFSClient::WaitForConnection()
{
  return NFSClient::WaitForConnection(LONG_MAX, INT_MAX);
}

bool NFSClient::WaitForConnection(int64_t sec, int32_t nsec)
{
  gpr_timespec timeout{sec, nsec, GPR_TIMESPAN};
  return m_channel->WaitForConnected<gpr_timespec>(timeout);
}

template <typename T> T* NFSClient::make_rpc()
{
  T *args = new T;
  rpcid_t id = recovery_mode ? current_rpcid : m_rpc_mgr.generate_rpc_id(m_client_id);
  current_rpcid = id;
  args->set_rpc_id(id);
  m_rpc_mgr.set_rpc(id, args);
  return args;
}

bool NFSClient::del_rpc_if_ok(rpcid_t rpcid, const Status &status)
{
    if (status.ok())
    {
      m_rpc_mgr.delete_rpc(rpcid);
      return true;
    }
    return false;
}

bool NFSClient::remap_fh()
{
  // std::cerr << "====== remap_fh ======" << std::endl;
  for (auto it = m_opened.begin(); it != m_opened.end(); ++it)
  {
    std::string pathname = it->first;
    struct fuse_file_info &fi = it->second;
    // std::cerr << pathname << ": " << fi.fh << " => ";
    int new_fh;
    int err = NFSPROC_OPEN(pathname.c_str(), &fi, &new_fh);
    // std::cerr << new_fh << " | err: " << err << std::endl;
    if (err) {
      return false;
    }
    fi.fh = new_fh;
  }
  // std::cerr << "====== remap_fh ======" << std::endl;
  m_opened.clear();
  return true;
}

int NFSClient::NFSPROC_NULL(void)
{
  ClientContext context;
  nfs::NULLargs args;
  nfs::NULLres res;
  Status status = stub_->NFSPROC_NULL(&context, args, &res);
  DEBUG_RESPONSE(res);
  return status.error_code();
}

int NFSClient::NFSPROC_GETATTR(const char *pathname, struct stat *statbuf)
{
  ClientContext context;

  nfs::GETATTRargs args;
  nfs::GETATTRres res;
  args.set_pathname(pathname);

  DEBUG_REQUEST_(args);
  Status status = stub_->NFSPROC_GETATTR(&context, args, &res);
  DEBUG_RESPONSE(res);
  
  const nfs::Stat stat = res.stat();
  copyStat2stat(stat, statbuf);

  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_MKNOD(const char *pathname, mode_t mode, dev_t dev)
{
  ClientContext context;

  nfs::MKNODargs* args = make_rpc<nfs::MKNODargs>();
  nfs::MKNODres res;

  args->set_pathname(pathname);
  args->set_mode(mode);
  args->set_dev(dev);
  
  DEBUG_REQUEST(args);
  Status status = stub_->NFSPROC_MKNOD(&context, *args, &res);
  del_rpc_if_ok(args->rpc_id(), status);
  DEBUG_RESPONSE(res);

  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_MKDIR(const char *pathname, mode_t mode)
{
  ClientContext context;

  nfs::MKDIRargs* args = make_rpc<nfs::MKDIRargs>();
  nfs::MKDIRres res;
  args->set_pathname(pathname);
  args->set_mode(mode);

  DEBUG_REQUEST(args);
  Status status = stub_->NFSPROC_MKDIR(&context, *args, &res);
  del_rpc_if_ok(args->rpc_id(), status);
  DEBUG_RESPONSE(res);

  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_UNLINK(const char *pathname)
{
  ClientContext context;

  nfs::UNLINKargs* args = make_rpc<nfs::UNLINKargs>();
  nfs::UNLINKres res;
  args->set_pathname(pathname);

  DEBUG_REQUEST(args);
  Status status = stub_->NFSPROC_UNLINK(&context, *args, &res);
  del_rpc_if_ok(args->rpc_id(), status);
  DEBUG_RESPONSE(res);

  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_RMDIR(const char *pathname)
{
  ClientContext context;

  nfs::RMDIRargs* args = make_rpc<nfs::RMDIRargs>();
  nfs::RMDIRres res;
  args->set_pathname(pathname);

  DEBUG_REQUEST(args);
  Status status = stub_->NFSPROC_RMDIR(&context, *args, &res);
  del_rpc_if_ok(args->rpc_id(), status);
  DEBUG_RESPONSE(res);

  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_RENAME(const char *oldpathname, const char * newpathname)
{
  ClientContext context;

  nfs::RENAMEargs* args = make_rpc<nfs::RENAMEargs>();
  nfs::RENAMEres res;
  args->set_oldpathname(oldpathname);
  args->set_newpathname(newpathname);

  DEBUG_REQUEST(args);
  Status status = stub_->NFSPROC_RENAME(&context, *args, &res);
  del_rpc_if_ok(args->rpc_id(), status);
  DEBUG_RESPONSE(res);

  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_TRUNCATE(const char *path, off_t length)
{
  ClientContext context;

  nfs::TRUNCATEargs* args = make_rpc<nfs::TRUNCATEargs>();
  nfs::TRUNCATEres res;
  args->set_pathname(path);
  args->set_length(length);

  DEBUG_REQUEST(args);
  Status status = stub_->NFSPROC_TRUNCATE(&context, *args, &res);
  del_rpc_if_ok(args->rpc_id(), status);
  DEBUG_RESPONSE(res);

  return status.error_code() | res.syscall_errno();
}

int NFSClient::NFSPROC_OPEN(const char *pathname, struct fuse_file_info *fi, int *ret)
{
  ClientContext context;
  nfs::OPENargs* args = make_rpc<nfs::OPENargs>();
  nfs::OPENres res;

  args->set_pathname(pathname);
  args->set_oflag(fi->flags);

  DEBUG_REQUEST(args);
  Status status = stub_->NFSPROC_OPEN(&context, *args, &res);
  del_rpc_if_ok(args->rpc_id(), status);
  DEBUG_RESPONSE(res);

  if (res.syscall_errno() == 0 && fi->flags & O_WRONLY)
  {
    m_mu_to_commit.lock();
    if (m_to_commit.find(std::string(pathname)) == m_to_commit.end())
    {
      m_to_commit[std::string(pathname)] = std::set<rpcid_t>();
    }
    m_mu_to_commit.unlock();
  }
  int err = status.error_code() | res.syscall_errno();
  if (!err)
  {
    m_mu_opened.lock();
    m_opened[std::string(pathname)] = *fi;
    m_mu_opened.unlock();
  }

  *ret = res.syscall_value();
  return err;
}

int NFSClient::NFSPROC_RELEASE(const char *pathname, struct fuse_file_info *fi)
{
  // Send COMMIT here
  if (fi->flags & O_WRONLY)
  {
    // std::cerr << "====== COMMIT ====== " << "fd: " << fi->fh << std::endl;
    ClientContext context;
    nfs::COMMITargs* args = make_rpc<nfs::COMMITargs>();
    nfs::COMMITres res;
    args->set_fh(fi->fh);
    m_mu_to_commit.lock();
    std::set<rpcid_t> &set_pending = m_to_commit[std::string(pathname)];

    for (auto it = set_pending.begin(); it != set_pending.end(); ++it)
    {
      string *to_commit = args->add_to_commit_id();
      to_commit->assign(*it);
    }

    DEBUG_REQUEST(args);
    std::set<rpcid_t> set_missing;
    std::set<rpcid_t> set_ok;
    // Check for missing
    if (CHECK_MISSING(*args, &set_missing) > 0) WRITE_BUFFER_SYNC(&set_missing);
    // actually commit
    std::shared_ptr<ClientReaderWriter<nfs::COMMITargs, nfs::COMMITres> > stream(stub_->NFSPROC_COMMIT(&context));

    stream->Write(*args);
    stream->WritesDone();
    while (stream->Read(&res)) {}
    DEBUG_RESPONSE(res);

    Status status = stream->Finish();
    del_rpc_if_ok(args->rpc_id(), status);
    for (int i = 0; i < res.commit_id_size(); ++i) set_ok.insert(res.commit_id(i));
    for (auto it = set_ok.begin(); it != set_ok.end(); ++it)
    {
      set_pending.erase(*it);
      m_rpc_mgr.delete_rpc(*it);
    }
    m_mu_to_commit.unlock();
    // std::cerr << "==================" <<std::endl;
  }

  ClientContext context;
  nfs::RELEASEargs* args = make_rpc<nfs::RELEASEargs>();
  nfs::RELEASEres res;

  args->set_fh(fi->fh);

  DEBUG_REQUEST(args);
  Status status = stub_->NFSPROC_RELEASE(&context, *args, &res);
  del_rpc_if_ok(args->rpc_id(), status);
  DEBUG_RESPONSE(res);


  m_mu_to_commit.lock();
  m_to_commit.erase(std::string(pathname));
  m_mu_to_commit.unlock();
  
  int err = status.error_code() | res.syscall_errno();
  if (!err)
  {
    m_mu_opened.lock();
    m_opened.erase(std::string(pathname));
    m_mu_opened.unlock();
  }

  return err;

}

int NFSClient::NFSPROC_READ(const char *pathname, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi, ssize_t *ret)
{
  // recovery
  {
    if (recovery_mode)
    {
      remap_fh();
    }
  }

  ClientContext context;
  nfs::READargs args;
  nfs::READres res;

  args.set_fh(fi->fh);
  args.set_size(size);
  args.set_offset(offset);

  DEBUG_REQUEST_(args);
  
  Status status = stub_->NFSPROC_READ(&context, args, &res);

  DEBUG_RESPONSE(res);

  int err = status.error_code() | res.syscall_errno();
  if (!err) {
    const int bytes_read = res.syscall_value();
    res.data().copy(buffer, bytes_read);
    *ret = bytes_read;
    return 0;
  }
  *ret = -1;

  return err;
}

int NFSClient::NFSPROC_WRITE(const char *pathname, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi, ssize_t *ret)
{
  // recovery
  {
    if (recovery_mode)
    {
      remap_fh();
    }
  }

  ClientContext context;
  nfs::WRITEargs *args = make_rpc<nfs::WRITEargs>();
  nfs::WRITEres res;
  
  args->set_fh(fi->fh);
  args->set_size(size);
  args->set_offset(offset);
  args->set_data(buffer, size);

  DEBUG_REQUEST(args);
  Status status = stub_->NFSPROC_WRITE(&context, *args, &res);
  DEBUG_RESPONSE(res);

  if (status.ok())
  {
    m_mu_to_commit.lock();
    m_to_commit[std::string(pathname)].insert(args->rpc_id());
    m_mu_to_commit.unlock();
    *ret = size;
  }

  return status.error_code(); 
}

int NFSClient::NFSPROC_FGETATTR(const char *pathname, struct stat *statbuf, const struct fuse_file_info *fi)
{
  ClientContext context;
  nfs::FGETATTRargs args;
  nfs::FGETATTRres res;

  args.set_fh(fi->fh);
  
  DEBUG_REQUEST_(args);
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
  
  DEBUG_REQUEST_(args);
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

int NFSClient::CHECK_MISSING(const nfs::COMMITargs &list, std::set<rpcid_t> *missing)
{
  // std::cerr << "CHECK_MISSING" << std::endl;
  ClientContext context;
  nfs::ResendList res;  

  Status status = stub_->CHECK_MISSING(&context, list, &res);
  missing->clear();
  // std::cerr << res.ShortDebugString() << std::endl;
  int size = res.rpc_id_size();
  // std::cerr << size << std::endl;
  for (int i = 0; i < size; ++i)
  {
    missing->insert(res.rpc_id(i));
  }

  return missing->size();
}

int NFSClient::WRITE_BUFFER_SYNC(const std::set<rpcid_t> *missing)
{
    ClientContext context;
    nfs::SyncResponse res;
    std::shared_ptr<ClientWriter<nfs::WRITEargs> > stream(stub_->WRITE_BUFFER_SYNC(&context, &res));

    for (auto it = missing->begin(); it != missing->end(); ++it)
    {
      const rpcid_t id = *it;
      Message * ptr = m_rpc_mgr.get_rpc(id);
      nfs::WRITEargs *_args = dynamic_cast<nfs::WRITEargs*>(ptr);
      if (!_args)
      {
        continue;
      }
      nfs::WRITEargs &args = *_args;
      stream->Write(args);
    }
    stream->WritesDone();

    Status status = stream->Finish();

    return status.error_code();
}
