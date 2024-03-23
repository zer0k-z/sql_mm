#include "sqlite_queryop.h"
#include "tier0/dbg.h"
#include "../sqlite_query.h"

TSQLiteQueryOp::~TSQLiteQueryOp()
{
    delete m_pQuery;
}

void TSQLiteQueryOp::RunThreadPart()
{
    auto pDatabase = m_pCon->GetDatabase();
    m_pQuery = m_pCon->PrepareQuery(m_szQuery.c_str(), NULL, 0, NULL);
    if (!m_pQuery)
    {
        return;
    }
    if (!m_pQuery->Execute())
    {
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
    if (m_pQuery->GetError()[0])
    {
        ConMsg("%s\n", m_pQuery->GetError());
        return;
    }
    m_callback(m_pQuery);
}
