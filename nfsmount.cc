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

#include "NFSClient.h"
#include "FileHandlerTable.h"
#include "UserData.h"

#ifdef ENABLE_NFS_DEBUG
#define NFS_DEBUG(path) fprintf(stderr, "FUSE: %s(%s)\n", __func__, path)
#else
#define NFS_DEBUG(path)
#endif


#define RECONNECT_IF_RPC_FAIL(__rpc, __err_addr, ...) \
do {\
  *__err_addr = get_user_data()->client()->__rpc(__VA_ARGS__);\
  if (NFSPROC_RPC_ERROR(*__err_addr))\
  {\
      get_user_data()->client()->WaitForConnection();\
      int recovery_code = 1;\
      while (recovery_code != 0)\
      {\
        puts("FUSE: entering recovery...");\
        recovery_code = get_user_data()->client()->RECOVERY();\
        get_user_data()->client()->WaitForConnection();\
      }\
  }\
} while (NFSPROC_RPC_ERROR(*__err_addr));\


UserData *get_user_data()
{
  struct fuse_context *context = fuse_get_context();
  return (UserData*)(context->private_data);
}

static void *nfs_init(struct fuse_conn_info *conn)
{
  auto channel = grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials());
  
  gpr_timespec timeout{10, 0, GPR_TIMESPAN};
  bool connected = channel->WaitForConnected<gpr_timespec>(timeout);
  if (connected) {
    std::cerr << "connected" << std::endl;
  } else {
    std::cerr << "timeout" << std::endl;
    exit(-1);
  }
  UserData *ud = new UserData(new NFSClient(channel));
  return (void *)ud;
}

static void nfs_destroy(void *userdata)
{
  delete (UserData *)userdata;
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
  return 0;
}

static int nfs_unlink(const char *path)
{
  // need to implement
  return 0;
}

static int nfs_rmdir(const char *path)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_RMDIR, &err, path);
  return 0;
}

static int nfs_rename(const char *oldpath, const char *newpath)
{
  NFS_DEBUG(path);
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_RENAME, &err, oldpath, newpath);
  return 0;
}

static int nfs_truncate(const char *path, off_t offset)
{
  // need to implement
  return 0;
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
  RECONNECT_IF_RPC_FAIL(NFSPROC_RELEASE, &err, nullptr, fi);
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
  RECONNECT_IF_RPC_FAIL(NFSPROC_READ, &err, nullptr, buffer, size, offset, fi, &bytes_read);
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return bytes_read;
}

static int nfs_write(const char *path, const char* buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  NFS_DEBUG(path);
  ssize_t bytes_wrote;
  int err;
  RECONNECT_IF_RPC_FAIL(NFSPROC_WRITE, &err, nullptr, buffer, size, offset, fi, &bytes_wrote);
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
  fuse_main(argc, argv, &nfs_oper, NULL);
}
