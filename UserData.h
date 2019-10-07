#pragma once

#include "FileHandlerTable.h"
#include "NFSClient.h"

class UserData {
  NFSClient *m_client;
  FileHandlerTable *m_fhtable;
public:
  explicit UserData(NFSClient *cli);
  NFSClient *client();
  FileHandlerTable *fhtable();
  ~UserData();
};
