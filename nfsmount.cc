#include "NFSClient.h"

#include <iostream>

NFSClient *client_ptr;

int main(int argc, char **argv)
{
  NFSClient *client_ptr = new NFSClient(
      grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials())
  );
  std::cerr << "connected" << std::endl;
  return 0;
}