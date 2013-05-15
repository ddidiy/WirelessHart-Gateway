/*
 * MySQLDriver.h
 *
 *  Created on: Nov 25, 2008
 *      Author: nicu.dascalu
 */

#ifndef NLIB_DBXX_MYSQLDRIVER_H_
#define NLIB_DBXX_MYSQLDRIVER_H_

#include "MySqlApi.h"
#include <string>
#include <cassert>
#include <boost/lexical_cast.hpp> //for converting numeric <-> to string
#include <nlib/log.h>
#include <nlib/datetime.h>
#include "../Exception.h"

namespace nlib {
namespace dbxx {
namespace mysql {

struct InitMySqlLibrary
{
	InitMySqlLibrary()
	{
		//nothing to do because mysql_init will call automatically mysql_server_init
	}
	~InitMySqlLibrary()
	{
		mysql_server_end();
	}
};

#define GET_POINTER(p, str)\
	(assert(p != NULL && str" must not be null!"), p)


class ConnectionDriver
{
	LOG_DEF("nlib.dbxx.mysql.ConnectionDriver");

private:
	std::string server, user, password, dbName;

	MYSQL* p;
	void Pointer(MYSQL* p_)
	{
		p = p_;
	}

	InitMySqlLibrary libraryinit;
public:
	ConnectionDriver()
	{
		p = NULL;
	}
	void ConnectionString(const std::string& server_, const std::string& user_, const std::string& password_,
	  const std::string& dbName_)
	{
		server = server_;
		user = user_;
		password = password_;
		dbName = dbName_;
	}

	void Open()
	{
		if (p)
		{
			//:INFO[Ovidiu] - exception not throwed to be able to try open database again later
			//THROW_EXCEPTION1(dbxx::Exception, "Database already opened!");
		}
		Pointer(mysql_init(NULL));
		if (!p)
		{
			THROW_EXCEPTION1(Exception, "insufficient memory to allocate a new MYSQL object!");
		}

		//TODO:[Ovidiu] add timeout option if necessary //MYSQL_OPT_CONNECT_TIMEOUT
		my_bool autoreconnect = true;
		if(mysql_options(GET_POINTER(p, "MYSQL"), MYSQL_OPT_RECONNECT, &autoreconnect))
		{
			LOG_WARN("Failed to set mysql options! Unknown option:'MYSQL_OPT_RECONNECT'.");
		}
		else
		{
			LOG_WARN("Succesfully set mysql options!");
		}

		if (!mysql_real_connect(GET_POINTER(p, "MYSQL"), server.c_str(), user.c_str(), password.c_str(), dbName.c_str(), 0, NULL, 0))
		{
			LOG_ERROR("Failed to connect to database " << ErrorMessage());

			THROW_EXCEPTION1(dbxx::Exception, boost::str(boost::format("Failed to connect to database! Error code: %1%. Error message: %2%.")
			   %ErrorCode() % ErrorMessage()));
		}

		LOG_DEBUG("Open database:" << dbName << " on server:" << server);
	}

	void Close()
	{
		if (p)
		{
			MYSQL* copy = p;
			Pointer(NULL);

			mysql_close(copy);
			LOG_DEBUG("Close database:" << dbName << " on server:" << server);
		}
	}

	unsigned int ErrorCode() const
	{
		return p
		  ? mysql_errno(p)
		  : (unsigned int)-1;
	}

	const char* ErrorMessage() const
	{
		return p
		  ? mysql_error(p)
		  : "MYSQL is NULL!";
	}

	void BeginTx();
	void CommitTx();
	void RollbackTx();

private:
	friend class CommandDriver;
};

/**
 *
 */
class ResultSetDriver
{

public:
	class Row
	{
		LOG_DEF("nlib.dbxx.mysql.Row");

		MYSQL_ROW p;
		ResultSetDriver& resultSetDriver;

		void Pointer(MYSQL_ROW p_)
		{
			p = p_;
		}

		Row(ResultSetDriver& resultSetDriver_) :
			resultSetDriver(resultSetDriver_)
		{
		}

public:
		/**
		 * Verifies if the value of the column is NULL.
		 * @column - column index; is 0 base indexed.
		 */
		bool IsNull(int column) const
		{
			assert(0 <= column && column <= resultSetDriver.ColsCount());
			return GET_POINTER(p, "MYSQL_ROW")[column] == NULL;
		}

		/**
		 * Verifies if the value of the column is NULL.
		 * @column - column name;
		 */
		//bool IsNull(const std::string& column) const;

		/**
		 * Retrieves the data from the specified column index.
		 * @column - is 0 base indexed.
		 */
		template<class DataType> DataType Value(int column) const
		{
			DataType result;
			GetValue(column, result);

			return result;
		}

private:
		template<class DataType> void GetValue(int column, DataType& value) const
		{
			assert(!IsNull(column));
			value = boost::lexical_cast<DataType>(GET_POINTER(p, "MYSQL_ROW")[column]);
		}

		//TODO: move this from here
		/*MySQL retrieves and displays DATETIME values in 'YYYY-MM-DD HH:MM:SS' format */
		DateTime FromMySqlDbString(const std::string& dateTime) const
		{
			if("1000-01-01 00:00:00" == dateTime)
					return DateTime();

			int year, month, day, hour, minute, second;
			char separator;
			std::istringstream streamDateTime(dateTime);
			if((streamDateTime >> year >> separator >> month >> separator >> day
							>> hour >> separator >> minute >> separator >> second))
			{
				return CreateTime(year, month, day, hour, minute, second);
			}
			else
			{
				throw std::invalid_argument(dateTime);
			}
		}

		void GetValue(int column, DateTime& value) const
		{
			assert(!IsNull(column));
			value=FromMySqlDbString(GET_POINTER(p, "MYSQL_ROW")[column]);
		}

		friend class ResultSetDriver;
	};

private:
	MYSQL_RES* p;

	void Pointer(MYSQL_RES* p_)
	{
		p = p_;
	}

	Row currentRow;

public:
	ResultSetDriver() : currentRow(*this)
	{
		p = NULL;
	}

	~ResultSetDriver()
	{
		if (p)
		{
			MYSQL_RES* copy = p;
			Pointer(NULL);

			mysql_free_result(copy);
		}
	}

	/**
	 * Returns the number of rows from the result set.
	 */
	int RowsCount() const
	{
		return (int) mysql_num_rows(GET_POINTER(p, "MYSQL_RES"));
	}

	/**
	 * Returns the number of columns from the result set.
	 */
	int ColsCount() const
	{
		return mysql_num_fields(GET_POINTER(p, "MYSQL_RES"));
	}

	const Row& CurrentRow() const
	{
		return currentRow;
	}

	/**
	 * 	//TODO:[Ovidiu] - implement this
	 * Lookups for specified column(field) name in the result set.
	 *
	 * @param name - name of the column.
	 * @return the index of the column or -1 if the name was not found; 0 - index based.
	 */
	int Column(const std::string& name) const;

	bool FetchNext()
	{
		currentRow.Pointer(mysql_fetch_row(GET_POINTER(p, "MYSQL_RES")));

		return currentRow.p != NULL;
	}

	friend class CommandDriver;
};

/**
 *
 */
class CommandDriver
{
	LOG_DEF("nlib.dbxx.mysql.CommandDriver");
public:
	typedef ConnectionDriver Driver;
	typedef mysql::ResultSetDriver ResultSetDriver;

public:
	std::string query;

private:
	ConnectionDriver& connection;

public:
	CommandDriver(ConnectionDriver& connection_, const std::string& query_) :
	query(query_), connection(connection_)
	{
	}

	void ExecuteQuery(ResultSetDriver& resultSet)
	{
		LOG_DEBUG("Executing query:" << query);

		if (mysql_query(GET_POINTER(connection.p, "MYSQL"), query.c_str()))
		{
			THROW_EXCEPTION1(dbxx::Exception, boost::str(boost::format(
									"ExecuteQuery failed. Error code=%1%. Error message=%2%. Query=[%3%].") % connection.ErrorCode() % connection.ErrorMessage() % query));
		}
		resultSet.Pointer(mysql_store_result(GET_POINTER(connection.p, "MYSQL")));

		// In case of multiple results, use first, drop the rest
		while(mysql_next_result(GET_POINTER(connection.p, "MYSQL")) == 0)
		{
			MYSQL_RES* result = mysql_use_result(GET_POINTER(connection.p, "MYSQL"));
			mysql_free_result(result);
		}

		if (!resultSet.p)
		{
			THROW_EXCEPTION1(dbxx::Exception, boost::str(boost::format("ExecuteStoreResult failed. Error code=%1%. Error message=%2%. Query=[%3%].") % connection.ErrorCode() % connection.ErrorMessage() % query));
		}
	}

	void ExecuteNonQuery()
	{
		LOG_DEBUG("Executing query:" << query);

		if (mysql_query(GET_POINTER(connection.p, "MYSQL"), query.c_str()))
		{
			THROW_EXCEPTION1(dbxx::Exception, boost::str(boost::format("ExecuteNonQuery failed! Error code=%1%. Error message=%2%. Query=[%3%].")	% connection.ErrorCode() % connection.ErrorMessage() % query));
		}
	}

	int GetLastInsertRowID() const
	{
		return mysql_insert_id(GET_POINTER(connection.p, "MYSQL"));
	}
};

inline
void ConnectionDriver::BeginTx()
{
	CommandDriver command(*this, "START TRANSACTION");
	command.ExecuteNonQuery();
}

inline
void ConnectionDriver::CommitTx()
{
	CommandDriver command(*this, "COMMIT");
	command.ExecuteNonQuery();
}

inline
void ConnectionDriver::RollbackTx()
{
	CommandDriver command(*this, "ROLLBACK");
	command.ExecuteNonQuery();
}

} // namespace mysql
} // namespace dbxx
} // namespace nlib


#endif /* NLIB_DBXX_MYSQLDRIVER_H_ */
