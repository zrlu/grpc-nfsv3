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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#ifdef BAZEL_BUILD
#include "examples/protos/nfs.grpc.pb.h"
#else
#include "nfs.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using nfs::NFS;
using nfs::NULLargs;
using nfs::NULLres;

using nfs::MKNODargs;
using nfs::MKNODres;

using std::chrono::system_clock;

class NFSImpl final : public NFS::Service
{
  std::mutex mu_;
  const std::string m_serverStoragePath;
  const char *getFullPath(const std::string &suffix) 
  {
    return (m_serverStoragePath + suffix).c_str();
  }

protected:
public:
  NFSImpl(const std::string &path): 
  m_serverStoragePath(path) {}

  Status NFSPROC_NULL(ServerContext *context, const NULLargs *request, NULLres *response) override
  {
    std::cerr << "NFSPROC3_NULL" << std::endl;
    nfs::NULLres res;
    *response = res;
    return Status::OK;
  }

  Status NFSPROC_MKNOD(ServerContext *context, const MKNODargs *request, MKNODres *response) override
  {
    std::cerr << "NFSPROC3_MKNOD" << std::endl;
    nfs::MKNODres res;
    res.set_ret(0);

    const char *pathname = getFullPath(request->pathname().c_str());
    mode_t mode = request->mode();
    dev_t dev = request->dev();

    if (!~mknod(pathname, mode, dev))
    {
      res.set_ret(-errno);
    }
    
    *response = res;
    return Status::OK;
  }

};

void RunServer(const char *path)
{
  std::string server_address("0.0.0.0:50055");
  NFSImpl service(path);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.SetMaxReceiveMessageSize(INT_MAX);
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cerr << "Server listening on " << server_address << std::endl;
  std::cerr << "Server storage path is " << path << std::endl;

  server->Wait();
}

int main(int argc, char **argv)
{
  RunServer(argv[1]);
  return 0;
}
