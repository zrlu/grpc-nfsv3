# Starter code for CS 454/654 Assignments 2 and 3

This `README` contains a short description of the provided starter code for CS 454/654 Assignments 2 and 3. Each file contains more detailed comments.

## Makefile
The provided `Makefile` compiles the starter code, if `make` creates run `libwatdfs.a` and `watdfs_server`. `make all` creates `libwatdfs.a, watdfs_client, watdfs_server`.

## RPC Components

`librpc.a` provides the implementation of the RPC library as described in Section 10 of the Assignment specification.

`rpc.h` defines the interface of the RPC library, including the functions such as `rpcRegister` and `rpcCall`. `rpc.h` also contains definitions for the argument types, and error codes. You should not modify `rpc.h`

## WatDFS Components

`libwatdfsmain.a` provides the implementation of the `main` function for the WatDFS Client.

`watdfs_client.h` contains the interface of the WatDFS Client that you must implement. Each of the functions defined in `watdfs_client.h` are described in Section 3 of the Assignment specification. You should not modify `watdfs_client.h`.

`watdfs_client.c` contains the functions needed to implement the WatDFS Client, as defined by `watdfs_client.h`. `watdfs_client.c` contains partial implementation of `watdfs_cli_getattr`. You should modify `watdfs_client.c` as part of Assignments 2 and 3.

`watdfs_server.c` contains starter code to implement the WatDFS Server. Specifically, it contains a partial implementation of the server implementation of the `getattr` RPC call. You should modify `watdfs_server.c` as part of Assignments 2 and 3.

## RW Lock (Assignment 3)

`rw_lock.c` and `rw_lock.h` contain the definition and implementation of a Reader-Writer lock, as described in Section 13.5 of the Assignment specification, which may be helpful when completing Assignment 3. You should not need to modify `rw_lock.c` and `rw_lock.h`.

