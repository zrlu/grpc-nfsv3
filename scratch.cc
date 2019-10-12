#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <string.h>

using namespace std;

int main()
{
    const char *a = "123";
    char buffer[3];
    memcpy(buffer, a, 4);
    puts(buffer);
}