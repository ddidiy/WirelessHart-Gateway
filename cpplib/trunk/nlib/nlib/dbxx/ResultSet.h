/*
 * ResultSet.h
 *
 *  Created on: Nov 26, 2008
 *      Author: nicu.dascalu
 */

#ifndef NLIB_DBXX_RESULTSET_H_
#define NLIB_DBXX_RESULTSET_H_

#include <string>
#include <memory> //for iterator traits
#include <nlib/datetime.h>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp> //for boost::noncopyable
namespace nlib {
namespace dbxx {

enum ResultSetType
{
	rstNoResultSet = 0, //not used
	rstInput = 1,
	rstForwardOnly = 2, //not used
	rstRandomAccess = 3
//not used
// not used
};

template<typename ResultSetDriver, ResultSetType type = rstInput>
class ResultSet;

/**
 *
 *
 */
template<typename ResultSetDriver>
class ResultSet<ResultSetDriver, rstInput> : public ResultSetDriver, private boost::noncopyable
{
public:
	typedef boost::shared_ptr<ResultSet> Ptr;

	/**
	 * the input iterator over the RecordSet.
	 */
	class Iterator//: public std::iterator<std::input_iterator_tag, const typename ResultSetDriver::Row>
	{
		LOG_DEF("nlib.dbxx.Iterator");
	private:
		/*
		 * access to driver implemantation
		 */
		ResultSetDriver& resultSetDriver;
		/*
		 * used to diff from a regular iterator instance and the end iterator.
		 * the end instance iterator supports only comparition.
		 */
		bool isEnd;

	public:
		typedef	const typename ResultSetDriver::Row& reference;
		typedef const typename ResultSetDriver::Row* pointer;

		Iterator(ResultSetDriver& resultSetDriver_, bool isEnd_) :
		resultSetDriver(resultSetDriver_), isEnd(isEnd_)
		{
		}

		reference operator*() const
		{
			assert(isEnd == false);
			return resultSetDriver.CurrentRow();
		}

		pointer operator->() const
		{
			return &(**this);
		}

		Iterator& operator++()
		{
			// preincrement
			assert(isEnd == false);

			isEnd = !resultSetDriver.FetchNext();
			return (*this);
		}

		Iterator operator++(int)
		{
			// postincrement, same with preincrement
			return ++(*this);
		}

		bool operator==(const Iterator& right) const
		{
			return (&resultSetDriver == &right.resultSetDriver && isEnd == right.isEnd);
		}

		bool operator!=(const Iterator& right) const
		{
			return !(this->operator ==(right));
		}
	};

private:
	//flag that assure that begin is called only one per instance
	bool beginCalled;

public:
	~ResultSet()
	{
	}

	/**
	 * This method should be called only once per RecordSet instance.
	 */
	Iterator Begin()
	{
		assert(beginCalled == false && "Single time should be called!");

		if (ResultSetDriver::RowsCount()> 0)
		{
			beginCalled = true;
			ResultSetDriver::FetchNext();//goto first row
			return Iterator(*this, false);
		}
		return End();
	}

	Iterator End()
	{
		return Iterator(*this, true);
	}

	//instanciable only from Command
private:
	ResultSet()
	{
		beginCalled = false;
	}

	template<typename CommandDriver>
	friend class Command;
};

} // namespace dbxx {
} //namespace nlib {


#endif /* NLIB_DBXX_RESULTSET_H_ */
