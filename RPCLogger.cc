#include "RPCLogger.h"
#include <unistd.h>

using namespace std;

RPCLogger::RPCLogger(const char *fn):
m_log_name(fn)
{
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, m_log_name, &m_db);
    if (!status.ok())
    {
        cerr << status.ToString() << endl;
    }
}

void RPCLogger::add_log(rpcid_t rpcid)
{
    std::string value = "1";
    leveldb::Status s = m_db->Put(leveldb::WriteOptions(), rpcid, value);
}

void RPCLogger::remove_log(rpcid_t rpcid)
{
    m_db->Delete(leveldb::WriteOptions(), rpcid);
}

list<rpcid_t> RPCLogger::list_logs()
{
    list<rpcid_t> lst;
    leveldb::Iterator* it = m_db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::cerr << it->key().ToString() << ": " << it->value().ToString() << std::endl;
        lst.push_back(it->key().ToString());
    }
    delete it;

    return lst;
}

RPCLogger::~RPCLogger()
{
    delete m_db;
}