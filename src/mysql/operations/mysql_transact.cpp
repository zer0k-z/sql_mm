#include "mysql_transact.h"
#include "tier0/dbg.h"
#include "sql_mm.h"

TMySQLTransactOp::~TMySQLTransactOp()
{
    for (auto query : m_pQueries)
    {
        delete query;
    }
    m_pQueries.clear();
}

void TMySQLTransactOp::RunThreadPart()
{
    if (!this->DoSimpleQuery("BEGIN"))
    {
        return;
    }
    for (size_t i = 0; i < this->m_txn.GetQueryCount(); i++)
    {
        CMySQLQuery *result = DoQuery(this->m_txn.GetQuery(i).c_str());
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

void TMySQLTransactOp::CancelThinkPart()
{
    mysql_close(m_pCon->GetDatabase());
    m_pCon->SetDatabase(nullptr);
}

void TMySQLTransactOp::RunThinkPart()
{
    if (!m_szError[0])
    {
        m_successCallback(m_pQueries);
        m_pQueries.clear();
        return;
    }
    META_CONPRINTF("TMySQLTransactOp ERROR: %s\n", m_szError);
    m_failureCallback(m_szError, m_failIndex);
}

bool TMySQLTransactOp::DoSimpleQuery(const char *query)
{
    auto pQuery = DoQuery(query);
    if (!pQuery)
    {
        return false;
    }
    delete pQuery;
    return true;
}

CMySQLQuery *TMySQLTransactOp::DoQuery(const char *query)
{
    auto pDatabase = m_pCon->GetDatabase();
    if (mysql_query(pDatabase, query))
    {
        V_snprintf(m_szError, sizeof m_szError, "MySQL query error: %s\n", mysql_error(pDatabase));
        return NULL;
    }

    MYSQL_RES *res = NULL;
    if (mysql_field_count(pDatabase))
    {
        res = mysql_store_result(pDatabase);
        if (!res)
        {
            return NULL;
        }
    }
    return new CMySQLQuery(m_pCon, res);
}
