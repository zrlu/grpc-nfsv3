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
#define NFS_DEBUG(path) fprintf(stderr, "%s(%s)\n", __func__, path)
#else
#define NFS_DEBUG(path)
#endif

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
  int err = get_user_data()->client()->NFSPROC_GETATTR(path, statbuf);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_mknod(const char *path, mode_t mode, dev_t dev)
{
  NFS_DEBUG(path);
  int err = get_user_data()->client()->NFSPROC_MKNOD(path, mode, dev);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_mkdir(const char *path, mode_t mode)
{
  // need to implement
  return 0;
}

static int nfs_unlink(const char *path)
{
  // need to implement
  return 0;
}

static int nfs_rmdir(const char *path)
{
  // need to implement
  return 0;
}

static int nfs_rename(const char *oldpath, const char *newpath)
{
  // need to implement
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
  int fh_server, err = get_user_data()->client()->NFSPROC_OPEN(path, fi, &fh_server);
  fi->fh = fh_server;
  if (!NFSPROC_OK(err)) get_user_data()->fhtable()->deallocate(fh);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return 0;
}

static int nfs_release(const char *path, struct fuse_file_info *fi)
{
  NFS_DEBUG(path);
  int err = get_user_data()->client()->NFSPROC_RELEASE(nullptr, fi);
  if (NFSPROC_OK(err)) get_user_data()->fhtable()->deallocate(fi->fh);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
  // If the datasync parameter is non-zero, then only the user data should be flushed, not the meta data. 
  return 0;
}

static int nfs_read(const char *path, char* buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  NFS_DEBUG(path);
  ssize_t bytes_read;
  int err = get_user_data()->client()->NFSPROC_READ(nullptr, buffer, size, offset, fi, &bytes_read);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return bytes_read;
}

static int nfs_write(const char *path, const char* buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  NFS_DEBUG(path);
  ssize_t bytes_wrote;
  int err = get_user_data()->client()->NFSPROC_WRITE(nullptr, buffer, size, offset, fi, &bytes_wrote);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return bytes_wrote;
}

static int nfs_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
  // does not need to implement but implemented anyways
  NFS_DEBUG(path);
  int err = get_user_data()->client()->NFSPROC_FGETATTR(nullptr, statbuf, fi);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_utimens(const char *path, const struct timespec tv[2])
{
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
  int err = get_user_data()->client()->NFSPROC_READDIR(path, buf, filler, offset, fi);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
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
