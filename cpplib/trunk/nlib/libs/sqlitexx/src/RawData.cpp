#include <nlib/sqlitexx/detail/RawData.h>

#include <cassert>

#include "sqlite-3.5.8/sqlite3.h"

namespace nlib {
namespace sqlitexx {
namespace detail {

RawData::RawData(char** data, int rowsCount, int colsCount) :
	data(data), rowsCount(rowsCount), colsCount(colsCount)
{
}

RawData::~RawData()
{
	if (data != 0)
	{
::		sqlite3_free_table(data);
		data = 0;
	}
}

const char* RawData::RawValue(int row, int column) const
{
	assert(0 <= column && column < ColsCount());
	assert(0 <= row && row < RowsCount() + 1);

	return data[ColsCount() * row + column];
}

int RawData::RowsCount() const
{
	return rowsCount;
}

int RawData::ColsCount() const
{
	return colsCount;
}

String::String()
{
	str = 0;
}

String::~String()
{
	if (str != 0)
	{
		::sqlite3_free(str);
		str = 0;
	}
}

char** String::operator& ()
{
	return &str;
}

const char* String::Value() const
{
	return str;
}

} // namespace detail
} // namespace sqlitexx
} // namespace nlib
