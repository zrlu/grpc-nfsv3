#include "UserData.h"

UserData::UserData(NFSClient *cli) :
    m_client(cli),
    m_fhtable(new FileHandlerTable)
{
}

NFSClient *UserData::client()
{
    return m_client;
}

FileHandlerTable *UserData::fhtable()
{
    return m_fhtable;
}

UserData::~UserData()
{
    delete m_client;
    delete m_fhtable;
}
