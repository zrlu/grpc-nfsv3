#!/bin/bash

pkill -9 nfsmount
gdb --args nfsmount client_cache -o auto_unmount -f
