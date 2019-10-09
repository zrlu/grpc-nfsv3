#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string>

using namespace std;

int main()
{
    string a = "123";
    char buffer[3];
    a.copy(buffer, 3);
    puts(buffer);
}