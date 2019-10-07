#include "FileHandlerTable.h"

FileHandlerTable::FileHandlerTable() :
    m_table(std::bitset<MAX_NUM_FILE_HANDLERS>()),
    m_idx(3)
{
    m_table[0] = 1;
    m_table[1] = 1;
    m_table[2] = 1;
}

int FileHandlerTable::allocate()
{
    int start = m_idx;
    while (1)
    {
        if (m_table[m_idx] == 0)
        {
            m_table[m_idx] = 1;
            return m_idx;
        }
        m_idx = (m_idx + 1) % MAX_NUM_FILE_HANDLERS;
        if (m_idx == start)
        {
            return -1;
        }
    }
}

void FileHandlerTable::deallocate(int fh)
{
    m_table[fh] = 0;
}