/*
 * Connection.h
 *
 *  Created on: Nov 24, 2008
 *      Author: nicu.dascalu
 */

#ifndef NLIB_DBXX_CONNECTION_H_
#define NLIB_DBXX_CONNECTION_H_

#include "Exception.h"
#include <boost/utility.hpp> //for boost::noncopyable

namespace nlib {
namespace dbxx {

/**
 * It is not thread safe.
 */
template<typename ConnectionDriver> class Connection : public ConnectionDriver, boost::noncopyable
{
public:
	~Connection()
	{
		Close();
	}

	/**
	 * Opens the provided database.
	 *
	 * @throws Exception - if database file not found or corrupted.
	 */
	void Open()
	{
		ConnectionDriver::Open();
	}

	/**
	 * Close the connection.
	 */
	void Close()
	{
		ConnectionDriver::Close();
	}

private:
//	template<typename CommandDriver>
//	friend class Command ;
//
//	template<typename Driver_>
//	friend class Transaction ;
};

/**
 * The sql transaction creator.
 */
template<typename Driver> class Transaction : boost::noncopyable
{
public:
	Transaction(Connection<Driver>& connection_) :
		connection(connection_)
	{
		started = false;
	}

	~Transaction()
	{
		Rollback();
	}

	void Begin()
	{
		if (started)
		{
			THROW_EXCEPTION1(dbxx::Exception, "A transaction already started!");			
		}
		connection.BeginTx();
		started = true;
	}

	void Commit()
	{
		if (!started)
		{
			THROW_EXCEPTION1(dbxx::Exception, "A transaction is not started! So nothing to commit!");
		}
		connection.CommitTx();
		started = false;
	}

	void Rollback()
	{
		if (started)
		{
			started = false;
			connection.RollbackTx();
		}
	}

private:
	Connection<Driver>& connection;
	bool started;
};

} // namespace dbxx
} // namespace nlib

#endif /* NLIB_DBXX_CONNECTION_H_ */
