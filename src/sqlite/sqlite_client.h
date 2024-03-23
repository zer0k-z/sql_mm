#pragma once
#include "public/sqlite_mm.h"

class SqClient : public ISQLiteClient
{
public:
	ISQLConnection *CreateSQLiteConnection(SQLiteConnectionInfo info);
};