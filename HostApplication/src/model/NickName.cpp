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


#include <WHartHost/model/NickName.h>

#include <algorithm>
#include <boost/format.hpp>
#include <stdio.h>


#include <WHartHost/Utils.h>

namespace hart7 {
namespace hostapp {



const NickName& NickName::NONE()
{
	static NickName instance;
	return instance;
}

NickName::NickName(WHartShortAddress address_)
{
	address = address_;
	textAddress = ConvertToHex((unsigned short)address);
}

NickName::NickName(const std::string& address_)
{
	if (address_.empty())
	{
		address = 0;
		textAddress = "";
	} else
	{
		address = atoi(address_.c_str());
		textAddress = address_;
	}	
}

NickName::~NickName()
{
}


void NickName::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << textAddress;
}

const std::string NickName::ToString() const
{
	return textAddress;
}

//operators
bool operator<(const NickName& left, const NickName& rigth)
{
	return left.address < rigth.address;
}
bool operator==(const NickName& left, const NickName& rigth)
{
	return left.address == rigth.address;
}

bool operator!=(const NickName& left, const NickName& rigth)
{
	return !(left == rigth);
}

std::ostream& operator<< (std::ostream& p_rStream, const NickName& p_rNickName)
{	p_rNickName.DumpToStream(p_rStream);
	return p_rStream;
}

} //namespace hostapp
} //namespace hart7
