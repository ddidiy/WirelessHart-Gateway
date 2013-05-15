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

#include "RawData.h"
#include "Exception.h"
#include <nlib/log.h>

#include <cassert>

#include "sqlite3.h"


namespace sqlitexx {
namespace detail {

RawData::RawData(char** p_pData, int p_nRowsCount, int p_nColsCount) :
	data(p_pData), rowsCount(p_nRowsCount), colsCount(p_nColsCount)
{
}

RawData::~RawData()
{
    try
    {
        if (data != 0)
        {
            ::sqlite3_free_table(data);
            data = 0;
        }
    }
    catch (std::exception& ex)
    {
        LOG_ERROR_APP("[RawData]: ~RawData() - Exception: " << ex.what());
    }
    catch(...)
    {
        LOG_ERROR_APP("[RawData]: ~RawData() - System failure!");
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

