#pragma once
#include "../sqlite_database.h"
#include "../sqlite_result.h"
#include "common/database.h"
class SqConnection;
class SqResults;
class SqQuery;

class TSQLiteQueryOp : public ThreadOperation
{
public:
    TSQLiteQueryOp(SqConnection *con, std::string query, QueryCallbackFunc func) : m_pCon(con), m_szQuery(query), m_callback(func) {}

    ~TSQLiteQueryOp();

    void RunThreadPart();
    void CancelThinkPart();
    void RunThinkPart();

private:
    SqConnection *m_pCon;
    std::string m_szQuery;
    QueryCallbackFunc m_callback;
    SqResults *m_res = nullptr;
    SqQuery *m_pQuery = nullptr;
};
