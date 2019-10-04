#!/bin/bash

# New for A3:
export CACHE_INTERVAL_SEC=5 # or any integer value >= 0

MOUNT_DIR=/tmp/zrlu/mount2
CACHE_DIR=/tmp/zrlu/cache2

mkdir -p $MOUNT_DIR
mkdir -p $CACHE_DIR

source server_info

echo "SERVER_ADDRESS=$SERVER_ADDRESS"
echo "SERVER_PORT=$SERVER_PORT"

./watdfs_client -s -f -o direct_io $CACHE_DIR $MOUNT_DIR


