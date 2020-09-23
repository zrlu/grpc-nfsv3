# An implementation of NFSv3 using gRPC

Prerequisites: gRPC

## Build

```bash
make
```

## Run

```bash
# In one terminal, run server
./run server

# In another terminal, run client
./run client

# In one terminal, go to the client_cache folder
echo "HELLO" > hello.txt

# In another terminal, go to the server_cache folder
cat hello.txt
```
