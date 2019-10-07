#pragma once

#include <bitset>

#define MAX_NUM_FILE_HANDLERS 1024

class FileHandlerTable
{
  std::bitset<MAX_NUM_FILE_HANDLERS> m_table;
  int m_idx;
public:
  FileHandlerTable();
  int allocate();
  void deallocate(int fh);
};