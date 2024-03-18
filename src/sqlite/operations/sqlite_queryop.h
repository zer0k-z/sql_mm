#pragma once
#include "../sqlite_database.h"
#include "../sqlite_result.h"

class TSQLiteQueryOp : public ThreadOperation
{
public:
	TSQLiteQueryOp(SqDatabase *con, std::string query, QueryCallbackFunc func) : m_pCon(con), m_szQuery(query), m_callback(func)
	{

	}

	~TSQLiteQueryOp();

	void RunThreadPart();
	void CancelThinkPart();
	void RunThinkPart();
private:
	SqDatabase *m_pCon;
	std::string m_szQuery;
	QueryCallbackFunc m_callback;
	SqResults *m_res = nullptr;
	SqQuery *m_pQuery = nullptr;
	char m_szError[255];
};