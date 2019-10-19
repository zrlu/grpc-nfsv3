#
# Copyright 2015 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ver = release

HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)
CXX = g++-9
CXXFLAGS = `pkg-config --cflags protobuf grpc`
CXXFLAGS += `pkg-config fuse --cflags --libs`
CXXFLAGS += -std=c++17
CXXFLAGS += -Wunused-variable

ifeq ($(ver), debug)
CXXFLAGS += -DENABLE_NFS_DEBUG
CXXFLAGS += -DCLIENT_ENABLE_DEBUG_MESSAGE
CXXFLAGS += -g
CXXFLAGS += -Og
else
CXXFLAGS += -O3
endif

LDFLAGS = `pkg-config fuse --libs`
LDFLAGS += -I./third_party/leveldb/include
LDFLAGS += -L./third_party/leveldb/build
LDFLAGS += -lleveldb
LDFLAGS += -Wl,-rpath,./shared

ifeq ($(SYSTEM),Darwin)
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
					 -pthread\
           -lgrpc++_reflection\
           -ldl
else
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl
endif
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = ./protos

vpath %.proto $(PROTOS_PATH)

EXECUTABLES = runserver nfsmount

ifeq ($(test), 1)
TESTS = scratch
endif

all: system-check $(EXECUTABLES) $(TESTS)

debug: all
	$(CXX) $(DEBUG_CXXFLAGS) $^ $(LDFLAGS) -o $@

UserData.o: UserData.cc UserData.h
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c

FileHandlerTable.o: FileHandlerTable.cc FileHandlerTable.h
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c

RPCLogger.o: RPCLogger.cc RPCLogger.h
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c

Logger.o: Logger.cc Logger.h
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c

RPCManager.o: RPCManager.cc RPCManager.h
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c

NFSClient.o: NFSClient.cc NFSClient.h helpers.h RPCManager.o
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c

NFSServer.o: NFSServer.cc NFSServer.h helpers.h
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c

runserver: nfs.pb.o nfs.grpc.pb.o NFSServer.o RPCManager.o RPCLogger.o Logger.o runserver.cc
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

nfsmount: NFSClient.o nfs.pb.o nfs.grpc.pb.o RPCManager.o FileHandlerTable.o UserData.o nfsmount.cc
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

scratch: scratch.cc RPCLogger.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

stattest: NFSClient.o stattest.cc nfs.pb.o nfs.grpc.pb.o

readtest: NFSClient.o readtest.cc nfs.pb.o nfs.grpc.pb.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

nfs.grpc.pb.cc: nfs.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

nfs.pb.cc: nfs.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

.PHONY: clean

clean:
	rm -f *.o *.pb.cc *.pb.h *.h.gch $(EXECUTABLES)


# The following is to test your system and ensure a smoother experience.
# They are by no means necessary to actually compile a grpc-enabled software.

PROTOC_CMD = which $(PROTOC)
PROTOC_CHECK_CMD = $(PROTOC) --version | grep -q libprotoc.3
PLUGIN_CHECK_CMD = which $(GRPC_CPP_PLUGIN)
HAS_PROTOC = $(shell $(PROTOC_CMD) > /dev/null && echo true || echo false)
ifeq ($(HAS_PROTOC),true)
HAS_VALID_PROTOC = $(shell $(PROTOC_CHECK_CMD) 2> /dev/null && echo true || echo false)
endif
HAS_PLUGIN = $(shell $(PLUGIN_CHECK_CMD) > /dev/null && echo true || echo false)

SYSTEM_OK = false
ifeq ($(HAS_VALID_PROTOC),true)
ifeq ($(HAS_PLUGIN),true)
SYSTEM_OK = true
endif
endif

system-check:
ifneq ($(HAS_VALID_PROTOC),true)
	@echo " DEPENDENCY ERROR"
	@echo
	@echo "You don't have protoc 3.0.0 installed in your path."
	@echo "Please install Google protocol buffers 3.0.0 and its compiler."
	@echo "You can find it here:"
	@echo
	@echo "   https://github.com/google/protobuf/releases/tag/v3.0.0"
	@echo
	@echo "Here is what I get when trying to evaluate your version of protoc:"
	@echo
	-$(PROTOC) --version
	@echo
	@echo
endif
ifneq ($(HAS_PLUGIN),true)
	@echo " DEPENDENCY ERROR"
	@echo
	@echo "You don't have the grpc c++ protobuf plugin installed in your path."
	@echo "Please install grpc. You can find it here:"
	@echo
	@echo "   https://github.com/grpc/grpc"
	@echo
	@echo "Here is what I get when trying to detect if you have the plugin:"
	@echo
	-which $(GRPC_CPP_PLUGIN)
	@echo
	@echo
endif
ifneq ($(SYSTEM_OK),true)
	@false
endif
