#ifndef __RPCMANAGER_H__
#define __RPCMANAGER_H__

#include <map>
#include <string>
#include <google/protobuf/message.h>

using namespace google::protobuf;

#define CLIENT_ID(rpc_id) (rpc_id >> 59)

typedef std::string rpcid_t;

class RPCManager
{
    std::map<rpcid_t, Message*> m_rpc_store;
    unsigned int m_rpc_count;
public:
    RPCManager();

    rpcid_t generate_rpc_id(short client_id);

    Message* get_rpc(rpcid_t rpcid);

    void set_rpc(rpcid_t rpcid, Message *msg);

    void delete_rpc(rpcid_t rpcid);

    void dump();

    ~RPCManager();
};

#endif
