#include "sqlite_client.h"
#include "sqlite_database.h"

extern std::vector<SqConnection *> g_vecSqliteConnections;

ISQLConnection *SqClient::CreateSQLiteConnection(SQLiteConnectionInfo info)
{
	auto connection = new SqConnection(info);
	g_vecSqliteConnections.push_back(connection);

	return connection;
}
