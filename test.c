

    /*
      FUSE: Filesystem in Userspace
      Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
      
      This program can be distributed under the terms of the GNU GPL.
      See the file COPYING.
      gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
    */
     
    #define FUSE_USE_VERSION 26 //先定义, fuse.h中有判断
    #include <fuse.h>
    #include <stdio.h>
    #include <string.h>
    #include <errno.h>
    #include <fcntl.h>
     
    static const char *hello_str = "Hello World!\n";
    static const char *hello_path = "/hello";
    // 与函数stat()类似，用于得到文件属性，并将其存入到结构体struct stat当中 struct stat *stbuf
    static int hello_getattr(const char *path, struct stat *stbuf)
    {
        int res = 0;
        memset(stbuf, 0, sizeof(struct stat));  // 使用memset进行初始化结构体
        if (strcmp(path, "/") == 0) {
            stbuf->st_mode = S_IFDIR | 0755; // S_IFDIR 用于说明 / 为目录
            stbuf->st_nlink = 2;
        } else if (strcmp(path, hello_path) == 0) {
            stbuf->st_mode = S_IFREG | 0444; // S_IFREG 用于说明/hello 为常规文件
            stbuf->st_nlink = 1;
            stbuf->st_size = strlen(hello_str); // 设置文件长度为hello_str的长度
        } else
            res = -ENOENT; // 返回错误信息，没有该文件或者目录
     
        return res; // 成功执行的时候，此函数返回值为 0
    }
    // 该函数用于读取目录中的内容，并在/目录下增加了. .. hello 三个目录项
    static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi)
    {
        (void) offset;
        (void) fi;
        if (strcmp(path, "/") != 0)
            return -ENOENT;
        /* fill, 其作用是在readdir函数中增加一个目录项
        typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
            const struct stat *stbuf, off_t off);
        */
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, hello_path + 1, NULL, 0); //指针+1(/hello), 即增加 hello 目录项，去掉前面的'/'
     
        return 0;
    }
    // 打开文件函数
    static int hello_open(const char *path, struct fuse_file_info *fi)
    {
        if (strcmp(path, hello_path) != 0)
            return -ENOENT;
        if ((fi->flags & 3) != O_RDONLY)
            return -EACCES;
        return 0;
    }
    // 读文件函数
    static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi)
    {
        size_t len;
        (void) fi;
        if(strcmp(path, hello_path) != 0)
            return -ENOENT;
        len = strlen(hello_str);
        if (offset < len) {
            if (offset + size > len)
                size = len - offset;
            memcpy(buf, hello_str + offset, size);
        } else
            size = 0;
        return size;
    }
    // 注册自定义函数

    static struct fuse_operations hello_oper = {
        .getattr    = hello_getattr,
        .readdir    = hello_readdir,
        .open        = hello_open,
        .read        = hello_read, // 读文件函数
    };
    // 调用 fuse_main , 把控制权交给了fuse
    int main(int argc, char *argv[])
    {
        return fuse_main(argc, argv, &hello_oper, NULL);
    }

