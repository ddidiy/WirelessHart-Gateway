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

#ifndef _CSV_H_
#define _CSV_H_

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <stdint.h>
#include <string>
class CCsv
{
public:
	CCsv() ;
	~CCsv() ;

public:
	//////////////////////////////////////////////////////////////////////////////
	/// @brief End of record.
	//////////////////////////////////////////////////////////////////////////////
	const bool Eor() const ;
	void Eor(bool) ;
	const char* CurrentIt() const ;
	const char* GetLine() ;
	CCsv& SetLine( const char* line ) ;
	void SetSeparator( char sep, char eor='\n' ) ;

	//////////////////////////////////////////////////////////////////////////////
	/// @brief Drop the current CSV line.
	//////////////////////////////////////////////////////////////////////////////
	void Reset() ;

	//////////////////////////////////////////////////////////////////////////////
	/// @brief Put an integer in the CSV line.
	//////////////////////////////////////////////////////////////////////////////
	void Put( int& in ) ;

	//////////////////////////////////////////////////////////////////////////////
	/// @brief Put a float in the CSV line.
	//////////////////////////////////////////////////////////////////////////////
	void Put( float& in ) ;
	void Put( double& in ) ;
	void Put( uint64_t in ) ;

	//////////////////////////////////////////////////////////////////////////////
	/// @brief Put a string in the CSV line.
	//////////////////////////////////////////////////////////////////////////////
	void Put( const char* in ) ;
	void Put( const char* in, size_t inLen ) ;


	//////////////////////////////////////////////////////////////////////////////
	/// @brief Get an integer out of the CSV line.
	//////////////////////////////////////////////////////////////////////////////
	CCsv& Get( int& out ) ;

	//////////////////////////////////////////////////////////////////////////////
	/// @brief Get a string out the CSV line.
	//////////////////////////////////////////////////////////////////////////////
	CCsv& Get( char*&out ) ;
	CCsv& Get( std::string&out) ;

protected:
	bool readValue() ;
	void addFreeSpace( int extra ) ;
	void updateIt( int rv ) ;

protected:
	struct Impl ;
	Impl* m_pImpl ;
} ;

#endif /*_CSV_H_ */
