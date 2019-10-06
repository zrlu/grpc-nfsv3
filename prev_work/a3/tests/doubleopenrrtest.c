#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
        int fd = open("mount/a", O_RDONLY);
        printf("%d\n", fd);
        int fd2 = open("mount2/a", O_RDONLY);
        printf("%d\n", fd2);
        char buf_read[7];
        int ret = read(fd2, buf_read, 7);
        printf("%s\n", buf_read);
        ret = read(fd, buf_read, 7);
        printf("%s\n", buf_read);
        close(fd);
        close(fd2);
}
