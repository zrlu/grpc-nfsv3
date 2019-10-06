#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
        int fd = open("mount/a", O_WRONLY, 0666);
        printf("%d\n", fd);
        fd = open("mount2/a", O_WRONLY, 0666);
        printf("%d\n", fd);
}
