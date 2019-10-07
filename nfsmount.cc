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

  int retval = get_user_data()->client()->NFSPROC_GETATTR(pathname, statbuf);
  if (retval > 0) {
    retval = -EINVAL;
  }
  return retval;
}

static int nfs_mknod(const char *pathname, mode_t mode, dev_t dev)
{
  int retval = get_user_data()->client()->NFSPROC_MKNOD(pathname, mode, dev);
  if (retval > 0) {
    retval = -EINVAL;
  }
  return retval;
}

static struct fuse_operations nfs_oper = {
  .getattr = nfs_getattr,
  .mknod = nfs_mknod,
  .init = nfs_init,
  .destroy = nfs_destroy
};

int main(int argc, char **argv)
{
  // gpr_timespec timeout{10, 0, GPR_TIMESPAN};
  // auto channel = grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials());
  // bool connected = channel->WaitForConnected<gpr_timespec>(timeout);
  // if (connected) {
  //   std::cerr << "connected" << std::endl;
  // } else {
  //   std::cerr << "timeout" << std::endl;
  //   exit(-1);
  // }
  // NFSClient *client_ptr = new NFSClient(channel);

  // just some tests...to be deleted
  // struct stat statbuf;
  // int code = client_ptr->NFSPROC_GETATTR("a", &statbuf);
  // to be deleted

  fuse_main(argc, argv, &nfs_oper, NULL);
}
