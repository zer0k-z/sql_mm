#include "sqlite_database.h"
#include "sqlite_query.h"
#include "sql_mm.h"
#include "tier0/dbg.h"

#include "operations/sqlite_connectop.h"
#include "operations/sqlite_queryop.h"
#include "operations/sqlite_transactop.h"

extern std::vector<SqConnection *> g_vecSqliteConnections;

SqConnection::SqConnection(const SQLiteConnectionInfo info)
{
    this->info.database = new char[strlen(info.database) + 1];
    strncopy(this->info.database, info.database, strlen(info.database) + 1);
}

SqConnection::~SqConnection()
{
    ConMsg("Destroying SQLite connection %s\n", info.database);
    if (m_thread)
    {
        {
            std::lock_guard<std::mutex> lock(m_Lock);
            m_Terminate = true;
            m_QueueEvent.notify_all();
        }

        m_thread->join();
        m_thread.reset();
        m_Terminate = false;
    }

    while (!m_ThinkQueue.empty())
    {
        ThreadOperation *op = m_ThinkQueue.front();
        m_ThinkQueue.pop();

        op->CancelThinkPart();
        delete op;
    }

    if (m_pDatabase)
    {
        sqlite3_close(m_pDatabase);
    }
    delete this->info.database;
}

void SqConnection::Connect(ConnectCallbackFunc callback)
{
    TSQLiteConnectOp *op = new TSQLiteConnectOp(this, callback);

    AddToThreadQueue(op);
}

void SqConnection::Query(char *query, QueryCallbackFunc callback)
{
    if (!m_pDatabase)
    {
        Warning("Failed querying the database at %s.\n", info.database);
        return;
    }
    TSQLiteQueryOp *op = new TSQLiteQueryOp(this, std::string(query), callback);

    AddToThreadQueue(op);
}

void SqConnection::Query(const char *query, QueryCallbackFunc callback, ...)
{
    va_list args;
    va_start(args, callback);

    va_list vaCopy;
    va_copy(vaCopy, args);
    const int iLen = std::vsnprintf(NULL, 0, query, vaCopy);
    va_end(vaCopy);

    std::vector<char> zc(iLen + 1);
    std::vsnprintf(zc.data(), zc.size(), query, args);
    va_end(args);

    if (!m_pDatabase)
    {
        Warning("Failed querying the database at %s.\n", info.database);
        return;
    }

    TSQLiteQueryOp *op = new TSQLiteQueryOp(this, std::string(zc.data(), zc.size()), callback);

    AddToThreadQueue(op);
}

void SqConnection::ExecuteTransaction(Transaction txn, TransactionSuccessCallbackFunc success, TransactionFailureCallbackFunc failure)
{
    TSQLiteTransactOp *op = new TSQLiteTransactOp(this, txn, success, failure);
    AddToThreadQueue(op);
}

void SqConnection::Destroy()
{
    g_vecSqliteConnections.erase(std::remove(g_vecSqliteConnections.begin(), g_vecSqliteConnections.end(), this), g_vecSqliteConnections.end());
    delete this;
}

void SqConnection::RunFrame()
{
    if (!m_ThinkQueue.size())
    {
        return;
    }

    ThreadOperation *op;
    {
        std::lock_guard<std::mutex> lock(m_ThinkLock);
        op = m_ThinkQueue.front();
        m_ThinkQueue.pop();
    }

    op->RunThinkPart();
    delete op;
}

std::string SqConnection::Escape(char *string)
{
    std::string result(strlen(string) * 2 + 1, ' ');
    sqlite3_snprintf(result.size(), result.data(), "%q", string);
    return result;
}

std::string SqConnection::Escape(const char *string)
{
    return Escape(const_cast<char *>(string));
}

SqQuery *SqConnection::PrepareQuery(const char *query, char *error, size_t maxlength, int *errCode)
{
    sqlite3_stmt *stmt = NULL;
    if ((m_LastErrorCode = sqlite3_prepare_v2(m_pDatabase, query, -1, &stmt, NULL)) != SQLITE_OK || !stmt)
    {
        const char *msg;
        if (m_LastErrorCode != SQLITE_OK)
        {
            msg = sqlite3_errmsg(m_pDatabase);
        }
        else
        {
            msg = "Invalid query string";
            m_LastErrorCode = SQLITE_ERROR;
        }
        if (error)
        {
            strncopy(error, msg, maxlength);
        }
        m_LastError.assign(msg);
        return NULL;
    }
    return new SqQuery(this, stmt);
}

void SqConnection::ThreadRun()
{
    std::unique_lock<std::mutex> lock(m_Lock);

    while (true)
    {
        if (m_threadQueue.empty())
        {
            if (m_Terminate)
            {
                return;
            }

            m_QueueEvent.wait(lock);
            continue;
        }

        ThreadOperation *op = m_threadQueue.front();
        m_threadQueue.pop();

        lock.unlock();
        op->RunThreadPart();

        {
            std::lock_guard<std::mutex> think_lock(m_ThinkLock);
            m_ThinkQueue.push(op);
        }

        lock.lock();
    }
}

void SqConnection::AddToThreadQueue(ThreadOperation *threadOperation)
{
    if (!m_thread)
    {
        m_thread = std::unique_ptr<std::thread>(new std::thread(&SqConnection::ThreadRun, this));
    }

    {
        std::lock_guard<std::mutex> lock(m_Lock);
        m_threadQueue.push(threadOperation);
        m_QueueEvent.notify_one();
    }
}
