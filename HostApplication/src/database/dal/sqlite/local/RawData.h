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

#ifndef NLIB_SQLITEXX_RAWDATA_H_
#define NLIB_SQLITEXX_RAWDATA_H_


namespace sqlitexx {
namespace detail {


class RawData
{
public:
	RawData(char** data, int rows, int cols);
	~RawData();

	const char* RawValue(int row, int col) const;

	int RowsCount() const;

	int ColsCount() const;

private:

	char** data;
	int rowsCount;
	int colsCount;
};

class String
{
public:
	String();
	~String();

	char** operator& ();

	const char* Value() const;

private:
	char *str;
};


} // namespace detail
} // namespace sqlitexx


#endif /*NLIB_SQLITEXX_RAWDATA_H_*/
