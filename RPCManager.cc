#include <time.h>
#include "RPCManager.h"
#include <google/protobuf/message.h>
#include <iomanip>

using namespace google::protobuf;

RPCManager::RPCManager():
m_rpc_count(0)
{

}

rpcid_t RPCManager::generate_rpc_id(short client_id)
{
    /*
     | client_id = 1 Byte | LSB of timestamp = 3 Byte | RPC count = 8 Byte |
    */
    
    time_t t = time(0);
    unsigned long rpc_id = (unsigned long)t;
    rpc_id &= 0xffffffffUL;
    rpc_id <<= 32;
    rpc_id |= (unsigned long)m_rpc_count;

    ++m_rpc_count;
    return std::to_string(client_id) + ":" + std::to_string(rpc_id);
}

bool RPCManager::has_rpc(rpcid_t rpcid)
{
    return m_rpc_store.find(rpcid) != m_rpc_store.end();
}

Message *RPCManager::get_rpc(rpcid_t rpcid)
{
    return m_rpc_store[rpcid];
}

void RPCManager::set_rpc(rpcid_t rpcid, Message *msg)
{
    m_rpc_store[rpcid] = msg;
}

void RPCManager::dump()
{
    std::cerr << "RPCManager::dump():" << std::endl;
    for (auto it = m_rpc_store.begin(); it != m_rpc_store.end(); ++it)
    {
        std::cerr << it->first << ": " << it->second->ShortDebugString() << std::endl;
    }
}

void RPCManager::delete_rpc(rpcid_t rpcid)
{
    Message *msg = get_rpc(rpcid);
    m_rpc_store.erase(rpcid);
    delete msg;
}

RPCManager::~RPCManager()
{

}

