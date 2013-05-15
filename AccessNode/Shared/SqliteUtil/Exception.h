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

#ifndef NLIB_SQLITEXX_EXCEPTION_H_
#define NLIB_SQLITEXX_EXCEPTION_H_

#include <nlib/exception.h>

/// @addtogroup libshared
/// @{

namespace sqlitexx {
	class Exception : public nlib::Exception
	{
	public :
		Exception(const std::string& message_, int errorCode_ = -1) throw()
			: nlib::Exception(message_), errorCode(errorCode_)  
		{		
		}

		const int ErrorCode() const throw()
		{
			return errorCode;
		}
		
	private:
		int errorCode;
	};

}// namespace sqlitexx


/// @}
#endif /*NLIB_SQLITEXX_EXCEPTION_H_*/
