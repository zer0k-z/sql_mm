#include "sqlite_queryop.h"
#include "tier0/dbg.h"
#include "../sqlite_query.h"
#include "sql_mm.h"

TSQLiteQueryOp::~TSQLiteQueryOp()
{
    delete m_pQuery;
}

void TSQLiteQueryOp::RunThreadPart()
{
    auto pDatabase = m_pCon->GetDatabase();
    m_pQuery = m_pCon->PrepareQuery(m_szQuery.c_str(), m_szError, sizeof(m_szError), NULL);
    if (!m_pQuery)
    {
        return;
    }
    if (!m_pQuery->Execute())
    {
        strncopy(m_szError, m_pQuery->GetError(), sizeof(m_szError));
        m_pQuery->Destroy();
    }
}

void TSQLiteQueryOp::CancelThinkPart()
{
    sqlite3_close(m_pCon->GetDatabase());
    m_pCon->SetDatabase(nullptr);
}

void TSQLiteQueryOp::RunThinkPart()
{
    if (m_szError[0])
    {
        META_CONPRINTF("TSQLiteQueryOp ERROR: %s\n", m_szError);
        return;
    }
    m_callback(m_pQuery);
}
