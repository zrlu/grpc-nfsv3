#include "Logger.h"
#include <unistd.h>

using namespace std;

Logger::Logger(const char *fn):
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

void Logger::set_log(const std::string &key, const std::string &value)
{
    leveldb::Status s = m_db->Put(leveldb::WriteOptions(), key, value);
}

bool Logger::get_log(const std::string &key, std::string *value)
{
    leveldb::Status s = m_db->Get(leveldb::ReadOptions(), key, value);
    return s.ok();
}

void Logger::remove_log(const std::string &key)
{
    m_db->Delete(leveldb::WriteOptions(), key);
}

bool Logger::destroy_logs()
{
    int retval = rmdir(m_log_name);
    return retval == 0;
}

Logger::~Logger()
{
    delete m_db;
}