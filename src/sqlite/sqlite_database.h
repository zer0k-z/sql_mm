#pragma once

#include "public/sqlite_mm.h"
#include "vendor/sqlite-source/sqlite3.h"
#include <mutex>

class SqDatabase : public ISQLConnection
{
public:
	void Connect(ConnectCallbackFunc callback);
	void Query(char *query, QueryCallbackFunc callback);
	void Query(const char *query, QueryCallbackFunc callback, ...);
	void Destroy();
	std::string Escape(char *string);
	std::string Escape(const char *string);
private:
	sqlite3 *m_sq3;
	std::mutex m_FullLock;
	bool m_Persistent;
	std::string m_LastError;
	int m_LastErrorCode;
};