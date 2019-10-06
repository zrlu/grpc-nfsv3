//
// Starter code for CS 454
// You SHOULD change this file
//
//

#include "watdfs_client.h"

#include "rpc.h"

using namespace std;

// You may want to include iostream or cstdio.h if you print to standard error.
#include <iostream>
#include <set>
#include <bitset>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unistd.h>
#include <time.h>
#include <cstdio>
#include <assert.h>

#define CHUNK_SIZE                MAX_ARRAY_LEN
#define MAX_NUM_FILE_HANDLERS     1024
#define NOW                       time((time_t*)0)
// #define CHECK_ERROR(__retcode__)  if (__retcode__ < 0) { fprintf(stderr, "WARNING: [%s] %s:%d => %d (errno: %d)\n", __FILE__, __FUNCTION__, __LINE__, __retcode__, errno );}
#define CHECK_ERROR(__retcode__) (void)__retcode__;

int fuse_file_info_size = sizeof(struct fuse_file_info);
int timespec_size = sizeof(struct timespec);
char *client_cache_dir = NULL;

char *get_cache_full_path(const char *short_path)
{
  int short_path_len = strlen(short_path);
  int dir_len = strlen(client_cache_dir);
  int full_len = dir_len + short_path_len + 1;

  char *full_path = (char *)malloc(full_len);

  // First fill in the directory.
  strcpy(full_path, client_cache_dir);
  // Then append the path.
  strcat(full_path, short_path);

  return full_path;
}

struct FileHandlerTableEntry
{
  int flags;
  int fh;
  std::string short_path;
  time_t last_validated;
};

class FileHandlerTable
{
public:

  std::unordered_map<std::string, FileHandlerTableEntry *> path_table;

  FileHandlerTable() : path_table(std::unordered_map<std::string, FileHandlerTableEntry *>()) {}
  int allocate(int fh, std::string fp, int flags)
  {
    if (path_table.find(fp) == path_table.end())
    {
      FileHandlerTableEntry *entry = new FileHandlerTableEntry();
      entry->fh = fh;
      entry->flags = flags;
      entry->short_path = fp;
      struct stat statbuf;
      char *cache_full_path = get_cache_full_path(fp.c_str());
      stat(cache_full_path, &statbuf);
      entry->last_validated = statbuf.st_mtime; 
      path_table[fp] = entry;
      return 0;
    }
    else
    {
      return -1;
    }
  }
  void deallocate(std::string fp)
  {
    FileHandlerTableEntry *ptr;
    auto it = path_table.find(fp);
    if (it != path_table.end())
    {
      ptr = it->second;
      path_table.erase(it);
      delete ptr;
    }
    else
    {
    }
  }
};

class UserData
{
public:
  FileHandlerTable *fhtable;
  char *path_to_cache;
  time_t cache_interval;
  UserData() : fhtable(new FileHandlerTable()) {}
  ~UserData()
  {
    delete fhtable;
  }
};

namespace A2
{

int watdfs_cli_open(void *userdata, const char *path, struct fuse_file_info *fi) {
  // Called during open.
  // You should fill in fi->fh.
  int num_args = 3;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)fi;
  args[2] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"a2open", arg_types, args);
  if (rpc_ret < 0) {
    fxn_ret = -EINVAL;
  } else {
    fxn_ret = *(int*)retcode;
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_release(void *userdata, const char *path,
                     struct fuse_file_info *fi) {
  // Called during close, but possibly asynchronously.
  int num_args = 3;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)fi;
  args[2] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"a2release", arg_types, args);
  if (rpc_ret < 0) {
    fxn_ret = -EINVAL;
  } else {
    fxn_ret = *(int*)retcode;
  }  
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_getattr(void *userdata, const char *path, struct stat *statbuf)
{
  int num_args = 3;
  void **args = (void **)malloc(3 * sizeof(void *));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  args[0] = (void *)path;
  arg_types[1] = (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | sizeof(struct stat); // statbuf
  args[1] = (void *)statbuf;
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  void *retcode = (void *)malloc(sizeof(int));
  args[2] = (void *)retcode;
  arg_types[3] = 0;
  int rpc_ret = rpcCall((char *)"a2getattr", arg_types, args);
  int fxn_ret = 0;
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  if (fxn_ret < 0)
  {
    memset(statbuf, 0, sizeof(struct stat));
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_fgetattr(void *userdata, const char *path, struct stat *statbuf,
                        struct fuse_file_info *fi)
{
  int num_args = 4;
  void **args = (void **)malloc(num_args * sizeof(void *));
  void *retcode = (void *)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void *)path;
  args[1] = (void *)statbuf;
  args[2] = (void *)fi;
  args[3] = (void *)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | sizeof(struct stat);
  arg_types[2] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[3] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[4] = 0;
  rpc_ret = rpcCall((char *)"a2fgetattr", arg_types, args);
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  if (fxn_ret < 0)
  {
    memset(statbuf, 0, sizeof(struct stat));
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

// CREATE, OPEN AND CLOSE
int watdfs_cli_mknod(void *userdata, const char *path, mode_t mode, dev_t dev)
{
  int num_args = 4;
  void **args = (void **)malloc(num_args * sizeof(void *));
  void *retcode = (void *)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void *)path;
  args[1] = (void *)&mode;
  args[2] = (void *)&dev;
  args[3] = (void *)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
  arg_types[2] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[3] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[4] = 0;
  rpc_ret = rpcCall((char *)"a2mknod", arg_types, args);
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_read_chunk(void *userdata, const char *path, char *buf, size_t size,
                          off_t offset, struct fuse_file_info *fi)
{
  int num_args = 6;
  void **args = (void **)malloc(num_args * sizeof(void *));
  void *retcode = (void *)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void *)path;
  args[1] = (void *)buf;
  args[2] = (void *)&size;
  args[3] = (void *)&offset;
  args[4] = (void *)fi;
  args[5] = (void *)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | size;
  arg_types[2] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[3] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[4] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[5] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[6] = 0;
  rpc_ret = rpcCall((char *)"a2read", arg_types, args);
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_read(void *userdata, const char *path, char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi)
{
  int fxn_ret = 0;
  int q = size / CHUNK_SIZE;
  int r = size % CHUNK_SIZE;
  int size_read = 0;
  for (int i = 0; i < q; ++i)
  {
    fxn_ret = watdfs_cli_read_chunk(userdata, path, &buf[i * CHUNK_SIZE], CHUNK_SIZE, offset + i * CHUNK_SIZE, fi);
    if (fxn_ret < 0)
    {
      return fxn_ret;
    }
    size_read += fxn_ret;
  }
  if (r)
  {
    fxn_ret = watdfs_cli_read_chunk(userdata, path, &buf[q * CHUNK_SIZE], r, offset + q * CHUNK_SIZE, fi);
    if (fxn_ret < 0)
    {
      return fxn_ret;
    }
    size_read += fxn_ret;
  }
  return size_read;
}

int watdfs_cli_write_chunk(void *userdata, const char *path, const char *buf,
                           size_t size, off_t offset, struct fuse_file_info *fi)
{
  int num_args = 6;
  void **args = (void **)malloc(num_args * sizeof(void *));
  void *retcode = (void *)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void *)path;
  args[1] = (void *)buf;
  args[2] = (void *)&size;
  args[3] = (void *)&offset;
  args[4] = (void *)fi;
  args[5] = (void *)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | size;
  arg_types[2] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[3] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[4] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[5] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[6] = 0;
  rpc_ret = rpcCall((char *)"a2write", arg_types, args);
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_write(void *userdata, const char *path, const char *buf,
                     size_t size, off_t offset, struct fuse_file_info *fi)
{
  int fxn_ret = 0;
  int q = size / CHUNK_SIZE;
  int r = size % CHUNK_SIZE;
  for (int i = 0; i < q; ++i)
  {
    fxn_ret = watdfs_cli_write_chunk(userdata, path, &buf[i * CHUNK_SIZE], CHUNK_SIZE, offset + i * CHUNK_SIZE, fi);
    if (fxn_ret < 0)
    {
      return fxn_ret;
    }
  }
  if (r)
  {
    fxn_ret = watdfs_cli_write_chunk(userdata, path, &buf[q * CHUNK_SIZE], r, offset + q * CHUNK_SIZE, fi);
    if (fxn_ret < 0)
    {
      return fxn_ret;
    }
  }
  return size;
}

int watdfs_cli_truncate(void *userdata, const char *path, off_t newsize)
{
  int num_args = 3;
  void **args = (void **)malloc(num_args * sizeof(void *));
  void *retcode = (void *)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void *)path;
  args[1] = (void *)&newsize;
  args[2] = (void *)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"a2truncate", arg_types, args);
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_fsync(void *userdata, const char *path,
                     struct fuse_file_info *fi)
{
  int num_args = 3;
  void **args = (void **)malloc(num_args * sizeof(void *));
  void *retcode = (void *)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void *)path;
  args[1] = (void *)fi;
  args[2] = (void *)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"a2fsync", arg_types, args);
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_utimens(void *userdata, const char *path,
                       const struct timespec ts[2])
{
  int num_args = 3;
  void **args = (void **)malloc(num_args * sizeof(void *));
  void *retcode = (void *)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void *)path;
  args[1] = (void *)ts;
  args[2] = (void *)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 2 * timespec_size;
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"a2utimens", arg_types, args);
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

} // namespace A2

int lock(const char *path, mode_t mode) {
  int num_args = 2;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)&mode;
  args[2] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | ( ARG_INT << 16);
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"lock", arg_types, args);
  if (rpc_ret < 0) {
    fxn_ret = -EINVAL;
  } else {
    fxn_ret = *(int*)retcode;
  }  
  free(args);
  free(retcode);
  return fxn_ret;
}

int unlock(const char *path, mode_t mode) {
  int num_args = 2;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)&mode;
  args[2] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | ( ARG_INT << 16);
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"unlock", arg_types, args);
  if (rpc_ret < 0) {
    fxn_ret = -EINVAL;
  } else {
    fxn_ret = *(int*)retcode;
  }  
  free(args);
  free(retcode);
  return fxn_ret;
}

// SETUP AND TEARDOWN
void *watdfs_cli_init(struct fuse_conn_info *conn, const char *path_to_cache,
                      time_t cache_interval)
{
  // You should set up the RPC library here, by calling rpcClientInit.
  int ret = rpcClientInit();
  if (ret != 0)
  {
    exit(ret);
  }
  // You should check the return code of the rpcClientInit call, as it may fail,
  // for example, if the incorrect port was exported. If there was an error,
  // it may be useful to print to stderr or stdout during debugging.
  // Important: Make sure you turn off logging prior to submission!
  // One useful technique is to use pre-processor flags like:
  // # ifdef PRINT_ERR
  // fprintf(stderr, "Failed to initialize RPC Client\n");
  // Or if you prefer c++:
  // #endif

  // You can also initialize any global state that you want to have in this
  // method, and return it. The value that you return here will be passed
  // as userdata in other functions.

  // path_to_cache and cache_interval are not necessary for Assignment 2, but should
  // be used in Assignment 3.
  UserData *ptr = new UserData();
  ptr->cache_interval = cache_interval;
  client_cache_dir = (char *)malloc(sizeof(char) * (strlen(path_to_cache) + 1));
  strcpy(client_cache_dir, path_to_cache);
  return (void *)ptr;
}

void watdfs_cli_destroy(void *userdata)
{
  // You should clean up your userdata state here.
  delete (UserData *)userdata;
  // You should also tear down the RPC library by calling rpcClientDestroy.
  rpcClientDestroy();
}

// rw_locks

int pull(const char *path, const char *cache_full_path, off_t size_server, struct timespec *ts) {
    int rpc_ret = 0;
    int sys_ret = 0;
    int fxn_ret = 0;
    char thisbuf[size_server];
    // download from remote
    struct fuse_file_info *new_fi = (struct fuse_file_info*)malloc(sizeof(struct fuse_file_info));
    new_fi->flags = O_RDONLY;

    lock(path, O_RDONLY);

    rpc_ret = A2::watdfs_cli_open(0, path, new_fi);
    if (rpc_ret == -ENOENT)
    {
      fxn_ret = -ENOENT;
    }
    CHECK_ERROR(rpc_ret);
    rpc_ret = A2::watdfs_cli_read(0, path, thisbuf, size_server, 0, new_fi);
    CHECK_ERROR(rpc_ret);
    rpc_ret = A2::watdfs_cli_release(0, path, new_fi);
    CHECK_ERROR(rpc_ret);

    unlock(path, O_RDONLY);

    free(new_fi);
    // write to local
    int fh = open(cache_full_path, O_CREAT | O_RDWR | O_TRUNC, 0666);
    CHECK_ERROR(fh);
    sys_ret = write(fh, thisbuf, size_server);
    CHECK_ERROR(sys_ret);
    sys_ret = fsync(fh);
    CHECK_ERROR(sys_ret);
    sys_ret = futimens(fh, ts);
    CHECK_ERROR(sys_ret);
    sys_ret = close(fh);
    CHECK_ERROR(sys_ret);


    return fxn_ret;
}

bool pull_as_needed(void *userdata, const char *path, const char *cache_full_path, struct fuse_file_info *fi)
{
  int rpc_ret = 0;
  int sys_ret = 0;
  int fxn_ret = 0;
  struct stat statbuf;

  FileHandlerTableEntry *entry = nullptr;
  
  auto it = ((UserData *)userdata)->fhtable->path_table.find(path);
  if (it != ((UserData *)userdata)->fhtable->path_table.end())
  {
    entry = it->second;
  } else {
    return 0;
  }

  if (NOW - entry->last_validated >= ((UserData *)userdata)->cache_interval) {

    sys_ret = stat(cache_full_path, &statbuf);
    time_t last_modified_client = statbuf.st_mtim.tv_sec;
    CHECK_ERROR(sys_ret);

    rpc_ret = A2::watdfs_cli_getattr(userdata, path, &statbuf);
    CHECK_ERROR(rpc_ret);
    time_t last_modified_server = statbuf.st_mtim.tv_sec;
    off_t size_server = statbuf.st_size;
    timespec ts[2];
    ts[0] = { 0, UTIME_OMIT };
    ts[1] = statbuf.st_mtim;

    if (last_modified_server > last_modified_client) {

      if (!entry)
      {
        // not opened
        pull(path, cache_full_path, size_server, ts);
      }
      else
      {
        // already opened
        sys_ret = close(entry->fh);
        CHECK_ERROR(sys_ret);

        pull(path, cache_full_path, size_server, ts);

        sys_ret = open(cache_full_path, entry->flags, 0666);
        CHECK_ERROR(sys_ret);
        entry->fh = sys_ret;
        if (fi)
        {
          fi->fh = sys_ret;
        }
      }
      fxn_ret = 1;
    }
    entry->last_validated = NOW;
  }
  return fxn_ret;
}

int push(const char *path, const char *cache_full_path, off_t size_client, struct timespec *ts) {

    int rpc_ret = 0;
    int sys_ret = 0;
    int fxn_ret = 0;
    char thisbuf[size_client];
    // read from local
    int fh = open(cache_full_path, O_RDONLY, 0666);
    CHECK_ERROR(fh);
    if (fh < 0 && errno == ENOENT)
    {
      fxn_ret = -ENOENT;
      return fxn_ret;
    }
    sys_ret = read(fh, thisbuf, size_client);
    CHECK_ERROR(sys_ret);
    sys_ret = close(fh);
    CHECK_ERROR(sys_ret);

    // upload to remote
    struct fuse_file_info *new_fi = (struct fuse_file_info*)malloc(sizeof(struct fuse_file_info));
    new_fi->flags = O_CREAT | O_WRONLY | O_TRUNC;

    lock(path, O_WRONLY);

    rpc_ret = A2::watdfs_cli_open(0, path, new_fi);
    CHECK_ERROR(rpc_ret);
    rpc_ret = A2::watdfs_cli_write(0, path, thisbuf, size_client, 0, new_fi);
    CHECK_ERROR(rpc_ret);
    rpc_ret = A2::watdfs_cli_utimens(0, path, ts);
    CHECK_ERROR(rpc_ret);
    rpc_ret = A2::watdfs_cli_release(0, path, new_fi);
    CHECK_ERROR(rpc_ret);

    unlock(path, O_WRONLY);

    free(new_fi);

    return fxn_ret;
}

bool push_as_needed(void *userdata, const char *path, const char *cache_full_path, struct fuse_file_info *fi)
{
  int rpc_ret = 0;
  int sys_ret = 0;
  int fxn_ret = 0;
  (void)rpc_ret;
  (void)sys_ret;
  struct stat statbuf;

  FileHandlerTableEntry *entry = nullptr;
  
  auto it = ((UserData *)userdata)->fhtable->path_table.find(path);
  if (it != ((UserData *)userdata)->fhtable->path_table.end())
  {
    entry = it->second;
  } else {
    return 0;
  }

  if (NOW - entry->last_validated >= ((UserData *)userdata)->cache_interval) {

    sys_ret = stat(cache_full_path, &statbuf);
    time_t last_modified_client = statbuf.st_mtim.tv_sec;
    off_t size_client = statbuf.st_size;
    timespec ts[2];
    ts[0] = { 0, UTIME_OMIT };
    ts[1] = statbuf.st_mtim;

    rpc_ret = A2::watdfs_cli_getattr(userdata, path, &statbuf);
    time_t last_modified_server = statbuf.st_mtim.tv_sec;


    if (last_modified_client > last_modified_server) {
      push(path, cache_full_path, size_client, ts);
    }
    entry->last_validated = NOW;
  }
  return fxn_ret;
}

// GET FILE ATTRIBUTES
int watdfs_cli_getattr(void *userdata, const char *path, struct stat *statbuf)
{

 // Change file access and modification times.
  int rpc_ret = 0;
  (void)rpc_ret;
  int fxn_ret = 0;
  int sys_ret = 0;
  char *cache_full_path = get_cache_full_path(path);

  auto it = ((UserData *)userdata)->fhtable->path_table.find(path);

  if (it == ((UserData *)userdata)->fhtable->path_table.end())
  {
    rpc_ret = A2::watdfs_cli_getattr(userdata, path, statbuf);
    CHECK_ERROR(rpc_ret);
    off_t size_server = statbuf->st_size;
    timespec ts_server[2];
    if (rpc_ret != -ENOENT)
    {
      ts_server[0] = { 0, UTIME_OMIT };
      ts_server[1] = statbuf->st_mtim;
    } else {
      ts_server[0] = { 0, UTIME_OMIT };
      ts_server[1] = { 0, UTIME_NOW };
    }
    pull(path, cache_full_path, size_server, ts_server);
  }

  sys_ret = stat(cache_full_path, statbuf);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0)
  {
    fxn_ret = -errno;
  }
  free(cache_full_path);
  return fxn_ret;
}

int watdfs_cli_fgetattr(void *userdata, const char *path, struct stat *statbuf,
                        struct fuse_file_info *fi)
{
  int rpc_ret = 0;
  (void)rpc_ret;
  int fxn_ret = 0;
  int sys_ret = 0;
  char *cache_full_path = get_cache_full_path(path);

  auto it = ((UserData *)userdata)->fhtable->path_table.find(path);

  if (it == ((UserData *)userdata)->fhtable->path_table.end())
  {
    rpc_ret = A2::watdfs_cli_getattr(userdata, path, statbuf);
    CHECK_ERROR(rpc_ret);
    off_t size_server = statbuf->st_size;
    timespec ts_server[2];
    if (rpc_ret != -ENOENT)
    {
      ts_server[0] = { 0, UTIME_OMIT };
      ts_server[1] = statbuf->st_mtim;
    } else {
      ts_server[0] = { 0, UTIME_OMIT };
      ts_server[1] = { 0, UTIME_NOW };
    }
    pull(path, cache_full_path, size_server, ts_server);
  }

  sys_ret = fstat(fi->fh, statbuf);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0)
  {
    fxn_ret = -errno;
  }
  free(cache_full_path);
  return fxn_ret;
}

// CREATE, OPEN AND CLOSE
int watdfs_cli_mknod(void *userdata, const char *path, mode_t mode, dev_t dev)
{
  int fxn_ret = 0;
  int sys_ret = 0;
  char * cache_full_path = get_cache_full_path(path);
  sys_ret = mknod(cache_full_path, mode, dev);

  CHECK_ERROR(sys_ret);

  if (sys_ret == 0)
  {
    push_as_needed(userdata, path, cache_full_path, nullptr);
  }
  if (sys_ret < 0)
  {
    fxn_ret = -errno;
  }
  free(cache_full_path);
  return fxn_ret;
}

int watdfs_cli_open(void *userdata, const char *path, struct fuse_file_info *fi)
{
  // Called during open.
  // You should fill in fi->fh.
  UserData *userdataptr = (UserData *)userdata;
  if (userdataptr->fhtable->path_table.find(path) != userdataptr->fhtable->path_table.end())
  {
    return -EMFILE;
  }
  int num_args = 3;
  void **args = (void **)malloc(num_args * sizeof(void *));
  void *retcode = (void *)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  int sys_ret = 0;
  char * cache_full_path = get_cache_full_path(path);
  struct stat statbuf;
  args[0] = (void *)path;
  args[1] = (void *)fi;
  args[2] = (void *)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;  
  rpc_ret = rpcCall((char *)"open", arg_types, args);
  CHECK_ERROR(sys_ret);
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  if (fxn_ret < 0)
  {
    free(args);
    free(retcode);
    free(cache_full_path);
    return fxn_ret;
  }

  // download metadata
  rpc_ret = A2::watdfs_cli_getattr(userdata, path, &statbuf);
  CHECK_ERROR(rpc_ret);

  sys_ret = open(cache_full_path, fi->flags, 0666);
  CHECK_ERROR(sys_ret);
  
  timespec ts[2];
  off_t size_server;

  A2::watdfs_cli_mknod(userdata, path, 0666, 0);

  if (sys_ret < 0 && errno == ENOENT) {
    size_server = statbuf.st_size;
    ts[0] = { 0, UTIME_OMIT };
    ts[1] = statbuf.st_mtim;
    pull(path, cache_full_path, size_server, ts);
    sys_ret = open(cache_full_path, fi->flags, 0666);
    CHECK_ERROR(sys_ret);
  }

  userdataptr->fhtable->allocate(sys_ret, path, fi->flags);
  fi->fh = sys_ret;
  free(args);
  free(retcode);
  free(cache_full_path);
  return fxn_ret;
}

int watdfs_cli_release(void *userdata, const char *path,
                       struct fuse_file_info *fi)
{
  // return 0;
  // Called during close, but possibly asynchronously.
  int num_args = 3;
  void **args = (void **)malloc(num_args * sizeof(void *));
  void *retcode = (void *)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int sys_ret = 0;
  (void)sys_ret;
  int rpc_ret = 0;
  int fxn_ret = 0;
  char *cache_full_path = get_cache_full_path(path);
  UserData *userdataptr = (UserData *)userdata;
  struct stat statbuf;
  args[0] = (void *)path;
  args[1] = (void *)fi;
  args[2] = (void *)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"release", arg_types, args);
  CHECK_ERROR(rpc_ret);
  if (rpc_ret < 0)
  {
    fxn_ret = -EINVAL;
  }
  else
  {
    fxn_ret = *(int *)retcode;
  }
  if (fxn_ret < 0)
  {
    free(cache_full_path);
    free(args);
    free(retcode);
    return fxn_ret;
  }
  sys_ret = close(fi->fh);
  CHECK_ERROR(sys_ret);
  sys_ret = stat(cache_full_path, &statbuf);
  CHECK_ERROR(sys_ret);
  off_t size_client = statbuf.st_size;
  timespec ts[2];
  ts[0] = { 0, UTIME_OMIT };
  ts[1] = statbuf.st_mtim;

  push(path, cache_full_path, size_client, ts);

  userdataptr->fhtable->deallocate(path);
  free(cache_full_path);
  free(args);
  free(retcode);
  return fxn_ret;
}

// READ AND WRITE DATA
int watdfs_cli_read(void *userdata, const char *path, char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi)
{
  int rpc_ret = 0;
  int fxn_ret = 0;
  int sys_ret = 0;
  (void)rpc_ret;
  char *cache_full_path = get_cache_full_path(path);

  pull_as_needed(userdata, path, cache_full_path, fi);
 
  sys_ret = pread(fi->fh, buf, size, offset);
  CHECK_ERROR(sys_ret);
  free(cache_full_path);
  fxn_ret = sys_ret;
  if (sys_ret < 0)
  {
    fxn_ret = -errno;
  }
  return fxn_ret;
}

int watdfs_cli_write(void *userdata, const char *path, const char *buf,
                     size_t size, off_t offset, struct fuse_file_info *fi)
{
  int rpc_ret = 0;
  int sys_ret = 0;
  int fxn_ret = 0;
  (void)rpc_ret;
  char *cache_full_path = get_cache_full_path(path);
  sys_ret = pwrite(fi->fh, buf, size, offset);
  CHECK_ERROR(sys_ret);  
  push_as_needed(userdata, path, cache_full_path, fi);
  free(cache_full_path);
  fxn_ret = sys_ret;
  if (sys_ret < 0)
  {
    fxn_ret = -errno;
  }
  return fxn_ret;
}

int watdfs_cli_truncate(void *userdata, const char *path, off_t newsize)
{
  // Change the file size to newsize.
  int rpc_ret = 0;
  int fxn_ret = 0;
  int sys_ret = 0;
  (void)rpc_ret;
  char *cache_full_path =  get_cache_full_path(path);

  pull_as_needed(userdata, path, cache_full_path, nullptr);

  sys_ret = truncate(cache_full_path, newsize);
  CHECK_ERROR(sys_ret);

  if (sys_ret == 0)
  {
    push_as_needed(userdata, path, cache_full_path, nullptr);
  }

  free(cache_full_path);
  if (sys_ret < 0)
  {
    fxn_ret = -errno;
  }
  return fxn_ret;
}

int watdfs_cli_fsync(void *userdata, const char *path,
                     struct fuse_file_info *fi)
{
  // Force a flush of file data.
  int fxn_ret = 0;
  int sys_ret = 0;
  char *cache_full_path = get_cache_full_path(path);
  struct stat statbuf;

  sys_ret = stat(cache_full_path, &statbuf);
  CHECK_ERROR(sys_ret);
  off_t size_client = statbuf.st_size;
  timespec ts[2];
  ts[0] = { 0, UTIME_OMIT };
  ts[1] = statbuf.st_mtim;
  if (sys_ret == 0)
  {
    push(path, cache_full_path, size_client, ts);
  }
  if (sys_ret < 0)
  {
    fxn_ret = -errno;
  }
  free(cache_full_path);
  return fxn_ret;
}

// CHANGE METADATA
int watdfs_cli_utimens(void *userdata, const char *path,
                       const struct timespec ts[2])
{
  // Change file access and modification times.
  int rpc_ret = 0;
  int fxn_ret = 0;
  int sys_ret = 0;
  (void)rpc_ret;
  char *cache_full_path = get_cache_full_path(path);
  struct stat statbuf;

  pull_as_needed(userdata, path, cache_full_path, nullptr);

  sys_ret = utimensat(0, cache_full_path, ts, 0);
  CHECK_ERROR(sys_ret);

  sys_ret = stat(cache_full_path, &statbuf);
  CHECK_ERROR(sys_ret);

  sys_ret = stat(cache_full_path, &statbuf);
  CHECK_ERROR(sys_ret);
  timespec ts_[2];
  memcpy(ts_, ts, 2*sizeof(struct timespec));
  if (sys_ret == 0)
  {
    push_as_needed(userdata, path, cache_full_path, nullptr);
  }
  if (sys_ret < 0)
  {
    fxn_ret = -errno;
  }
  free(cache_full_path);
  return fxn_ret;
}
