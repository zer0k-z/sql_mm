#pragma once
#include "sql_mm.h"

struct SQLiteConnectionInfo
{
	// Absolute file path.
	const char *database;
};

class ISQLiteClient
{
public:
	virtual ISQLConnection *CreateSQLiteConnection(SQLiteConnectionInfo info) = 0;
};