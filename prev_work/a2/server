#!/bin/bash

PERSIST_DIR=/tmp/zrlu/server

mkdir -p $PERSIST_DIR

stdbuf -i0 -o0 -e0 ./watdfs_server $PERSIST_DIR | tee server_info