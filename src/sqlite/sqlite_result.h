#pragma once

#include "sqlite_database.h"
#include "public/sqlite_mm.h"
#include "common/memtable.h"
#include "common/database.h"
#include "vendor/sqlite-source/sqlite3.h"
class SqQuery;

struct SqField
{
	int type;
	union
	{
		int idx;
		float f;
	} u;
	size_t size;
};

class SqResults : public ISQLResult, ISQLRow
{
	friend class SqQuery;
public:
	SqResults(SqQuery *query);
	~SqResults();
public: //IResultSet
	int GetRowCount();
	int GetFieldCount();
	const char *FieldNumToName(unsigned int columnId);
	bool FieldNameToNum(const char *name, unsigned int *columnId);
	bool MoreRows();
	ISQLRow *FetchRow();
	ISQLRow *CurrentRow();
	bool Rewind();
	int GetFieldType(unsigned int field) { return 0; }
public: //IResultRow
	int GetFieldType(unsigned int field);
	char *GetString(unsigned int columnId, size_t *length = nullptr);
	size_t GetDataSize(unsigned int columnId);
	float GetFloat(unsigned int columnId);
	int GetInt(unsigned int columnId);
	bool IsNull(unsigned int columnId);

private:
	sqlite3_stmt *m_pStmt;		/** DOES NOT CHANGE */
	unsigned int m_ColCount;	/** DOES NOT CHANGE */
	BaseStringTable m_Strings;	/** DOES NOT CHANGE */
	BaseMemTable *m_pMemory;	/** DOES NOT CHANGE */
	int m_RowCount;
	int m_MaxRows;
	SqField *m_Rows;
	int m_CurRow;
	int m_NextRow;
};
