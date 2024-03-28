#include "sqlite_connectop.h"
#include "sql_mm.h"
#include "filesystem.h"
#include "bufferstring.h"

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
    g_pFullFileSystem->CreateDirHierarchyForFile(m_pCon->info.database, nullptr);

    /* Get directory path and file name*/
    char directory[MAX_PATH];
    V_ExtractFilePath(m_pCon->info.database, directory, MAX_PATH);
    const char *fileName = V_UnqualifiedFileName(m_pCon->info.database);

    /* Convert directory path to full path */
    CBufferStringGrowable<MAX_PATH> fullPathBuffer;
    const char *path = g_pFullFileSystem->RelativePathToFullPath(directory, nullptr, fullPathBuffer);

    /* Merge the abs path and file name */
    char fullPath[MAX_PATH];
    V_ComposeFileName(fullPathBuffer.Get(), fileName, fullPath, MAX_PATH);

    int err = sqlite3_open(fullPath, &sql);
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
