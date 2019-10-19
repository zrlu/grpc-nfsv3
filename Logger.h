#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <fstream>
#include <set>
#include <iostream>
#include <list>
#include <leveldb/db.h>
#include <string>

class Logger
{
    const char *m_log_name;
    leveldb::DB *m_db;
public:

    explicit Logger(const char *);

    void set_log(const std::string &key, const std::string &value);

    void remove_log(const std::string &key);

    bool get_log(const std::string &key, std::string *value);

    bool destroy_logs();

    ~Logger();
};

#endif