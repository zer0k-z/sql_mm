#pragma once
#include "public/sql_mm.h"
class SQLInterface : public ISQLInterface
{
	IMySQLClient *GetMySQLClient();
	ISQLiteClient *GetSQLiteClient();
};