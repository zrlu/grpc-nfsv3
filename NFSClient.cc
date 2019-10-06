#include "NFSClient.h"

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

NFSClient::NFSClient(std::shared_ptr<Channel> channel) : stub_(NFS::NewStub(channel)) {}

void NFSClient::NFSPROC_NULL()
{
  ClientContext context;
  nfs::NULLres res;
  stub_->NFSPROC_NULL(&context, nfs::NULLargs(), &res);
}

void NFSClient::NFSPROC_MKNOD(const char *pathname, mode_t mode, dev_t dev)
{
  ClientContext context;
  nfs::MKNODres res;
  nfs::MKNODargs args;
  args.set_pathname(pathname);
  args.set_mode(mode);
  args.set_dev(dev);
  stub_->NFSPROC_MKNOD(&context, args, &res);
}

NFSClient *client_ptr;

int main(int argc, char **argv)
{
  NFSClient *client_ptr = new NFSClient(
      grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials()));

  return 0;
}
