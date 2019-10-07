#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#include <iostream>

#include "NFSClient.h"

NFSClient *client_ptr;

#include <fuse.h>

static int nfs_getattr(const char *pathname, struct stat *statbuf)
{
  int retval = client_ptr->NFSPROC_GETATTR(pathname, statbuf);
  if (retval > 0) {
    retval = -EINVAL;
  }
  return retval;
}

static int nfs_mknod(const char *pathname, mode_t mode, dev_t dev)
{
  int retval = client_ptr->NFSPROC_MKNOD(pathname, mode, dev);
  if (retval > 0) {
    retval = -EINVAL;
  }
  return retval;
}

static struct fuse_operations nfs_oper = {
  .getattr = nfs_getattr,
  .mknod = nfs_mknod
};

int main(int argc, char **argv)
{
  gpr_timespec timeout{3, 0, GPR_TIMESPAN};
  auto channel = grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials());
  bool connected = channel->WaitForConnected<gpr_timespec>(timeout);
  if (connected) {
    std::cerr << "connected" << std::endl;
  } else {
    std::cerr << "timeout" << std::endl;
    exit(-1);
  }
  NFSClient *client_ptr = new NFSClient(channel);

  // just some tests...to be deleted
  struct stat statbuf;
  int code = client_ptr->NFSPROC_GETATTR("./a", &statbuf);
  // to be deleted

  fuse_main(argc, argv, &nfs_oper, NULL);
}
