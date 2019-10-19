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
    const char *m_log_name;
    leveldb::DB *m_db;
public:

    explicit RPCLogger(const char *);

    void set_log(rpcid_t rpcid, const std::string &value);

    void remove_log(rpcid_t rpcid);

    bool get_log(rpcid_t rpcid, std::string *value);

    std::list<rpcid_t> list_logs();

    bool destroy_logs();

    ~RPCLogger();
};

#endif