#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
        int fd = open("mount/a", O_WRONLY, 0666);
        printf("%d\n", fd);
        int fd2 = open("mount2/a", O_RDONLY);
        printf("%d\n", fd2);
        char buf_read[7];
        char *buf_write = "Hello!\n";
        int ret = write(fd, buf_write, 7);
        ret = read(fd2, buf_read, 7);
        printf("%s\n", buf_read);
        close(fd);
        close(fd2);
}
