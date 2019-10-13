#ifndef __RPCMANAGER_H__
#define __RPCMANAGER_H__

#include <map>
#include <google/protobuf/message.h>

using namespace google::protobuf;

typedef unsigned long rpcid_t;

class RPCManager
{
    std::map<rpcid_t, Message*> m_rpc_store;
    unsigned int m_rpc_count;
public:
    RPCManager();

    rpcid_t generate_rpc_id();

    Message* get_rpc(rpcid_t rpcid);

    void set_rpc(rpcid_t rpcid, Message *msg);

    void delete_rpc(rpcid_t rpcid);

    void dump();

    ~RPCManager();
};

#endif
