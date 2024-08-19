#include <stdio.h>

#ifdef WIN32
#include <WinSock2.h>
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

#include "sql_mm.h"
#include "iserver.h"

#include "common/interface.h"

#include "mysql/mysql_database.h"
#include "mysql/mysql_client.h"

#include "sqlite/sqlite_database.h"
#include "sqlite/sqlite_client.h"
#include "utils/module.h"

#ifdef WIN32
#define GAMEBIN "/csgo/bin/win64/"
#else
#define GAMEBIN "/csgo/bin/linuxsteamrt64/"
#endif

SH_DECL_HOOK1_void(IGameSystem, ServerGamePostSimulate, SH_NOATTRIB, 0, const EventServerGamePostSimulate_t *);

SQLPlugin g_SQLPlugin;
IServerGameDLL *server = nullptr;
IVEngineServer *engine = nullptr;
CModule *serverModule = nullptr;
ISQLInterface *g_sqlInterface = nullptr;

int serverGamePostSimulateHook;

// MySQL
IMySQLClient *g_mysqlClient = nullptr;
std::vector<MySQLConnection *> g_vecMysqlConnections;

// SQLite
ISQLiteClient *g_sqliteClient = nullptr;
std::vector<SqConnection *> g_vecSqliteConnections;

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars *GetGameGlobals()
{
    INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

    if (!server)
    {
        return nullptr;
    }

    return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

PLUGIN_EXPOSE(SQLPlugin, g_SQLPlugin);

bool SQLPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();

    GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
    GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
    GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);

    // Required to get the IMetamodListener events
    g_SMAPI->AddListener(this, this);

    META_CONPRINTF("Starting plugin.\n");

    if (mysql_library_init(0, NULL, NULL))
    {
        snprintf(error, maxlen, "Failed to initialize mysql library\n");
        return false;
    }
    serverModule = new CModule(GAMEBIN, "server");
    g_mysqlClient = new CMySQLClient();
    g_sqliteClient = new SqClient();
    g_sqlInterface = new SQLInterface();
    // clang-format off
    serverGamePostSimulateHook = SH_ADD_DVPHOOK(IGameSystem,
        ServerGamePostSimulate, 
        (IGameSystem *)serverModule->FindVirtualTable("CEntityDebugGameSystem"),
        SH_MEMBER(this, &SQLPlugin::Hook_ServerGamePostSimulate), 
        true);
    // clang-format on
    return true;
}

bool SQLPlugin::Unload(char *error, size_t maxlen)
{
    mysql_library_end();

    delete serverModule;
    delete g_mysqlClient;
    delete g_sqliteClient;
    delete g_sqlInterface;
    return true;
}

void *SQLPlugin::OnMetamodQuery(const char *iface, int *ret)
{
    if (!strcmp(iface, SQLMM_INTERFACE))
    {
        *ret = META_IFACE_OK;
        return g_sqlInterface;
    }

    *ret = META_IFACE_FAILED;
    return nullptr;
}

void SQLPlugin::Hook_ServerGamePostSimulate(const EventServerGamePostSimulate_t *)
{
    for (auto connection : g_vecMysqlConnections)
    {
        connection->RunFrame();
    }
    for (auto connection : g_vecSqliteConnections)
    {
        connection->RunFrame();
    }
}

const char *SQLPlugin::GetLicense()
{
    return "GPLv3";
}

const char *SQLPlugin::GetVersion()
{
    return "1.3.3.0";
}

const char *SQLPlugin::GetDate()
{
    return __DATE__;
}

const char *SQLPlugin::GetLogTag()
{
    return "SQLMM";
}

const char *SQLPlugin::GetAuthor()
{
    return "Poggu, zer0.k";
}

const char *SQLPlugin::GetDescription()
{
    return "Exposes SQL connectivity";
}

const char *SQLPlugin::GetName()
{
    return "SQLMM";
}

const char *SQLPlugin::GetURL()
{
    return "https://poggu.me";
}

IMySQLClient *SQLInterface::GetMySQLClient()
{
    return g_mysqlClient;
}

ISQLiteClient *SQLInterface::GetSQLiteClient()
{
    return g_sqliteClient;
}

size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    size_t len = vsnprintf(buffer, maxlength, fmt, ap);
    va_end(ap);

    if (len >= maxlength)
    {
        buffer[maxlength - 1] = '\0';
        return (maxlength - 1);
    }
    else
    {
        return len;
    }
}

unsigned int strncopy(char *dest, const char *src, size_t count)
{
    if (!count)
    {
        return 0;
    }

    char *start = dest;
    while ((*src) && (--count))
    {
        *dest++ = *src++;
    }
    *dest = '\0';

    return (dest - start);
}
