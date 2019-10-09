#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

int main()
{
    DIR *dp;
    struct dirent *de;

    dp = opendir("server_cache");
    if (dp == NULL) return -errno;

    de = readdir(dp);

    puts(de->d_name);
    
}