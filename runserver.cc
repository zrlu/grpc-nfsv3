#include "NFSServer.h"

void runServer(const char *path)
{
  std::string server_address("0.0.0.0:50055");
  NFSImpl service(path);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cerr << "Server listening on " << server_address << std::endl;
  std::cerr << "Server storage path is " << path << std::endl;

  server->Wait();
}

int main(int argc, char **argv)
{
  runServer(argv[1]);
  return 0;
}
