#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fuse.h>

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

    char *buf = new char[100000000];
    bzero(buf, 100000000);

    int ret, fh;
    ssize_t size_read;
    struct fuse_file_info fi;
    fi.fh = 3;
    fi.flags = O_RDONLY;
    ret = client.NFSPROC_OPEN("/verylarge.txt", &fi, &fh);
    std::cerr << "OPEN: " << ret << std::endl;
    std::cerr << "fh: " << fh << std:: endl;

    fi.fh = fh;
    ret = client.NFSPROC_READ(nullptr, buf, 100000000, 0, &fi, &size_read);
    std::cerr << "READ large: " << size_read << std::endl;

    //std::cerr << buf << std::endl;
}