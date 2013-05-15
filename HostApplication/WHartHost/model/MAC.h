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

#ifndef MAC_H_
#define MAC_H_

#include <string>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include <boost/cstdint.hpp> //used for inttypes
#include <boost/format.hpp>

#include <nlib/log.h>

#include <nlib/exception.h>

#include <WHartStack/WHartTypes.h>


namespace hart7 {
namespace hostapp {


/*
 * Represents a serial number for a node.
 */
class MAC
{
	LOG_DEF("hart7.hostapp.MAC");
	
//
public:
	MAC(const MAC& mac);
	/**
	 * last 5 bytes
	 */
	MAC(const boost::uint8_t * address = 0);

	/**
	 * the format should by like: XX-XX-XX-XX-XX-XX-XX-XX (old format: XXXX:XXXX:XXXX:XXXX)
	 */
	MAC(const std::string& address);
	
//conversion
public:
	const WHartUniqueID Address() const;
	const std::string ToString() const;
protected:
	void DumpToStream(std::ostream& p_rStream) const;
	

//operators
public:
	const MAC& operator=(const MAC& rhs);
	friend bool operator<(const MAC& lhs, const MAC& rhs);
	friend bool operator==(const MAC& lhs, const MAC& rhs);
	friend std::ostream& operator<< (std::ostream& p_rStream, const MAC& p_rMac);

public:
	static const int SIZE = 8;
	static const int TEXT_SIZE = 23;
//
private:
	boost::uint8_t	address[SIZE];
	std::string		textAddress;
};

bool operator<(const MAC& lhs, const MAC& rhs);
bool operator==(const MAC& lhs, const MAC& rhs);
bool operator!=(const MAC& lhs, const MAC& rhs);

std::ostream& operator<< (std::ostream& p_rStream, const MAC& p_rMac);

} // namespace hostapp
} // namespace hart7

#endif /*MAC_H_*/
