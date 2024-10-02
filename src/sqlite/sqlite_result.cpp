#include "sqlite_result.h"
#include "sqlite_query.h"
#include "sql_mm.h"

SqResults::SqResults(SqQuery *query)
    : m_pStmt(query->GetStmt()), m_Strings(1024), m_RowCount(0), m_MaxRows(0), m_Rows(NULL), m_CurRow(-1), m_NextRow(0)
{
    m_ColCount = sqlite3_column_count(m_pStmt);
    if (m_ColCount)
    {
        m_ColNames = new std::string[m_ColCount];
        for (unsigned int i = 0; i < m_ColCount; i++)
        {
            m_ColNames[i].assign(sqlite3_column_name(m_pStmt, i));
        }
    }
    else
    {
        m_ColNames = NULL;
    }

    m_pMemory = m_Strings.GetMemTable();
}

SqResults::~SqResults()
{
    delete[] m_ColNames;
    free(m_Rows);
}

int SqResults::GetRowCount()
{
    return m_RowCount;
}

int SqResults::GetFieldCount()
{
    return m_ColCount;
}

const char *SqResults::FieldNumToName(unsigned int columnId)
{
    if (columnId >= m_ColCount)
    {
        return NULL;
    }

    return m_ColNames[columnId].c_str();
}

bool SqResults::FieldNameToNum(const char *name, unsigned int *columnId)
{
    for (unsigned int i = 0; i < m_ColCount; i++)
    {
        if (m_ColNames[i].compare(name) == 0)
        {
            if (columnId)
            {
                *columnId = i;
            }
            return true;
        }
    }
    return false;
}

bool SqResults::MoreRows()
{
    return (m_CurRow < 0) ? (m_RowCount > 0) : (m_CurRow < m_RowCount);
}

ISQLRow *SqResults::FetchRow()
{
    m_CurRow = m_NextRow;
    if (m_CurRow >= m_RowCount)
    {
        return NULL;
    }
    m_NextRow++;
    return this;
}

ISQLRow *SqResults::CurrentRow()
{
    if (!m_RowCount || m_CurRow < 0 || m_CurRow >= m_RowCount)
    {
        return NULL;
    }
    return this;
}

bool SqResults::Rewind()
{
    m_CurRow = -1;
    m_NextRow = 0;
    return true;
}

const char *SqResults::GetString(unsigned int columnId, size_t *length)
{
    SqField *field = GetField(columnId);
    if (!field)
    {
        return nullptr;
    }

    if (field->type == SQLITE_TEXT || field->type == SQLITE_BLOB)
    {
        if (length)
        {
            *length = field->size;
        }
        return m_Strings.GetString(field->u.idx);
    }
    else if (field->type == SQLITE_INTEGER)
    {
        char number[24];
        field->size = UTIL_Format(number, sizeof(number), "%lld", field->u.idx);
        field->type = SQLITE_TEXT;
        field->u.idx = m_Strings.AddString(number);
        if (length)
        {
            *length = field->size;
        }
        return m_Strings.GetString(field->u.idx);
    }
    else if (field->type == SQLITE_FLOAT)
    {
        char number[24];
        field->size = UTIL_Format(number, sizeof(number), "%llf", field->u.d);
        field->type = SQLITE_TEXT;
        field->u.idx = m_Strings.AddString(number);
        if (length)
        {
            *length = field->size;
        }
        return m_Strings.GetString(field->u.idx);
    }
    return nullptr;
}

size_t SqResults::GetDataSize(unsigned int columnId)
{
    SqField *field = GetField(columnId);
    if (!field)
    {
        return 0;
    }

    return field->size;
}

float SqResults::GetFloat(unsigned int columnId)
{
    SqField *field = GetField(columnId);
    if (!field)
    {
        return 0.0f;
    }
    else if (field->type == SQLITE_BLOB || field->type == SQLITE_NULL)
    {
        return 0.0f;
    }

    float fVal = 0.0f;
    if (field->type == SQLITE_FLOAT)
    {
        fVal = field->u.d;
    }
    else if (field->type == SQLITE_TEXT)
    {
        const char *ptr = m_Strings.GetString(field->u.idx);
        if (ptr)
        {
            fVal = (float)atof(ptr);
        }
    }
    else if (field->type == SQLITE_INTEGER)
    {
        fVal = (float)field->u.idx;
    }

    return fVal;
}

int64_t SqResults::GetInt64(unsigned int columnId)
{
    SqField *field = GetField(columnId);
    if (!field)
    {
        return 0.0f;
    }
    else if (field->type == SQLITE_BLOB || field->type == SQLITE_NULL)
    {
        return 0.0f;
    }

    long long val = 0;
    if (field->type == SQLITE_INTEGER)
    {
        val = field->u.idx;
    }
    else if (field->type == SQLITE_TEXT)
    {
        const char *ptr = m_Strings.GetString(field->u.idx);
        if (ptr)
        {
            val = atoll(ptr);
        }
    }
    else if (field->type == SQLITE_FLOAT)
    {
        val = (int64_t)field->u.d;
    }
    return val;
}

int SqResults::GetInt(unsigned int columnId)
{
    SqField *field = GetField(columnId);
    if (!field)
    {
        return 0.0f;
    }
    else if (field->type == SQLITE_BLOB || field->type == SQLITE_NULL)
    {
        return 0.0f;
    }

    int val = 0;
    if (field->type == SQLITE_INTEGER)
    {
        val = field->u.idx;
    }
    else if (field->type == SQLITE_TEXT)
    {
        const char *ptr = m_Strings.GetString(field->u.idx);
        if (ptr)
        {
            val = atoi(ptr);
        }
    }
    else if (field->type == SQLITE_FLOAT)
    {
        val = (int)field->u.d;
    }
    return val;
}

bool SqResults::IsNull(unsigned int columnId)
{
    SqField *field = GetField(columnId);
    if (!field)
    {
        return true;
    }

    return (field->type == SQLITE_NULL);
}

SqField *SqResults::GetField(unsigned int col)
{
    if (m_CurRow < 0 || m_CurRow >= m_RowCount || col >= m_ColCount)
    {
        return NULL;
    }

    return &m_Rows[(m_CurRow * m_ColCount) + col];
}

void SqResults::ResetResultCount()
{
    m_RowCount = 0;
    m_CurRow = -1;
    m_NextRow = 0;
    m_pMemory->Reset();
}

void SqResults::PushResult()
{
    if (m_RowCount + 1 > m_MaxRows)
    {
        if (!m_Rows)
        {
            m_MaxRows = 8;
            m_Rows = (SqField *)malloc(sizeof(SqField) * m_ColCount * m_MaxRows);
        }
        else
        {
            m_MaxRows *= 2;
            m_Rows = (SqField *)realloc(m_Rows, sizeof(SqField) * m_ColCount * m_MaxRows);
        }
    }

    SqField *row = &m_Rows[m_RowCount * m_ColCount];
    for (unsigned int i = 0; i < m_ColCount; i++)
    {
        row[i].type = sqlite3_column_type(m_pStmt, i);
        if (row[i].type == SQLITE_INTEGER)
        {
            row[i].u.idx = sqlite3_column_int64(m_pStmt, i);
            row[i].size = sizeof(long long);
        }
        else if (row[i].type == SQLITE_FLOAT)
        {
            row[i].u.d = (float)sqlite3_column_double(m_pStmt, i);
            row[i].size = sizeof(double);
        }
        else if (row[i].type == SQLITE_BLOB)
        {
            int bytes = sqlite3_column_bytes(m_pStmt, i);
            const void *pOrig;
            if ((pOrig = sqlite3_column_blob(m_pStmt, i)) != NULL)
            {
                void *pAddr;
                row[i].u.idx = m_pMemory->CreateMem(bytes, &pAddr);
                memcpy(pAddr, pOrig, bytes);
            }
            else
            {
                row[i].u.idx = -1;
            }
            row[i].size = sqlite3_column_bytes(m_pStmt, i);
        }
        else if (row[i].type == SQLITE_TEXT)
        {
            const char *str = (const char *)sqlite3_column_text(m_pStmt, i);
            if (str)
            {
                row[i].u.idx = m_Strings.AddString(str);
            }
            else
            {
                row[i].u.idx = -1;
            }
            row[i].size = sqlite3_column_bytes(m_pStmt, i);
        }
        else
        {
            row[i].size = 0;
        }
    }

    m_RowCount++;
}
