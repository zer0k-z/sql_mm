#pragma once

#include "sqlite_database.h"
#include "public/sqlite_mm.h"
#include "vendor/sqlite-source/sqlite3.h"
class SqResults;
class SqDatabase;

class SqQuery : public ISQLQuery
{
	friend class SqResults;
public:
	SqQuery(SqDatabase *parent, sqlite3_stmt *stmt);
	~SqQuery();
public: //IQuery
	ISQLResult *GetResultSet();
	bool FetchMoreResults();
	void Destroy();
public: //IPreparedQuery
	unsigned int GetAffectedRows();
	unsigned int GetInsertID();
#if 0
public: //IResultSet
	unsigned int GetRowCount();
	unsigned int GetFieldCount();
	const char *FieldNumToName(unsigned int columnId);
	bool FieldNameToNum(const char *name, unsigned int *columnId);
	bool MoreRows();
	IResultRow *FetchRow();
	IResultRow *CurrentRow();
	bool Rewind();
	DBType GetFieldType(unsigned int field);
	DBType GetFieldDataType(unsigned int field);
public: //IResultRow
	DBResult GetString(unsigned int columnId, const char **pString, size_t *length);
	DBResult CopyString(unsigned int columnId, 
		char *buffer, 
		size_t maxlength, 
		size_t *written);
	DBResult GetFloat(unsigned int columnId, float *pFloat);
	DBResult GetInt(unsigned int columnId, int *pInt);
	bool IsNull(unsigned int columnId);
	size_t GetDataSize(unsigned int columnId);
	DBResult GetBlob(unsigned int columnId, const void **pData, size_t *length);
	DBResult CopyBlob(unsigned int columnId, void *buffer, size_t maxlength, size_t *written);
public:
	sqlite3_stmt *GetStmt();
#endif
private:
	SqDatabase* m_pDatabase;
	sqlite3_stmt *m_pStmt;
	SqResults *m_pResults;
	//unsigned int m_ParamCount;
	//String m_LastError;
	//int m_LastErrorCode;
	unsigned int m_AffectedRows;
	unsigned int m_InsertID;
	//unsigned int m_ColCount;
};