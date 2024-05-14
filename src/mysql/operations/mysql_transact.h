#pragma once
#include "../mysql_database.h"
#include "../mysql_result.h"

class TMySQLTransactOp : public ThreadOperation
{
public:
    TMySQLTransactOp(MySQLConnection *con, Transaction txn, TransactionSuccessCallbackFunc success, TransactionFailureCallbackFunc failure)
        : m_pCon(con), m_txn(txn), m_successCallback(success), m_failureCallback(failure)
    {
    }

    ~TMySQLTransactOp();

    void RunThreadPart();
    void CancelThinkPart();
    void RunThinkPart();

private:
    MySQLConnection *m_pCon;
    Transaction m_txn;
    TransactionSuccessCallbackFunc m_successCallback;
    TransactionFailureCallbackFunc m_failureCallback;
    std::vector<ISQLQuery *> m_pQueries;
    char m_szError[255] {};
    int m_failIndex;

    bool DoSimpleQuery(const char *query);
    CMySQLQuery *DoQuery(const char *query);
};
