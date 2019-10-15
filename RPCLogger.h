#ifndef __RPC_LOGGER_H__
#define __RPC_LOGGER_H__

#include <fstream>
#include <set>
#include <iostream>
#include <list>
#include <leveldb/db.h>

#include "RPCManager.h"

class RPCLogger
{
    std::fstream m_fs;
    const char *m_log_name;
    leveldb::DB *m_db;
public:

    explicit RPCLogger(const char *);

    void add_log(rpcid_t rpcid);

    void remove_log(rpcid_t rpcid);

    std::list<rpcid_t> list_logs();

    ~RPCLogger();
};

#endif