#include "sqlite_query.h"
#include "sqlite_result.h"

SqQuery::SqQuery(SqConnection *conn, sqlite3_stmt *stmt) : m_pCon(conn), m_pStmt(stmt), m_pResults(NULL), m_AffectedRows(0), m_InsertID(0)
{
    m_ParamCount = sqlite3_bind_parameter_count(m_pStmt);
    m_ColCount = sqlite3_column_count(m_pStmt);
}

SqQuery::~SqQuery()
{
    delete m_pResults;
    sqlite3_finalize(m_pStmt);
}

ISQLResult *SqQuery::GetResultSet()
{
    return m_pResults;
}

bool SqQuery::Execute()
{
    int rc;

    if (!m_pResults && m_ColCount)
    {
        m_pResults = new SqResults(this);
    }

    if (m_pResults)
    {
        m_pResults->ResetResultCount();
    }

    while ((rc = sqlite3_step(m_pStmt)) == SQLITE_ROW)
    {
        if (!m_pResults)
        {
            m_pResults = new SqResults(this);
        }
        m_pResults->PushResult();
    }

    sqlite3 *db = m_pCon->GetDatabase();
    if (rc != SQLITE_OK && rc != SQLITE_DONE && rc == sqlite3_errcode(db))
    {
        m_LastErrorCode = rc;
        m_LastError.assign(sqlite3_errmsg(db));
        m_AffectedRows = 0;
        m_InsertID = 0;
    }
    else
    {
        m_LastErrorCode = SQLITE_OK;
        m_AffectedRows = (unsigned int)sqlite3_changes(db);
        m_InsertID = (unsigned int)sqlite3_last_insert_rowid(db);
    }

    sqlite3_reset(m_pStmt);
    sqlite3_clear_bindings(m_pStmt);

    return (m_LastErrorCode == SQLITE_OK);
}

void SqQuery::Destroy()
{
    delete this;
}

const char *SqQuery::GetError()
{
    return m_LastError.c_str();
}

unsigned int SqQuery::GetAffectedRows()
{
    return m_AffectedRows;
}

unsigned int SqQuery::GetInsertId()
{
    return m_InsertID;
}

sqlite3_stmt *SqQuery::GetStmt()
{
    return nullptr;
}
