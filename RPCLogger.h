#ifndef __RPC_LOGGER_H__
#define __RPC_LOGGER_H__

#include <fstream>
#include <set>

typedef unsigned long rpcid_t;

class RPCLogger
{
    std::set<rpcid_t> m_set;
public:

    RPCLogger();

    ~RPCLogger();
};

#endif