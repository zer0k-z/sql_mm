#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>
#include "public/sqlite_mm.h"
#include "vendor/sqlite-source/sqlite3.h"
#include "operations/sqlite_queryop.h"
#include "common/database.h"
class SqQuery;

class SqConnection : public ISQLConnection
{
public:
    SqConnection(const SQLiteConnectionInfo info);
    ~SqConnection();
    void Connect(ConnectCallbackFunc callback);
    void Query(char *query, QueryCallbackFunc callback);
    void Query(const char *query, QueryCallbackFunc callback, ...);
    void ExecuteTransaction(Transaction *txn, TransactionSuccessCallbackFunc success, TransactionFailureCallbackFunc failure);
    void Destroy();
    void RunFrame();
    std::string Escape(char *string);
    std::string Escape(const char *string);

    void SetDatabase(sqlite3 *db)
    {
        m_pDatabase = db;
    }

    SqQuery *PrepareQuery(const char *query, char *error, size_t maxlength, int *errCode = NULL);

    sqlite3 *GetDatabase()
    {
        return m_pDatabase;
    }

    SQLiteConnectionInfo info;

private:
    void ThreadRun();
    void AddToThreadQueue(ThreadOperation *threadOperation);

    std::queue<ThreadOperation *> m_threadQueue;
    std::queue<ThreadOperation *> m_ThinkQueue;
    std::unique_ptr<std::thread> m_thread;
    std::condition_variable m_QueueEvent;
    std::mutex m_Lock;
    std::mutex m_ThinkLock;
    bool m_Terminate = false;

    sqlite3 *m_pDatabase;
    std::string m_LastError;
    int m_LastErrorCode;
};

inline bool IsPathSepChar(char c)
{
#ifdef WIN32
    return (c == '\\' || c == '/');
#else
    return (c == '/');
#endif
}
