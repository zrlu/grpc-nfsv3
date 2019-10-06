#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#include <iostream>

#include "NFSClient.h"

NFSClient *client_ptr;

int main(int argc, char **argv)
{
  NFSClient *client_ptr = new NFSClient(
      grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials())
  );
  std::cerr << "connected" << std::endl;
  return 0;
}