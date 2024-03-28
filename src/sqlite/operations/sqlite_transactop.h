#pragma once
#include "../sqlite_database.h"
#include "../sqlite_result.h"
#include "common/database.h"
class SqConnection;
class SqResults;
class SqQuery;

class TSQLiteTransactOp : public ThreadOperation
{
public:
    TSQLiteTransactOp(SqConnection *con, Transaction txn, TransactionSuccessCallbackFunc success, TransactionFailureCallbackFunc failure)
        : m_pCon(con), m_txn(txn), m_successCallback(success), m_failureCallback(failure)
    {
    }

    ~TSQLiteTransactOp();

    void RunThreadPart();
    void CancelThinkPart();
    void RunThinkPart();

private:
    SqConnection *m_pCon;
    Transaction m_txn;
    char m_szError[256];
    int m_failIndex;
    TransactionSuccessCallbackFunc m_successCallback;
    TransactionFailureCallbackFunc m_failureCallback;
    std::vector<ISQLQuery *> m_pQueries;

    bool DoSimpleQuery(const char *query);
    SqQuery *DoQuery(const char *query);
};
