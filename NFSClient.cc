/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include <sys/types.h>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#ifdef BAZEL_BUILD
#include "examples/protos/nfs.grpc.pb.h"
#else
#include "nfs.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using nfs::NULLargs;
using nfs::NULLres;

using nfs::MKNODargs;
using nfs::MKNODres;

using nfs::NFS;

class NFSClient
{
public:
  NFSClient(std::shared_ptr<Channel> channel) : stub_(NFS::NewStub(channel))
  {
  }

  void NFSPROC_NULL()
  {
    ClientContext context;
    nfs::NULLres res;
    stub_->NFSPROC_NULL(&context, nfs::NULLargs(), &res);
  }

  void NFSPROC_MKNOD(const char *pathname, mode_t mode, dev_t dev)
  {
    ClientContext context;
    nfs::MKNODres res;
    nfs::MKNODargs args;
    args.set_pathname(pathname);
    args.set_mode(mode);
    args.set_dev(dev);
    stub_->NFSPROC_MKNOD(&context, args, &res);
  }

private:
  std::unique_ptr<NFS::Stub> stub_;
};

int main(int argc, char **argv)
{
  NFSClient client(
      grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials()));
      client.NFSPROC_NULL();

  return 0;
}
