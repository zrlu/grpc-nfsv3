#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <fstream>

using namespace std;

struct RPCRecord
{

};

int main()
{
    long a = 65535;
    fstream fs;
    fs.open("binlog", fstream::in | fstream::out | fstream::app);
    fs << "Hello World!" << endl;
    fs << a << endl;
    fs.flush();
    fs.close();

}