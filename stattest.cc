#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

int main()
{
    struct stat statbuf;
    int ret = stat("./client_cache/a", &statbuf);
    std::cout << ret << std::endl;
    std::cout << statbuf.st_ino << std::endl;
    return 0;
}