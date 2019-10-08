#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

#include "NFSClient.h"

int main()
{
    auto channel = grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials());
    gpr_timespec timeout{10, 0, GPR_TIMESPAN};
    bool connected = channel->WaitForConnected<gpr_timespec>(timeout);
    if (connected)
    {
        std::cerr << "connected" << std::endl;
    }
    else
    {
        std::cerr << "timeout" << std::endl;
        exit(-1);
    }
    auto client = NFSClient(channel);

    struct stat statbuf;
    int ret;
    
    ret = client.NFSPROC_GETATTR("./a.txt", &statbuf);
    std::cerr << ret << std::endl;

    ret = client.NFSPROC_GETATTR("./dne.txt", &statbuf);
    std::cerr << ret << std::endl;
}