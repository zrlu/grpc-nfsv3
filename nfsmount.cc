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
#define NFS_DEBUG(pathname) fprintf(stderr, "%s(%s)\n", __func__, pathname)
#else
#define NFS_DEBUG(pathname)
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

static int nfs_getattr(const char *pathname, struct stat *statbuf)
{
  NFS_DEBUG(pathname);
  int err = get_user_data()->client()->NFSPROC_GETATTR(pathname, statbuf);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_mknod(const char *pathname, mode_t mode, dev_t dev)
{
  NFS_DEBUG(pathname);
  int err = get_user_data()->client()->NFSPROC_MKNOD(pathname, mode, dev);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_open(const char *pathname, struct fuse_file_info *fi)
{
  NFS_DEBUG(pathname);
  int fh = get_user_data()->fhtable()->allocate();
  if (fh == -1) return -ENFILE;
  fi->fh = fh;
  int fh_server, err = get_user_data()->client()->NFSPROC_OPEN(pathname, fi, &fh_server);
  fi->fh = fh_server;
  if (!NFSPROC_OK(err)) get_user_data()->fhtable()->deallocate(fh);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return 0;
}

static int nfs_release(const char *pathname, struct fuse_file_info *fi)
{
  NFS_DEBUG(pathname);
  int err = get_user_data()->client()->NFSPROC_RELEASE(nullptr, fi);
  if (NFSPROC_OK(err)) get_user_data()->fhtable()->deallocate(fi->fh);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_read(const char *pathname, char* buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  NFS_DEBUG(pathname);
  ssize_t bytes_read;
  int err = get_user_data()->client()->NFSPROC_READ(nullptr, buffer, size, offset, fi, &bytes_read);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return bytes_read;
}

static int nfs_write(const char *pathname, const char* buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  NFS_DEBUG(pathname);
  return 0;
}

static int nfs_fgetattr(const char *pathname, struct stat *statbuf, struct fuse_file_info *fi)
{
  NFS_DEBUG(pathname);
  int err = get_user_data()->client()->NFSPROC_FGETATTR(nullptr, statbuf, fi);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_opendir(const char *pathname, struct fuse_file_info *fi)
{
  NFS_DEBUG(pathname);
  return 0;
}

static int nfs_readdir(const char *pathname, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  NFS_DEBUG(pathname);
  int err = get_user_data()->client()->NFSPROC_READDIR(pathname, buf, filler, offset, fi);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  if (NFSPROC_SYSCALL_ERROR(err)) return err;
  return 0;
}

static int nfs_releasedir(const char *pathname, struct fuse_file_info *fi)
{
  NFS_DEBUG(pathname);
  return 0;
}

static struct fuse_operations nfs_oper = {
  .getattr = nfs_getattr,
  .mknod = nfs_mknod,
  .open = nfs_open,
  .read = nfs_read,
  .write = nfs_write,
  .release = nfs_release,
  .opendir = nfs_opendir,
  .readdir = nfs_readdir,
  .releasedir = nfs_releasedir,
  .init = nfs_init,
  .destroy = nfs_destroy,
  .fgetattr = nfs_fgetattr
};

int main(int argc, char **argv)
{
  fuse_main(argc, argv, &nfs_oper, NULL);
}
