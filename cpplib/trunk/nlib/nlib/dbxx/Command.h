/*
 * Command.h
 *
 *  Created on: Nov 25, 2008
 *      Author: nicu.dascalu
 */

#ifndef NLIB_DBXX_COMMAND_H_
#define NLIB_DBXX_COMMAND_H_

#include "ResultSet.h"
#include "Exception.h"

#include <string>
#include <nlib/datetime.h>
#include <boost/utility.hpp> //for boost::noncopyable
#include <boost/lexical_cast.hpp> //todo: remove it, use boost formatter


namespace nlib {
namespace dbxx {

/**
 * The sql query executor.
 *
 */
template<typename CommandDriver>
class Command: boost::noncopyable
{
	LOG_DEF("nlib.dbxx.mysql.Command");
public:
	Command(Connection<typename CommandDriver::Driver>& connection, const std::string& query) :
		commandDriver(connection, query)
	{
	}

	const std::string& Query() const
	{
		return commandDriver.query;
	}

	typedef dbxx::ResultSet<typename CommandDriver::ResultSetDriver, rstInput> ResultSet;

	typename ResultSet::Ptr ExecuteQuery()
	{
		typename ResultSet::Ptr resultSet(new ResultSet());
		commandDriver.ExecuteQuery(*resultSet);

		return resultSet;
	}

	//	template <ResultSetType type>
	//	typename ResultSet<typename CommandDriver::ResultSetDriver, type>::Ptr ExecuteQuery(ResultSetType type)
	//	{
	//		typedef ResultSet<typename CommandDriver::ResultSetDriver, type> ResultSet_;
	//
	//		typename ResultSet_::Ptr resultSet(new ResultSet_( commandDriver));
	//		commandDriver.ExecuteQuery(*resultSet);
	//
	//		return resultSet;
	//	}

	void ExecuteNonQuery()
	{
		//another form ???
		//ResultSet<CommandDriver, rstNoResultSet> noResultSet;
		//commandDriver.ExecuteQuery(noResultSet);

		commandDriver.ExecuteNonQuery();
	}

	int GetLastInsertRowID()
	{
		return commandDriver.GetLastInsertRowID();
	}

private:
	void Replace(std::string& source, const std::string& find, const std::string& replace)
	{
		if (find.empty() || source.empty() || source.find(find) == std::string::npos || find == replace)
		{
			THROW_EXCEPTION1(dbxx::Exception, boost::str(
			    boost::format("Unable to find: %1% onto %2% for replacing with %3%!") % find % source % replace));
		}
		size_t i;
		while ((i = source.find(find)) != std::string::npos) /*replace all occurences*/
		{
			source.replace(i, find.length(), replace);
		}
	}

	std::string ParameterName(int parameterPos)
	{
		return boost::str(boost::format("?%1$03d") % parameterPos);
	}

	//TODO: move this from here
	std::string ToSqlDbString(const DateTime& dateTime)
	{
		if (DateTime() == dateTime)
		{
			return "1000-01-01 00:00:00";
		}
		//HACK:[Ovidiu] - to avoid representing month as string
		int month = (int) dateTime.date().month();
		return boost::str(boost::format("%1$04d-%2$02d-%3$02d %4$02d:%5$02d:%6$02d") % dateTime.date().year() % month
		    % dateTime.date().day() % dateTime.time_of_day().hours() % dateTime.time_of_day().minutes()
		    % dateTime.time_of_day().seconds());
	}

public:
	void BindParam(int parameterPos, const int& value)
	{
		try
		{
			std::string parameterValue = boost::lexical_cast<std::string>(value);

			Replace(commandDriver.query, ParameterName(parameterPos), parameterValue);
		}
		catch (std::exception& exc)
		{
			THROW_EXCEPTION1(dbxx::Exception, boost::str(boost::format(
			    "Unable to bind int parameter: %1% on position %2%! Exception: %3%") % value % parameterPos % exc.what()));
		}
	}

	void BindParam(int parameterPos, const double& value)
	{
		try
		{
			std::string parameterValue = boost::lexical_cast<std::string>(value);

			Replace(commandDriver.query, ParameterName(parameterPos), parameterValue);
		}
		catch (std::exception& exc)
		{
			THROW_EXCEPTION1(dbxx::Exception, boost::str(boost::format(
			    "Unable to bind int parameter: %1% on position %2%! Exception: %3%") % value % parameterPos % exc.what()));
		}
	}

	void BindParam(int parameterPos, const std::string& value)
	{
		try
		{
			//prepare string with escape chars
			std::string parameterValue;
			parameterValue.reserve(value.size() + 2 + 10);
			parameterValue.push_back('\''); //starting '
			//any ' char should be duplicated for sqlite escapping ...
			for (std::string::const_iterator it = value.begin(); it != value.end(); it++)
			{
				parameterValue.push_back(*it);
				if ('\'' == *it)
					parameterValue.push_back('\'');
			}
			parameterValue.push_back('\''); //endding '

			Replace(commandDriver.query, ParameterName(parameterPos), parameterValue);
			//LOG_DEBUG("Query after bind:" << commandDriver.query << " with value:" << parameterValue);
		}
		catch (std::exception& exc)
		{
			THROW_EXCEPTION1(dbxx::Exception, boost::str(boost::format(
			    "Unable to bind string parameter: %1% on position %2%! Exception: %3%") % value % parameterPos % exc.what()));
		}
	}

	void BindParam(int parameterPos, const DateTime& value)
	{
		try
		{
			BindParam(parameterPos, ToSqlDbString(value));
		}
		catch (std::exception& exc)
		{
			THROW_EXCEPTION1(dbxx::Exception, boost::str(boost::format(
			    "Unable to bind date time parameter on position %1%! Exception: %2%") % parameterPos % exc.what()));
		}
	}

protected:
	CommandDriver commandDriver;
};

} // namespace dbxx {
} //namespace nlib {


#endif /* NLIB_DBXX_COMMAND_H_ */
