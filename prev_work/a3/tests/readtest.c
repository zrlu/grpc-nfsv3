#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

int main() {
        int fd = open("mount/a", O_RDONLY);
        printf("%d %d\n", fd, errno);
        char buf[10];
        read(fd, buf, 10);
        printf("%s\n", buf);
        close(fd);
}
