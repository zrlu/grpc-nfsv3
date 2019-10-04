#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

int main() {
        int ret = truncate("mount/a", 1000);  
        printf("%d %d\n", ret, errno);
}
