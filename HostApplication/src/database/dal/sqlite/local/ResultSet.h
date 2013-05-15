/*
* Copyright (C) 2013 Nivis LLC.
* Email:   opensource@nivis.com
* Website: http://www.nivis.com
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3 of the License.
* 
* Redistribution and use in source and binary forms must retain this
* copyright notice.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef NLIB_SQLITEXX_RESULTSET_H_
#define NLIB_SQLITEXX_RESULTSET_H_

#include <memory> //iterator traits
#include <string>
#include <algorithm>
#include <cassert>
#include <iostream>

#include <boost/utility.hpp> //for noncopyable
#include <boost/format.hpp> //for printf formating style
#include <nlib/datetime.h>
#include "Exception.h"
#include "RawData.h"


namespace sqlitexx {

class Command;

namespace detail {
class Row;
class RowIterator;
}

/**
 * Holds the resulted query date from a command.
 * 
 * @author nicu.dascalu@nivis.com
 */
class ResultSet : boost::noncopyable
{
public:
	typedef detail::RowIterator Iterator;
	typedef detail::Row Row;

private:
	ResultSet(char** data, int rows, int cols);

public:
	~ResultSet();

	/**
	 * Returns the number of rows from the result set.
	 */
	int RowsCount() const;

	/**
	 * Returns the number of columns from the result set.
	 */
	int ColsCount() const;

	/**
	 * Lookups for specified column(field) name in the result set.
	 *
	 * @param name - name of the column.
	 * @return the index of the column or -1 if the name was not found; 0 - index based.
	 */
	int Column(const std::string& name) const;

	/**
	 * 
	 */
	Iterator Begin() const;

	/**
	 *
	 */
	Iterator End() const;

private:
	detail::RawData rawData;

	friend class Command;
	friend class detail::Row;
};

namespace detail {
const int COLUMN_NAMES_ROW = 0;
const int FIRST_DATA_ROW = 1;

/**
 * Holds the current iterated row data.
 */
class Row
{
private:
	/**
	 * Creates a new row object.
	 */
	Row(const ResultSet& resultSet, int row);

public:
	/**
	 * Verifies if the value of the column is NULL.
	 * @column - column index; is 0 base indexed.
	 */
	bool IsNull(int column) const;

	/**
	 * Verifies if the value of the column is NULL.
	 * @column - column name;
	 */
	bool IsNull(const std::string& column) const;

	/**
	 * Retrieves the data from the specified column index.
	 * @column - is 0 base indexed.
	 */
	template <class DataType> DataType Value(int column) const;
	//TODO: add explicit specialization
	nlib::DateTime ValueDateTime(int column) const;

	/**
	 * Retrieves the data from the specified column name.
	 * @column - column name.
	 */
	template <class DataType> DataType Value(const std::string& column);
	//TODO: add explicit specialization
	//DateTime ValueDateTime(const std::string& column);

private:
	const ResultSet& resultSet;
	int row;

	friend class RowIterator;
};

/**
 * Random iterator for ResultSet.
 */
class RowIterator : public std::iterator<std::random_access_iterator_tag, const Row>
{
public:
	typedef RowIterator _Myt;

	RowIterator(const ResultSet& resultSet, int rowIndex) :
		row(resultSet, rowIndex)
	{
	}

	reference operator*() const
	{
		assert(1 <= row.row && row.row <= row.resultSet.RowsCount());
		return row;
	}

	pointer operator->() const
	{
		return &(**this);
	}

	_Myt& operator++()
	{
		// preincrement
		++row.row;
		return (*this);
	}

	_Myt operator++(int)
	{
		// postincrement
		_Myt tmp = *this;
		++*this;
		return (tmp);
	}

	_Myt& operator--()
	{
		// predecrement
		--row.row;
		return (*this);
	}

	_Myt operator--(int)
	{
		// postdecrement
		_Myt tmp = *this;
		--*this;
		return (tmp);
	}

	_Myt& operator+=(difference_type offset);

	_Myt operator+(difference_type offset) const;

	difference_type operator-(const _Myt& right) const;

	reference operator[](difference_type offest) const;

	bool operator==(const _Myt& right) const
	{
		return (&row.resultSet == &right.row.resultSet && row.row == right.row.row);
	}

	bool operator!=(const _Myt& right) const
	{
		return !(this->operator ==(right));
	}

	bool operator<(const _Myt& right) const;

	bool operator>(const _Myt& right) const;

	bool operator<=(const _Myt& right) const;

	bool operator>=(const _Myt& right) const;

private:
	Row row;
};

inline const std::string& NO_DATETIME()
{
	static std::string nodate("no date");
	return nodate;
}
std::string ToDbString(const nlib::DateTime& dateTime);
nlib::DateTime FromDbString(const std::string& dateTime);

} //namespace detail


inline ResultSet::ResultSet(char** data, int rowsCount, int colsCount) :
	rawData(data, rowsCount, colsCount)
{
}

inline ResultSet::~ResultSet()
{
	//TODO: [nicu.dascalu] - release the row data.
}

inline
int ResultSet::RowsCount() const
{
	return rawData.RowsCount();
}

inline
int ResultSet::ColsCount() const
{
	return rawData.ColsCount();
}

inline
int ResultSet::Column(const std::string& name) const
{
	return -1;
}

inline
ResultSet::Iterator ResultSet::Begin() const
{
	return ResultSet::Iterator(*this, detail::FIRST_DATA_ROW);
}

inline
ResultSet::Iterator ResultSet::End() const
{
	return ResultSet::Iterator(*this, RowsCount() + 1);
}

namespace detail {
inline Row::Row(const ResultSet& p_rResultSet, int p_nRow) :
	resultSet(p_rResultSet), row(p_nRow)
{
}

inline
bool Row::IsNull(int column) const
{
	const char* value = resultSet.rawData.RawValue(row, column);
	return value == 0;
}

template <class DataType> inline
DataType Row::Value(int column) const
{
	assert(column >= 0);

	const char* value = resultSet.rawData.RawValue(row, column);
	DataType p;
	return p;
}

inline
nlib::DateTime Row::ValueDateTime(int column) const
{
	assert(column >= 0);

	const char* value = resultSet.rawData.RawValue(row, column);
	assert(value != NULL);

	return detail::FromDbString(value);
}

template <class DataType>
inline
DataType Row::Value(const std::string& column)
{
	int columnIndex = resultSet.Column(column);
	assert(columnIndex >= 0);

	return Value<DataType>(columnIndex);
}

inline
std::string ToDbString(const nlib::DateTime& dateTime)
{
	if(nlib::DateTime() == dateTime)
	{
		return NO_DATETIME();
	}
	//HACK:[Ovidiu] - to avoid representing month as string
	int month = (int)dateTime.date().month();
	return boost::str(
			boost::format("%1$04d-%2$02d-%3$02d %4$02d:%5$02d:%6$02d")
			% dateTime.date().year()
			% month
			% dateTime.date().day()
			% dateTime.time_of_day().hours()
			% dateTime.time_of_day().minutes()
			% dateTime.time_of_day().seconds()
	);
}

//YYYY-MM-DD HH:mm:ss
inline
nlib::DateTime FromDbString(const std::string& dateTime)
{
	if(NO_DATETIME() == dateTime)
		return nlib::DateTime();
	
	int year, month, day, hour, minute, second;
	char	separator; 
	std::istringstream streamDateTime(dateTime);
	if((streamDateTime >> year >> separator >> month >> separator >> day 			
			>> hour >> separator >> minute >> separator >> second))			
	{
		return nlib::CreateTime(year, month, day, hour, minute, second);			
	}
	else
	{				
		throw std::invalid_argument(dateTime);
	}
}

}// namespace detail

} //namespace sqlitexx


#endif /*NLIB_SQLITEXX_RESULTSET_H_*/
