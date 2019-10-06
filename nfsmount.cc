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

static int nfs_getattr(const char *pathname, struct stat *st)
{
  return 0;
}

static int nfs_mknod(const char *pathname, mode_t mode, dev_t dev)
{
  return client_ptr->NFSPROC_MKNOD(pathname, mode, dev);
}

static struct fuse_operations nfs_oper = {
  getattr : nfs_getattr,
  readlink : nullptr,
  getdir : nullptr,
  mknod : nfs_mknod,
  mkdir : nullptr,
  unlink : nullptr,
  rmdir : nullptr,
  symlink : nullptr,
  rename : nullptr,
  link : nullptr,
  chmod : nullptr,
  chown : nullptr,
  truncate : nullptr,
  utime : nullptr,
  open : nullptr,
  read : nullptr,
  write : nullptr,
  statfs : nullptr,
  flush : nullptr,
  release : nullptr,
  fsync : nullptr,
  setxattr : nullptr,
  getxattr : nullptr,
  listxattr : nullptr,
  removexattr : nullptr,
  opendir : nullptr,
  readdir : nullptr,
  releasedir : nullptr,
  fsyncdir : nullptr,
  init : nullptr,
  destroy : nullptr,
  access : nullptr,
  create : nullptr,
  ftruncate : nullptr,
  fgetattr : nullptr,
  lock : nullptr,
  utimens : nullptr,
  bmap : nullptr,
  flag_nullpath_ok : 1,
  flag_nopath : 1,
  flag_utime_omit_ok : 1,
  flag_reserved : 29,
  ioctl : nullptr,
  poll : nullptr,
  write_buf : nullptr,
  read_buf : nullptr,
  flock : nullptr,
  fallocate : nullptr
};

int main(int argc, char **argv)
{
  NFSClient *client_ptr = new NFSClient(
      grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials()));
  if (client_ptr->NFSPROC_NULL())
  {
    exit(-1);
  }
  std::cerr << "connected" << std::endl;
  return fuse_main(argc, argv, &nfs_oper, NULL);
}
