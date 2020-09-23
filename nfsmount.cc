#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <fuse.h>
#include <dirent.h>

#include <iostream>
#include <string>
#include <set>
#include <mutex>

#include "NFSClient.h"
#include "FileHandlerTable.h"
#include "UserData.h"

// #undef ENABLE_NFS_DEBUG
#ifdef ENABLE_NFS_DEBUG
#define NFS_DEBUG(path) fprintf(stderr, "FUSE ===> : %s(%s)\n", __func__, path)
#else
#define NFS_DEBUG(path)
#endif

#define NFS_OPT(t, p, v) { t, offsetof(struct nfs_config, p), v }

#define DEFAULT_SERVER "127.0.0.1:50055"
#define DEFAULT_CLIENT_ID 0

enum {
  SERVER,
  CLIENT_ID
};

struct nfs_config
{
  char *server;
  int client_id;
};

rpcid_t current_rpcid;
bool recovery_mode = false;
std::mutex mu_;

struct nfs_config conf;

static struct fuse_opt nfs_opts[] = {
     NFS_OPT("--server=%s",         server   , 0 ),
     NFS_OPT("--client_id=%i",      client_id, 0 ),
     FUSE_OPT_END
};

#define RECONNECT_IF_RPC_FAIL(__rpc, __err_addr, ...) \
do {\
  *__err_addr = get_user_data()->client()->__rpc(__VA_ARGS__);\
  if (NFSPROC_RPC_ERROR(*__err_addr))\
  {\
      std::cerr << "Something's wrong, trying to reconnect..." << std::endl;\
      get_user_data()->client()->WaitForConnection();\
      /*\
      std::cerr << "FUSE: [[ Entering recovery mode for RPC ID: " << current_rpcid << " ]]" << std::endl; */ \
      mu_.lock();\
      recovery_mode = true; /* send the same rpcid */ \
      *__err_addr = get_user_data()->client()->__rpc(__VA_ARGS__);\
      recovery_mode = false;\
      mu_.unlock();\
      puts("FUSE: recovery success!");\
      break;\
  }\
} while (NFSPROC_RPC_ERROR(*__err_addr));\


UserData *get_user_data()
{
  struct fuse_context *context = fuse_get_context();
  return (UserData*)(context->private_data);
}

static void *nfs_init(struct fuse_conn_info *conn)
{
  auto channel = grpc::CreateChannel(conf.server, grpc::InsecureChannelCredentials());
  
  gpr_timespec timeout{10, 0, GPR_TIMESPAN};
  bool connected = channel->WaitForConnected<gpr_timespec>(timeout);
  if (connected) {
    std::cerr << "connected" << std::endl;
  } else {
    std::cerr << "timeout" << std::endl;
    exit(-1);
  }
  UserData *ud = new UserData(new NFSClient(channel, conf.client_id));
  return (void *)ud;
}

static void nfs_destroy(void *userdata)
{
  delete (UserData *)userdata;
}

static void nfs_access(const char *path, int)
{

}

static int nfs_getattr(const char *path, struct stat *statbuf)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_GETATTR, &err, path, statbuf);
  return err;
}

static int nfs_mknod(const char *path, mode_t mode, dev_t dev)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_MKNOD, &err, path, mode, dev);
  return err;
}

static int nfs_mkdir(const char *path, mode_t mode)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_MKDIR, &err, path, mode);
  return err;
}

static int nfs_unlink(const char *path)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_UNLINK, &err, path);  return 0;
  return err;
}

static int nfs_rmdir(const char *path)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_RMDIR, &err, path);
  return err;
}

static int nfs_rename(const char *oldpath, const char *newpath)
{
  NFS_DEBUG(oldpath);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_RENAME, &err, oldpath, newpath);
  return err;
}

static int nfs_truncate(const char *path, off_t offset)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_TRUNCATE, &err, path, offset);
  return err;
}
  
static int nfs_open(const char *path, struct fuse_file_info *fi)
{
  NFS_DEBUG(path);
  int fh = get_user_data()->fhtable()->allocate();
  if (fh == -1) return -ENFILE;
  fi->fh = fh;
  int fh_server, err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_OPEN, &err, path, fi, &fh_server);
  fi->fh = fh_server;
  return err;
}

static int nfs_release(const char *path, struct fuse_file_info *fi)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_RELEASE, &err, path, fi);
  if (!NFSPROC_SYSCALL_ERROR(err)) {
    get_user_data()->fhtable()->deallocate(fi->fh);
    return 0;
  }
  return err;
}

static int nfs_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
  // If the datasync parameter is non-zero, then only the user data should be flushed, not the meta data.
  // I think this is not like the local fsync, it is like the "periodic commit", we will do later
  return 0;
}

static int nfs_read(const char *path, char* buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  NFS_DEBUG(path);
  ssize_t bytes_read;
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_READ, &err, path, buffer, size, offset, fi, &bytes_read);
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return bytes_read;
}

static int nfs_write(const char *path, const char* buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  NFS_DEBUG(path);
  ssize_t bytes_wrote;
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_WRITE, &err, path, buffer, size, offset, fi, &bytes_wrote);
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return bytes_wrote;
}

static int nfs_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
  // does not need to implement but implemented anyways
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_FGETATTR, &err, nullptr, statbuf, fi);
  return err;
}

static int nfs_utimens(const char *path, const struct timespec tv[2])
{
  // low priority
  return 0;
}

static int nfs_opendir(const char *path, struct fuse_file_info *fi)
{
  // does not need to implement
  NFS_DEBUG(path);
  return 0;
}

static int nfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_READDIR, &err, path, buf, filler, offset, fi);
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return 0;
}

static int nfs_releasedir(const char *path, struct fuse_file_info *fi)
{
  // does not need to implement
  NFS_DEBUG(path);
  return 0;
}

static int nfs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs) {
  // std::cerr << arg << ":" << key << std::endl;
  return 0;
}

static struct fuse_operations nfs_oper = {
  .getattr = nfs_getattr,
  .mknod = nfs_mknod,
  .mkdir = nfs_mkdir,
  .unlink = nfs_unlink,
  .rmdir = nfs_rmdir,
  .rename = nfs_rename,
  .truncate = nfs_truncate,
  .open = nfs_open,
  .read = nfs_read,
  .write = nfs_write,
  .release = nfs_release,
  .fsync = nfs_fsync,
  .opendir = nfs_opendir,
  .readdir = nfs_readdir,
  .releasedir = nfs_releasedir,
  .init = nfs_init,
  .destroy = nfs_destroy,
  .fgetattr = nfs_fgetattr,
  .utimens = nfs_utimens
};

int main(int argc, char **argv)
{

  conf.client_id = DEFAULT_CLIENT_ID;
  conf.server = new char[sizeof(DEFAULT_SERVER)];
  strcpy(conf.server, DEFAULT_SERVER);
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  fuse_opt_parse(&args, &conf, nfs_opts, nfs_opt_proc);

  printf("server -> %s\n", conf.server);
  printf("client_id -> %d\n", conf.client_id);

  fuse_opt_add_arg(&args, argv[2]);
  fuse_opt_add_arg(&args, argv[1]); 

  fuse_main(args.argc, args.argv, &nfs_oper, NULL);
}
