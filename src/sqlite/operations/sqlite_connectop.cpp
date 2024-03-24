#include "sqlite_connectop.h"
#include "sql_mm.h"

int busy_handler(void *unused1, int unused2)
{
#if defined PLATFORM_WINDOWS
    Sleep(100);
#elif defined PLATFORM_POSIX
    usleep(100000);
#endif
    return 1;
}

void TSQLiteConnectOp::RunThreadPart()
{
    m_szError[0] = '\0';

    /* Try to open a new connection */
    sqlite3 *sql;
    int err = sqlite3_open(m_pCon->info.database, &sql);
    if (err != SQLITE_OK)
    {
        strncopy(m_szError, sqlite3_errmsg(sql), sizeof(m_szError));
        sqlite3_close(sql);
        return;
    }

    sqlite3_busy_handler(sql, busy_handler, NULL);

    m_pDatabase = sql;
}

void TSQLiteConnectOp::CancelThinkPart()
{
    sqlite3_close(m_pDatabase);
    m_pCon->SetDatabase(nullptr);
}

void TSQLiteConnectOp::RunThinkPart()
{
    if (m_szError[0])
    {
        META_CONPRINTF("Failed to establish a SQLite connection: %s\n", m_szError);
    }

    m_pCon->SetDatabase(m_pDatabase);
    m_callback(m_pDatabase != nullptr);
}
