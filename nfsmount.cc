#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <fuse.h>

#include <iostream>

#include "NFSClient.h"
#include "FileHandlerTable.h"
#include "UserData.h"

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
  struct fuse_context *context = fuse_get_context();

  int err = get_user_data()->client()->NFSPROC_GETATTR(pathname, statbuf);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_mknod(const char *pathname, mode_t mode, dev_t dev)
{
  int err = get_user_data()->client()->NFSPROC_MKNOD(pathname, mode, dev);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  return err;
}

static int nfs_open(const char *pathname, struct fuse_file_info *fi)
{
  int fh = get_user_data()->fhtable()->allocate();
  if (!~fh) return -ENFILE;
  fi->fh = fh;
  int ret;
  int err = get_user_data()->client()->NFSPROC_OPEN(pathname, fi, &ret);
  if (!NFSPROC_OK(err)) get_user_data()->fhtable()->deallocate(fh);
  if (NFSPROC_RPC_ERROR(err)) return -EINVAL;
  if (NFSPROC_SYSCALL_ERROR(err) < 0) return err;
  return ret;
}

static int nfs_release(const char *pathname, struct fuse_file_info *fi)
{
  int err = get_user_data()->client()->NFSPROC_RELEASE(nullptr, fi);
  if (NFSPROC_OK(err)) get_user_data()->fhtable()->deallocate(fi->fh);
  if (NFSPROC_RPC_ERROR(err > 0)) return -EINVAL;
  return err;
}

static int nfs_read(const char *pathname, char* buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  return 0;
}

static int nfs_write(const char *pathname, const char* buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
  return 0;
}

static struct fuse_operations nfs_oper = {
  .getattr = nfs_getattr,
  .mknod = nfs_mknod,
  .open = nfs_open,
  .read = nfs_read,
  .write = nfs_write,
  .release = nfs_release,
  .init = nfs_init,
  .destroy = nfs_destroy
};

int main(int argc, char **argv)
{
  fuse_main(argc, argv, &nfs_oper, NULL);
}
