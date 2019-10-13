#include <time.h>
#include "RPCManager.h"
#include <google/protobuf/message.h>

using namespace google::protobuf;

RPCManager::RPCManager():
m_rpc_count(0)
{

}

rpcid_t RPCManager::generate_rpc_id()
{
    time_t t = time(0);
    unsigned long rpc_id = (unsigned long)t;
    rpc_id <<= 32;
    rpc_id |= (unsigned long)m_rpc_count;
    ++m_rpc_count;
    return rpc_id;
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

