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

#ifndef NICKNAME_H_
#define NICKNAME_H_

#include <string>
#include <iostream>

#include <boost/cstdint.hpp>

#include <nlib/log.h>

#include <nlib/exception.h>

#include <WHartStack/WHartTypes.h>

namespace hart7 {
namespace hostapp {


class NickName
{
	LOG_DEF("hart7.hostapp.Nickname");
	
public:
	static const NickName& NONE();
	NickName(WHartShortAddress address = 0);
	NickName(const std::string& address);
	virtual ~NickName();

//conversion
public:
	const WHartShortAddress Address() const { return address; }
	const std::string ToString() const;

protected:
	void DumpToStream(std::ostream& p_rStream) const;


//operators
public:
	friend bool operator<(const NickName& left, const NickName& rigth);
	friend bool operator==(const NickName& left, const NickName& rigth);
	friend std::ostream& operator<< (std::ostream& p_rStream, const NickName& p_rNickName);

//
private:
	WHartShortAddress address;
	std::string textAddress;
};

bool operator!=(const NickName& left, const NickName& rigth);

std::ostream& operator<< (std::ostream& p_rStream, const NickName& p_rNickName);

} // namespace hostapp
} // namespace hart7

#endif 
