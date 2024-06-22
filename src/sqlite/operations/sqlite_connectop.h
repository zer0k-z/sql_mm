#pragma once
#include "../sqlite_database.h"

class TSQLiteConnectOp : public ThreadOperation
{
public:
    TSQLiteConnectOp(SqConnection *con, ConnectCallbackFunc func) : m_pCon(con), m_callback(func) {}

    void RunThreadPart();
    void CancelThinkPart();
    void RunThinkPart();

private:
    SqConnection *m_pCon;
    ConnectCallbackFunc m_callback;
    sqlite3 *m_pDatabase = nullptr;
    char m_szError[255] {};
};
