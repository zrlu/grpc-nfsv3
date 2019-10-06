#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

int main() {
        int fd = open("mount/a", O_CREAT | O_WRONLY);
        printf("%d %d\n", fd, errno);
        char buf[8] = "teststr\n";
        write(fd, buf, 8);
        close(fd);
}
