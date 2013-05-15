/*
 * Address.cpp
 *
 *  Created on: Dec 5, 2008
 *      Author: nicu.dascalu
 */

#include <nlib/socket/Address.h>

#include <boost/format.hpp>

namespace nlib {
namespace socket {

std::string Address::ToString() const
{
	return boost::str(boost::format("[%1%:%2%]") % host % port);
}

const Address& Address::operator=(const Address& rigth)
{
	host = rigth.host;
	port = rigth.port;
	return *this;
}


bool operator<(const Address& left, const Address& rigth)
{
	return left.host < rigth.host || left.port < rigth.port;
}

bool operator==(const Address& left, const Address& rigth)
{
	return !(left < rigth) && !(rigth < left);
}

bool operator!=(const Address& left, const Address& rigth)
{
	return !(left == rigth);
}

} //namespace socket
} //namespace nlib

