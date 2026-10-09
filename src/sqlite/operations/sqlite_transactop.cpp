#include "sqlite_transactop.h"
#include "tier0/dbg.h"
#include "../sqlite_query.h"
#include "sql_mm.h"

TSQLiteTransactOp::~TSQLiteTransactOp()
{
    FreeQueries();
}

void TSQLiteTransactOp::FreeQueries()
{
    for (auto query : m_pQueries)
    {
        delete static_cast<SqQuery *>(query);
    }
    m_pQueries.clear();
}

void TSQLiteTransactOp::RunThreadPart()
{
    if (!this->DoSimpleQuery("BEGIN"))
    {
        return;
    }
    for (size_t i = 0; i < this->m_txn.queries.size(); i++)
    {
        SqQuery *result = DoQuery(this->m_txn.queries[i].c_str());
        if (!result)
        {
            this->DoSimpleQuery("ROLLBACK");
            m_failIndex = (int)i;
            return;
        }
        this->m_pQueries.push_back(result);
    }
    if (!this->DoSimpleQuery("COMMIT"))
    {
        this->DoSimpleQuery("ROLLBACK");
        return;
    }
}

void TSQLiteTransactOp::CancelThinkPart()
{
    FreeQueries();
    sqlite3_close_v2(m_pCon->GetDatabase());
    m_pCon->SetDatabase(nullptr);
}

void TSQLiteTransactOp::RunThinkPart()
{
    if (!m_szError[0])
    {
        m_successCallback(m_pQueries);
        return;
    }
    META_CONPRINTF("TSQLiteTransactOp ERROR: %s\n", m_szError);
    m_failureCallback(m_szError, m_failIndex);
}

bool TSQLiteTransactOp::DoSimpleQuery(const char *query)
{
    auto pDatabase = m_pCon->GetDatabase();
    auto pQuery = m_pCon->PrepareQuery(query, m_szError, sizeof(m_szError), NULL);
    if (!pQuery)
    {
        return false;
    }
    if (!pQuery->Execute())
    {
        strncopy(m_szError, pQuery->GetError(), sizeof(m_szError));
        pQuery->Destroy();
        return false;
    }
    pQuery->Destroy();
    return true;
}

SqQuery *TSQLiteTransactOp::DoQuery(const char *query)
{
    auto pDatabase = m_pCon->GetDatabase();
    auto pQuery = m_pCon->PrepareQuery(query, m_szError, sizeof(m_szError), NULL);
    if (!pQuery)
    {
        return nullptr;
    }
    if (!pQuery->Execute())
    {
        strncopy(m_szError, pQuery->GetError(), sizeof(m_szError));
        pQuery->Destroy();
        return nullptr;
    }
    return pQuery;
}
