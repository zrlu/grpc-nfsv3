//
// Starter code for CS 454
// You SHOULD change this file
//
//

#include "rpc.h"

// You may need to change your includes depending on whether you use C or C++.
#include <fuse.h>

// Needed for stat.
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Needed for errors.
#include <errno.h>

// Needed for string operations.
#include <cstring>

// Need malloc and free.
#include <cstdlib>

// You may want to include iostream or cstdio.h if you print to standard error.
#include <iostream>
#include <cstdio>
#include <unordered_map>
#include <string>
#include "rw_lock.h"
#include <pthread.h>
#include <fcntl.h>

using namespace std;

// #define CHECK_ERROR(__retcode__)  if (__retcode__ < 0) { fprintf(stderr, "WARNING: [%s] %s:%d => %d (errno: %d)\n", __FILE__, __FUNCTION__, __LINE__, __retcode__, errno );}
#define CHECK_ERROR(__retcode__) (void)__retcode__;

struct LockTableEntry
{
  rw_lock_t *lock;
  int num_readers;
  int num_writers;
  
  std::unordered_map<int, int> clients;
  LockTableEntry(): num_readers(0), num_writers(0)
  {
    lock = (rw_lock_t *)malloc(sizeof(rw_lock_t));
    rw_lock_init(this->lock);
  }
  ~LockTableEntry()
  {
    rw_lock_destroy(this->lock);
    free(lock);
  }
};

typedef std::unordered_map<std::string, LockTableEntry*> LockTable;
LockTable lock_table = LockTable();
pthread_mutex_t lock_table_mutex = PTHREAD_MUTEX_INITIALIZER;

// Global state server_persist_dir.
char *server_persist_dir = NULL;

// We need to operate on the path relative to the the server_persist_dir.
// This function returns a path that appends the given short path to the
// server_persist_dir. The character array is allocated on the heap, therefore
// it should be freed after use.
char *get_full_path(char *short_path)
{
  int short_path_len = strlen(short_path);
  int dir_len = strlen(server_persist_dir);
  int full_len = dir_len + short_path_len + 1;

  char *full_path = (char *)malloc(full_len);

  // First fill in the directory.
  strcpy(full_path, server_persist_dir);
  // Then append the path.
  strcat(full_path, short_path);

  return full_path;
}

namespace A2
{
  
int watdfs_getattr(int *argTypes, void **args)
{
  // Get the arguments.
  // The first argument is the path relative to the mountpoint.
  char *short_path = (char *)args[0];
  // The second argument is the stat structure, which should be filled in
  // by this function.
  struct stat *statbuf = (struct stat *)args[1];
  // The third argument is the return code, which will be 0, or -errno.
  int *ret = (int *)args[2];

  // Get the local file name, so we call our helper function which appends
  // the server_persist_dir to the given path.
  char *full_path = get_full_path(short_path);

  // Initially we set set the return code to be 0.
  *ret = 0;

  // Make the stat system call, which is the corresponding system call needed
  // to support getattr. You should make the stat system call here:

  // Let sys_ret be the return code from the stat system call.
  int sys_ret = 0;

  // You should use the statbuf as an argument to the stat system call, but it
  // is currently unused.
  // (void)statbuf;
  sys_ret = stat(full_path, statbuf);
  CHECK_ERROR(sys_ret);

  if (sys_ret < 0)
  {
    // If there is an error on the system call, then the return code should
    // be -errno.
    *ret = -errno;
  }

  // Clean up the full path, it was allocated on the heap.
  free(full_path);

  // The RPC call should always succeed, so return 0.
  return 0;
}

int watdfs_fgetattr(int *argTypes, void **args)
{
  int sys_ret = 0;
  char *short_path = (char *)args[0];
  struct stat *statbuf = (struct stat *)args[1];
  struct fuse_file_info *fi = (struct fuse_file_info *)args[2];
  int *ret = (int *)args[3];
  char *full_path = get_full_path(short_path);
  *ret = 0;
  sys_ret = fstat(fi->fh, statbuf);
  CHECK_ERROR(sys_ret);

  if (sys_ret < 0)
  {
    *ret = -errno;
  }
  free(full_path);
  return 0;
}

int watdfs_mknod(int *argTypes, void **args)
{
  int sys_ret = 0;
  char *short_path = (char *)args[0];
  mode_t mode = *(mode_t *)args[1];
  dev_t dev = *(dev_t *)args[2];
  int *ret = (int *)args[3];
  char *full_path = get_full_path(short_path);
  *ret = 0;
  sys_ret = mknod(full_path, mode, dev);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0)
  {
    *ret = -errno;
  }
  free(full_path);
  return 0;
}

int watdfs_open(int *argTypes, void **args)
{
  int sys_ret = 0;
  char *short_path = (char *)args[0];
  struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
  int *ret = (int *)args[2];
  char *full_path = get_full_path(short_path);
  *ret = 0;
  sys_ret = open(full_path, fi->flags, 0666);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0)
  {
    *ret = -errno;
  }
  else
  {
    fi->fh = sys_ret;
  }
  free(full_path);
  return 0;
}

int watdfs_release(int *argTypes, void **args)
{
  int sys_ret = 0;
  char *short_path = (char *)args[0];
  struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
  int *ret = (int *)args[2];
  char *full_path = get_full_path(short_path);
  *ret = 0;
  sys_ret = close(fi->fh);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0)
  {
    *ret = -errno;
  }
  free(full_path);
  return 0;
}

int watdfs_read(int *argTypes, void **args) {
  int sys_ret = 0;
  char *short_path = (char*)args[0];
  char *buf = (char*)args[1];
  size_t size = *(size_t*)args[2];
  off_t offset = *(off_t*)args[3];
  struct fuse_file_info *fi = (struct fuse_file_info*)args[4];
  int *ret = (int*)args[5];
  char *full_path = get_full_path(short_path);
  *ret = 0;
  sys_ret = pread(fi->fh, buf, size, offset);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0) {
    *ret = -errno;
  }
  *ret = sys_ret;
  free(full_path);
  return 0;
}

int watdfs_write(int *argTypes, void **args)
{
  int sys_ret = 0;
  char *short_path = (char *)args[0];
  char *buf = (char *)args[1];
  size_t size = *(size_t *)args[2];
  off_t offset = *(off_t *)args[3];
  struct fuse_file_info *fi = (struct fuse_file_info *)args[4];
  int *ret = (int *)args[5];
  char *full_path = get_full_path(short_path);
  *ret = 0;
  sys_ret = pwrite(fi->fh, buf, size, offset);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0)
  {
    *ret = -errno;
  }
  *ret = size;
  free(full_path);
  return 0;
}

int watdfs_truncate(int *argTypes, void **args)
{
  int sys_ret = 0;
  char *short_path = (char *)args[0];
  off_t offset = *(off_t *)args[1];
  int *ret = (int *)args[2];
  char *full_path = get_full_path(short_path);
  *ret = 0;
  sys_ret = truncate(full_path, offset);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0)
  {
    *ret = -errno;
  }
  free(full_path);
  return 0;
}

int watdfs_fsync(int *argTypes, void **args)
{
  int sys_ret = 0;
  char *short_path = (char *)args[0];
  struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
  int *ret = (int *)args[2];
  char *full_path = get_full_path(short_path);
  *ret = 0;
  sys_ret = fsync(fi->fh);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0)
  {
    *ret = -errno;
  }
  free(full_path);
  return 0;
}

int watdfs_utimens(int *argTypes, void **args)
{
  int sys_ret = 0;
  char *short_path = (char *)args[0];
  struct timespec *ts = (struct timespec *)args[1];
  int *ret = (int *)args[2];
  char *full_path = get_full_path(short_path);
  *ret = 0;
  sys_ret = utimensat(0, full_path, ts, 0);
  CHECK_ERROR(sys_ret);
  if (sys_ret < 0)
  {
    *ret = -errno;
  }
  free(full_path);
  return 0;
}

} // namespace A2

int lock(int *argTypes, void **args)
{
  char *short_path = (char *)args[0];
  mode_t mode = *(int *)args[1];
  int *ret = (int *)args[2];
  int lock_retcode;
  *ret = 0;
  auto it = lock_table.find(short_path);
  if (it == lock_table.end())
  {
    *ret = -ENOENT;
  }
  else
  {
    switch (mode | O_ACCMODE)
    {
    case O_RDONLY:
      lock_retcode = rw_lock_lock(it->second->lock, RW_READ_LOCK);
      if (lock_retcode < 0)
      {
        *ret = -errno;
      }
      break;
    case O_WRONLY:
      lock_retcode = rw_lock_lock(it->second->lock, RW_WRITE_LOCK);
      if (lock_retcode < 0)
      {
        *ret = -errno;
      }
      break;
    case O_RDWR:
      lock_retcode = rw_lock_lock(it->second->lock, RW_READ_LOCK);
      if (lock_retcode < 0)
      {
        *ret = -errno;
        break;
      }
      lock_retcode = rw_lock_lock(it->second->lock, RW_WRITE_LOCK);
      if (lock_retcode < 0)
      {
        *ret = -errno;
      }
      break;
    default:
      *ret = -EINVAL;
    }
  }
  return 0;
}

int unlock(int *argTypes, void **args)
{
  char *short_path = (char *)args[0];
  mode_t mode = *(int*)args[1];
  int *ret = (int *)args[2];
  int lock_retcode;
  *ret = 0;

  auto it = lock_table.find(short_path);
  if (it == lock_table.end())
  {
    *ret = -ENOENT;
  } else {
    switch (mode | O_ACCMODE)
    {
    case O_RDONLY:
      lock_retcode = rw_lock_unlock(it->second->lock, RW_READ_LOCK);
      if (lock_retcode < 0)
      {
        *ret = -errno;
      }
      break;
    case O_WRONLY:
      lock_retcode = rw_lock_unlock(it->second->lock, RW_WRITE_LOCK);
      if (lock_retcode < 0)
      {
        *ret = -errno;
      }
      break;
    case O_RDWR:
      lock_retcode = rw_lock_unlock(it->second->lock, RW_READ_LOCK);
      if (lock_retcode < 0)
      {
        *ret = -errno;
        break;
      }
      lock_retcode = rw_lock_unlock(it->second->lock, RW_WRITE_LOCK);
      if (lock_retcode < 0)
      {
        *ret = -errno;
      }
      break;
    default:
      *ret = -EINVAL;
    }
  }


  return 0;
}

int watdfs_open(int *argTypes, void **args)
{
  char *short_path = (char *)args[0];
  struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
  int *ret = (int *)args[2];
  *ret = 0;
  pthread_mutex_lock(&lock_table_mutex);
  auto it = lock_table.find(short_path);
  LockTableEntry *entry;
  if (it == lock_table.end())
  {
    // does not exists
    entry = new LockTableEntry();
    if ((fi->flags & O_ACCMODE) == O_RDONLY)
    {
      entry->num_readers++;
    }
    if ((fi->flags & O_ACCMODE) == O_WRONLY)
    {
      entry->num_writers++;
    }
    if ((fi->flags & O_ACCMODE) == O_RDWR)
    {
      entry->num_writers++;
      entry->num_readers++;
    }
    lock_table[short_path] = entry;
  } else {
    // exists
    entry = it->second;
    if (entry->num_writers > 0 && ((fi->flags & O_ACCMODE) == O_WRONLY || (fi->flags & O_ACCMODE) == O_RDWR))
    {
      // already opened in write mode
      *ret = -EACCES;
      pthread_mutex_unlock(&lock_table_mutex);
      return 0;
    }
    if ((fi->flags & O_ACCMODE) == O_RDONLY)
    {
      entry->num_readers++;
    }
    if ((fi->flags & O_ACCMODE) == O_WRONLY)
    {
      entry->num_writers++;
    }
    if ((fi->flags & O_ACCMODE) == O_RDWR)
    {
      entry->num_writers++;
      entry->num_readers++;
    }
  }
  pthread_mutex_unlock(&lock_table_mutex);
  return 0;
}

int watdfs_release(int *argTypes, void **args)
{
  char *short_path = (char *)args[0];
  struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
  int *ret = (int *)args[2];
  *ret = 0;
  pthread_mutex_lock(&lock_table_mutex);
  auto it = lock_table.find(short_path);
  LockTableEntry *entry;
  (void)fi;
  (void)entry;
  (void)it;
  entry = it->second;
  if ((fi->flags & O_ACCMODE) == O_RDONLY)
  {
    entry->num_readers--;
  }
  if ((fi->flags & O_ACCMODE) == O_WRONLY)
  {
    entry->num_writers--;
  }
  if ((fi->flags & O_ACCMODE) == O_RDWR)
  {
    entry->num_writers--;
    entry->num_readers--;
  }
  if (entry->num_readers == 0 && entry->num_writers == 0)
  {
    lock_table.erase(it);
    delete entry;
  }
  pthread_mutex_unlock(&lock_table_mutex);

  return 0;
}

// The main function of the server.
int main(int argc, char *argv[])
{
  // argv[1] should contain the directory where you should store data on the
  // server. If it is not present it is an error, that we cannot recover from.
  if (argc != 2)
  {
    // In general you shouldn't print to stderr or stdout, but it may be
    // helpful here for debugging. Important: Make sure you turn off logging
    // prior to submission!
    // See watdfs_client.c for more details
    // # ifdef PRINT_ERR
    // fprintf(stderr, "Usage: %s server_persist_dir\n", argv[0]);
    // Or if you prefer c++:
    // #endif
    return -1;
  }
  // Store the directory in a global variable.
  server_persist_dir = argv[1];

  // Initialize the rpc library by calling rpcServerInit. You should call
  // rpcServerInit here:
  int ret = 0;
  ret = rpcServerInit();

  // If there is an error with rpcServerInit, it maybe useful to have
  // debug-printing here, and then you should return.

  if (ret != 0)
  {
    exit(ret);
  }

  // Register your functions with the RPC library.
  {
    // There are 3 args for the function (see watdfs_client.c for more detail).
    int getattr_argTypes[4];
    // First is the path.
    getattr_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    // Note for arrays we can set the length to be anything  > 1.

    // The second argument is the statbuf.
    getattr_argTypes[1] = (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    // The third argument is the retcode.
    getattr_argTypes[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    // Finally we fill in the null terminator.
    getattr_argTypes[3] = 0;

    // We need to register the function with the types and the name.
    ret = rpcRegister((char *)"a2getattr", getattr_argTypes, A2::watdfs_getattr);
    if (ret < 0)
    {
      // It may be useful to have debug-printing here.
      return ret;
    }
  }

  {
    int fgetattr_argTypes[5];
    fgetattr_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    fgetattr_argTypes[1] = (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    fgetattr_argTypes[2] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    fgetattr_argTypes[3] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    fgetattr_argTypes[4] = 0;
    ret = rpcRegister((char *)"a2fgetattr", fgetattr_argTypes, A2::watdfs_fgetattr);
    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int mknod_argTypes[5];
    mknod_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    mknod_argTypes[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
    mknod_argTypes[2] = (1 << ARG_INPUT) | (ARG_LONG << 16);
    mknod_argTypes[3] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    mknod_argTypes[4] = 0;
    ret = rpcRegister((char *)"a2mknod", mknod_argTypes, A2::watdfs_mknod);
    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int open_argTypes[4];
    open_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    open_argTypes[1] = (1 << ARG_INPUT) | (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    open_argTypes[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    open_argTypes[3] = 0;
    ret = rpcRegister((char *)"open", open_argTypes, watdfs_open);
    ret = rpcRegister((char *)"a2open", open_argTypes, A2::watdfs_open);
    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int release_argTypes[4];
    release_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    release_argTypes[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    release_argTypes[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    release_argTypes[3] = 0;
    ret = rpcRegister((char *)"release", release_argTypes, watdfs_release);
    ret = rpcRegister((char *)"a2release", release_argTypes, A2::watdfs_release);

    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int read_argTypes[7];
    read_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    read_argTypes[1] = (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    read_argTypes[2] = (1 << ARG_INPUT) | (ARG_LONG << 16);
    read_argTypes[3] = (1 << ARG_INPUT) | (ARG_LONG << 16);
    read_argTypes[4] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    read_argTypes[5] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    read_argTypes[6] = 0;
    ret = rpcRegister((char *)"a2read", read_argTypes, A2::watdfs_read);
    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int write_argTypes[7];
    write_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    write_argTypes[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    write_argTypes[2] = (1 << ARG_INPUT) | (ARG_LONG << 16);
    write_argTypes[3] = (1 << ARG_INPUT) | (ARG_LONG << 16);
    write_argTypes[4] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    write_argTypes[5] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    write_argTypes[6] = 0;
    ret = rpcRegister((char *)"a2write", write_argTypes, A2::watdfs_write);
    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int truncate_argTypes[4];
    truncate_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    truncate_argTypes[1] = (1 << ARG_INPUT) | (ARG_LONG << 16);
    truncate_argTypes[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    truncate_argTypes[3] = 0;
    ret = rpcRegister((char *)"a2truncate", truncate_argTypes, A2::watdfs_truncate);
    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int fsync_argTypes[4];
    fsync_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    fsync_argTypes[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    fsync_argTypes[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    fsync_argTypes[3] = 0;
    ret = rpcRegister((char *)"a2fsync", fsync_argTypes, A2::watdfs_fsync);
    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int utimens_argTypes[4];
    utimens_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    utimens_argTypes[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    utimens_argTypes[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    utimens_argTypes[3] = 0;
    ret = rpcRegister((char *)"a2utimens", utimens_argTypes, A2::watdfs_utimens);
    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int rw_lock_argTypes[4];
    rw_lock_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    rw_lock_argTypes[1] = (1 << ARG_INPUT) | ( ARG_INT << 16);
    rw_lock_argTypes[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    rw_lock_argTypes[3] = 0;
    ret = rpcRegister((char *)"lock", rw_lock_argTypes, lock);
    if (ret < 0)
    {
      return ret;
    }
  }

  {
    int rw_unlock_argTypes[4];
    rw_unlock_argTypes[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 1;
    rw_unlock_argTypes[1] = (1 << ARG_INPUT) | ( ARG_INT << 16);
    rw_unlock_argTypes[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    rw_unlock_argTypes[3] = 0;
    ret = rpcRegister((char *)"unlock", rw_unlock_argTypes, unlock);
    if (ret < 0)
    {
      return ret;
    }
  }


  // Hand over control to the RPC library by calling rpcExecute. You should call
  // rpcExecute here:
  ret = rpcExecute();
  // rpcExecute could fail so you may want to have debug-printing here, and then
  // you should return.
  return ret;
}
