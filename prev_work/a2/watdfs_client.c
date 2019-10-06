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
#define MAX_NUM_FILE_HANDLERS 1024

int fuse_file_info_size = sizeof(struct fuse_file_info);
int timespec_size = sizeof(struct timespec);

class FileHandlerTable
{
  bitset<MAX_NUM_FILE_HANDLERS> table;
  int idx;
public:
  FileHandlerTable():
    table(bitset<MAX_NUM_FILE_HANDLERS>()),
    idx(3)
    {
      table[0] = 1;
      table[1] = 1;
      table[2] = 1;
    }
  int allocate() {
    int start = idx;
    while (1) {
      if (table[idx] == 0) {
        table[idx] = 1;
        return idx;
      }
      idx = (idx + 1) % MAX_NUM_FILE_HANDLERS;
      if (idx == start) {
        return -1;
      }
    }
  }
  void deallocate(int fh) {
    table[fh] = 0;
  }
};

class UserData 
{
public:
  FileHandlerTable *fhtable;
  UserData(): fhtable(new FileHandlerTable()) {}
  ~UserData() 
  {
    delete fhtable;
  }
};

// SETUP AND TEARDOWN
void *watdfs_cli_init(struct fuse_conn_info *conn, const char *path_to_cache,
                    time_t cache_interval) {
  // You should set up the RPC library here, by calling rpcClientInit.
  int ret = rpcClientInit();
  if (ret != 0) {
    // cerr << "rpcClientInit() returns " << ret << endl; 
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
  // std::// cerr << "Failed to initialize RPC Client";
  // #endif


  // You can also initialize any global state that you want to have in this
  // method, and return it. The value that you return here will be passed
  // as userdata in other functions.

  // path_to_cache and cache_interval are not necessary for Assignment 2, but should
  // be used in Assignment 3.
  UserData *ptr = new UserData();
  return (void*)ptr;
}

void watdfs_cli_destroy(void *userdata) {
  // You should clean up your userdata state here.
  delete (UserData*)userdata;
  // You should also tear down the RPC library by calling rpcClientDestroy.
  rpcClientDestroy();
}

// GET FILE ATTRIBUTES
int watdfs_cli_getattr(void *userdata, const char *path, struct stat *statbuf) {

  // SET UP THE RPC CALL

  // getattr has 3 arguments.
  int num_args = 3;

  // Allocate space for the output arguments.
  void **args = (void**) malloc(3 * sizeof(void*));

  // Allocate the space for arg types, and one extra space for the null
  // array element.
  int arg_types[num_args + 1];

  // The path has string length (strlen) + 1 (for the null character).
  int pathlen = strlen(path) + 1;

  // Fill in the arguments
  // The first argument is the path, it is an input only argument, and a char
  // array. The length of the array is the length of the path.
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  // For arrays the argument is the array pointer, not a pointer to a pointer.
  args[0] = (void*)path;

  // The second argument is the stat structure. This argument is an output
  // only argument, and we treat it as a char array. The length of the array
  // is the size of the stat structure, which we can determine with sizeof.
  arg_types[1] = (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | sizeof(struct stat); // statbuf
  args[1] = (void*)statbuf;

  // The third argument is the return code, an output only argument, which is
  // an integer. You should fill in this argument type here:
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);

  // The return code is not an array, so we need to hand args[2] an int*.
  // The int* could be the address of an integer located on the stack, or use
  // a heap allocated integer, in which case it should be freed.
  // You should fill in the argument here:
  void *retcode = (void*)malloc(sizeof(int));
  args[2] = (void*)retcode;

  // Finally, the last position of the arg types is 0. There is no corresponding
  // arg.
  arg_types[3] = 0;

  // MAKE THE RPC CALL
  int rpc_ret = rpcCall((char *)"getattr", arg_types, args);

  // HANDLE THE RETURN

  // The integer value watdfs_cli_getattr will return.
  int fxn_ret = 0;
  if (rpc_ret < 0) {
    // Something went wrong with the rpcCall, return a sensible return value.
    // In this case lets return, -EINVAL
    fxn_ret = -EINVAL;
  } else {
    // Our RPC call succeeded. However, it's possible that the return code
    // from the server is not 0, that is it may be -errno. Therefore, we should
    // set our function return value to the retcode from the server.
    // You should set the function return variable to the return code from the
    // server here:
    fxn_ret = *(int*)retcode;
  }

  if (fxn_ret < 0) {
    // Important: if the return code of watdfs_cli_getattr is negative (an
    // error), then we need to make sure that the stat structure is filled with
    // 0s. Otherwise, FUSE will be confused by the contradicting return values.
    memset(statbuf, 0, sizeof(struct stat));
  }

  // Clean up the memory we have allocated.
  free(args);
  free(retcode);

  // cerr << "watdfs_cli_getattr() returns 0" << endl;

  // Finally return the value we got from the server.
  return fxn_ret;
}

int watdfs_cli_fgetattr(void *userdata, const char *path, struct stat *statbuf,
                      struct fuse_file_info *fi) {
  int num_args = 4;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)statbuf;
  args[2] = (void*)fi;
  args[3] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | sizeof(struct stat);
  arg_types[2] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[3] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[4] = 0;
  rpc_ret = rpcCall((char *)"fgetattr", arg_types, args);
  if (rpc_ret < 0) {
    // cerr << "watdfs_cli_fgetattr rpc_ret " << rpc_ret << endl;
    fxn_ret = -EINVAL;
  } else {
    // cerr << "watdfs_cli_fgetattr rpc_ret: 0" << endl;
    fxn_ret = *(int*)retcode;
  }  
  if (fxn_ret < 0) {
    memset(statbuf, 0, sizeof(struct stat));
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

// CREATE, OPEN AND CLOSE
int watdfs_cli_mknod(void *userdata, const char *path, mode_t mode, dev_t dev) {
  int num_args = 4;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)&mode;
  args[2] = (void*)&dev;
  args[3] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
  arg_types[2] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[3] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[4] = 0;
  rpc_ret = rpcCall((char *)"mknod", arg_types, args);
  if (rpc_ret < 0) {
    // cerr << "watdfs_cli_mknod rpc_ret " << rpc_ret << endl;
    fxn_ret = -EINVAL;
  } else {
    // cerr << "watdfs_cli_mknod rpc_ret: 0" << endl;
    fxn_ret = *(int*)retcode;
  }
  free(args);
  free(retcode);
  // cerr << "watdfs_cli_mknod() returns 0" << endl;
  return fxn_ret;
}

int watdfs_cli_open(void *userdata, const char *path, struct fuse_file_info *fi) {
  // Called during open.
  // You should fill in fi->fh.
  // cerr << "watdfs_cli_open path: " << path << endl;
  UserData *userdataptr = (UserData*)userdata;
  int fh = userdataptr->fhtable->allocate();
  if (fh == -1)
  {
    // cerr << "ENFILE" << endl;
    return -ENFILE;
  }
  fi->fh = fh;
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
  rpc_ret = rpcCall((char *)"open", arg_types, args);
  if (rpc_ret < 0) {
    // cerr << "watdfs_cli_open rpc_ret " << rpc_ret << endl;
    fxn_ret = -EINVAL;
  } else {
    // cerr << "watdfs_cli_open rpc_ret: 0" << endl;
    fxn_ret = *(int*)retcode;
  } 
  if (fxn_ret < 0)
  {
    userdataptr->fhtable->deallocate(fh);
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
  rpc_ret = rpcCall((char *)"release", arg_types, args);
  if (rpc_ret < 0) {
    // cerr << "watdfs_cli_release rpc_ret " << rpc_ret << endl;
    fxn_ret = -EINVAL;
  } else {
    // cerr << "watdfs_cli_release rpc_ret: 0" << endl;
    fxn_ret = *(int*)retcode;
  }  
  free(args);
  free(retcode);
  if (fxn_ret == 0)
  {
    UserData *userdataptr = (UserData*)userdata;
    userdataptr->fhtable->deallocate(fi->fh);
  }
  return fxn_ret;
}

// READ AND WRITE DATA

#define CHUNK_SIZE MAX_ARRAY_LEN

int watdfs_cli_read_chunk(void *userdata, const char *path, char *buf, size_t size,
                  off_t offset, struct fuse_file_info *fi) {
  // Read size amount of data at offset of file into buf.
  int num_args = 6;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)buf;
  args[2] = (void*)&size;
  args[3] = (void*)&offset;
  args[4] = (void*)fi;
  args[5] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_OUTPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | size;
  arg_types[2] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[3] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[4] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[5] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[6] = 0;
  rpc_ret = rpcCall((char *)"read", arg_types, args);
  if (rpc_ret < 0) {
    // cerr << "watdfs_cli_read rpc_ret " << rpc_ret << endl;
    fxn_ret = -EINVAL;
  } else {
    // cerr << "watdfs_cli_read rpc_ret: 0" << endl;
    fxn_ret = *(int*)retcode;
  }  
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_read(void *userdata, const char *path, char *buf, size_t size,
                  off_t offset, struct fuse_file_info *fi) {
  // Read size amount of data at offset of file into buf.
  // Remember that size may be greater then the maximum array size of the RPC
  // library.
  int fxn_ret = 0;
  int q = size / CHUNK_SIZE;
  int r = size % CHUNK_SIZE;
  int size_read = 0;
  // cerr << "will read " << q << "chunks" << endl;
  for (int i = 0; i < q; ++i) {
    fxn_ret = watdfs_cli_read_chunk(userdata, path, &buf[i*CHUNK_SIZE], CHUNK_SIZE, offset + i*CHUNK_SIZE, fi);
    if (fxn_ret < 0)
    {
      return fxn_ret;
    }
    size_read += fxn_ret;
  }
  if (r) {
    fxn_ret = watdfs_cli_read_chunk(userdata, path, &buf[q*CHUNK_SIZE], r, offset + q*CHUNK_SIZE, fi);
    if (fxn_ret < 0)
    {
      return fxn_ret;
    }
    size_read += fxn_ret;
  }
  return size_read;
}
int watdfs_cli_write_chunk(void *userdata, const char *path, const char *buf,
                   size_t size, off_t offset, struct fuse_file_info *fi) {
  // Write size amount of data at offset of file from buf.
  int num_args = 6;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)buf;
  args[2] = (void*)&size;
  args[3] = (void*)&offset;
  args[4] = (void*)fi;
  args[5] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | size;
  arg_types[2] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[3] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[4] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | fuse_file_info_size;
  arg_types[5] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[6] = 0;
  rpc_ret = rpcCall((char *)"write", arg_types, args);
  if (rpc_ret < 0) {
    // cerr << "watdfs_cli_write rpc_ret " << rpc_ret << endl;
    fxn_ret = -EINVAL;
  } else {
    // cerr << "watdfs_cli_write rpc_ret: 0" << endl;
    fxn_ret = *(int*)retcode;
  }  
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_write(void *userdata, const char *path, const char *buf,
                   size_t size, off_t offset, struct fuse_file_info *fi) {
  // Write size amount of data at offset of file from buf.
  // Remember that size may be greater then the maximum array size of the RPC
  // library.
  int fxn_ret = 0;
  int q = size / CHUNK_SIZE;
  // cerr << "will write " << q << " chunks" << endl;
  int r = size % CHUNK_SIZE;
  for (int i = 0; i < q; ++i) {
    // cerr << "chunk " << i << endl;
    fxn_ret = watdfs_cli_write_chunk(userdata, path, &buf[i*CHUNK_SIZE], CHUNK_SIZE, offset + i*CHUNK_SIZE, fi);
    if (fxn_ret < 0)
    {
      // cerr << "loop:" << fxn_ret << endl;
      return fxn_ret;
    }
  }
  if (r) {
    fxn_ret = watdfs_cli_write_chunk(userdata, path, &buf[q*CHUNK_SIZE], r, offset + q*CHUNK_SIZE, fi);
    // cerr << "r:" << fxn_ret << endl;
    if (fxn_ret < 0)
    {
      return fxn_ret;
    }
  }
  return size;
}

int watdfs_cli_truncate(void *userdata, const char *path, off_t newsize) {
  // Change the file size to newsize.
  int num_args = 3;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)&newsize;
  args[2] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (ARG_LONG << 16);
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"truncate", arg_types, args);
  if (rpc_ret < 0) {
    // cerr << "watdfs_cli_truncate rpc_ret " << rpc_ret << endl;
    fxn_ret = -EINVAL;
  } else {
    // cerr << "watdfs_cli_truncate rpc_ret: 0" << endl;
    fxn_ret = *(int*)retcode;
  }
  free(args);
  free(retcode);
  return fxn_ret;
}

int watdfs_cli_fsync(void *userdata, const char *path,
                   struct fuse_file_info *fi) {
  // Force a flush of file data.
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
  rpc_ret = rpcCall((char *)"fsync", arg_types, args);
  if (rpc_ret < 0) {
    // cerr << "watdfs_cli_fsync rpc_ret " << rpc_ret << endl;
    fxn_ret = -EINVAL;
  } else {
    // cerr << "watdfs_cli_fsync rpc_ret: 0" << endl;
    fxn_ret = *(int*)retcode;
  }  
  free(args);
  free(retcode);
  return fxn_ret;
}

// CHANGE METADATA
int watdfs_cli_utimens(void *userdata, const char *path,
                     const struct timespec ts[2]) {
  // Change file access and modification times.
  int num_args = 3;
  void **args = (void**) malloc(num_args * sizeof(void*));
  void *retcode = (void*)malloc(sizeof(int));
  int arg_types[num_args + 1];
  int pathlen = strlen(path) + 1;
  int rpc_ret;
  int fxn_ret = 0;
  args[0] = (void*)path;
  args[1] = (void*)ts;
  args[2] = (void*)retcode;
  arg_types[0] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | pathlen;
  arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_ARRAY) | (ARG_CHAR << 16) | 2*timespec_size;
  arg_types[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  arg_types[3] = 0;
  rpc_ret = rpcCall((char *)"utimens", arg_types, args);
  if (rpc_ret < 0) {
    // cerr << "watdfs_cli_utimens rpc_ret " << rpc_ret << endl;
    fxn_ret = -EINVAL;
  } else {
    // cerr << "watdfs_cli_utimens rpc_ret: 0" << endl;
    fxn_ret = *(int*)retcode;
  }  
  free(args);
  free(retcode);
  return fxn_ret;
}
