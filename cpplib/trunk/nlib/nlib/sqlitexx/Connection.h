#ifndef NLIB_SQLITEXX_H_
#define NLIB_SQLITEXX_H_

#include <string>
#include <nlib/datetime.h>
//#include <nlib/log.h>
#include "Log.h"
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

#include "ResultSet.h"
#include <libs/sqlitexx/src/sqlite-3.5.8/sqlite3.h>



namespace nlib {
namespace sqlitexx {

class Exception;

typedef boost::shared_ptr<ResultSet> ResultSetPtr;

/**
 * Holds the raw connection to the sqllite database.
 * It is not thread safe.
 */
class Connection : boost::noncopyable
{
	LOG_DEF("nlib.sqlitexx.Connection")
	;
public:
	Connection();
	~Connection();

	/**
	 * Opens the provided database.
	 *
	 * @dbPath - path to the database file.
	 * @timeout - the timeout of any database operation (in seconds)
	 * @throws Exception - if database file not found or corrupted.
	 */
	void Open(const std::string& dbPath, int timeout = 20);

	/**
	 * Close the connection.
	 */
	void Close();

	sqlite3* GetRawDbObj() { return database; }
	void LogIfLastError(int p_nResultCode, const char* p_szContext = "");

private:
	friend class Command;
	sqlite3* database;
};

/**
 * The sql query executor.
 */
class Command : boost::noncopyable
{
	LOG_DEF("nlib.sqlitexx.Command")
	;
public:
	Command(Connection& connection, const std::string& query);
	~Command();

	template <class DataType> void BindParam(int parameter, const DataType& value);

	void BindParam(int parameterPos, const int& value);
	void BindParam(int parameterPos, const double& value);
	void BindParam(int parameterPos, const std::string& value);
	void BindParam(int parameterPos, const DateTime& value);

	ResultSetPtr ExecuteQuery();

	void ExecuteNonQuery();

	static ResultSetPtr ExecuteQuery(sqlite3* database, const char* p_szQuery);

	static void ExecuteNonQuery(sqlite3* database, const char* p_szQuery );

	int GetLastInsertRowID();

	
public:
	inline
	const std::string Query() const
	{
		return query;
	}
private:
	Connection& connection;
	std::string query;
};

/**
 * The sql transaction creator.
 */
class Transaction : boost::noncopyable
{
	LOG_DEF("nlib.sqlitexx.Transaction")
	;
public:
	Transaction(Connection& connection);
	~Transaction();

	void Begin();

	void Commit();

	void Rollback();

private:
	Connection& connection;
	bool started;
};


/**
* The sqlite stmt helper
*/
class CSqliteStmtHelper : boost::noncopyable
{
	LOG_DEF("nlib.sqlitexx.CSqliteStmtHelper")
		;
public:
	CSqliteStmtHelper(Connection& connection);
	~CSqliteStmtHelper();
	void	Release();		

	sqlite3_stmt *GetStmt() { return m_pStmt; }

	// prepare and step
	int		Prepare(const char* p_szSql);
	int		Step (bool p_bExec);
	int		Step_Exec () { return Step(true); }
	int		Step_GetRow () { return Step(false); }

	// bind
	int		BindInt ( int p_nPos, int p_nValue );
	//int		BindInt64 ( int p_nPos, int p_nValue );
	int		BindDouble ( int p_nPos, double p_dValue );
	int		BindText ( int p_nPos, const char* p_szText, void(*p_fctFree)(void*) = SQLITE_STATIC ); //use SQLITE_TRANSIENT if you want that sqlite to make a copy
	int		BindDateTime( int p_nPos,const nlib::DateTime& p_oDateTime );
	int		BindNull ( int p_nPos);
	

	// get value
	int		Column_Check ( int p_nCol, int p_nColType);

	int		Column_IsNull (int p_nCol, bool *  p_pbNull) { int res = Column_Check(p_nCol, SQLITE_NULL); *p_pbNull = (res == SQLITE_OK); return res; }
	int		Column_GetInt ( int p_nCol, int * p_pnValue);
	//int		Column_GetInt64 ( int p_nCol, int * p_pnValue);
	int		Column_GetDouble ( int p_nCol, double * p_pdValue);
	int		Column_GetText ( int p_nCol, char* p_pText, int p_nMax);
	int		Column_GetText ( int p_nCol, std::string* p_poText);
	int		Column_GetDateTime ( int p_nCol, nlib::DateTime* p_poDateTime);

	// if you do not care for error
	bool	Column_IsNull (int p_nCol) { return Column_Check(p_nCol, SQLITE_NULL) == SQLITE_OK; }
	int		Column_GetInt (int p_nCol) 
	{ 
		int nValue = 0;
		Column_GetInt(p_nCol,&nValue);
		return nValue;
	}

	nlib::DateTime	Column_GetDateTime(int p_nCol) 
	{ 
		nlib::DateTime	oTmp ;
		Column_GetDateTime(p_nCol,&oTmp);
		return oTmp;
	}
	std::string		Column_GetText(int p_nCol) 
	{ 
		std::string	oTmp = "";
		Column_GetText(p_nCol,&oTmp);
		return oTmp;
	}

	void	ResetStep();	

private:
	Connection&		m_oConnection;
	char*			m_szSql;

	

	sqlite3_stmt *	m_pStmt;
	int				m_nStmtNeedReset;			

};





} //namespace sqlitexx
} //namspace nlib

#endif /*NLIB_SQLITEXX_H_*/
