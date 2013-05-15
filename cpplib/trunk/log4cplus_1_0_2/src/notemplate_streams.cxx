/*
 * notemplate_streams.cxx
 *
 *  Created on: Jan 19, 2009
 *      Author: nicu.dascalu
 */

#include <log4cplus/notemplate_streams.h>
#include <sstream>

namespace log4cplus {
namespace notemplate {

class raw_ostringstring
{
public:
	std::ostringstream s;
};

ostring_stream::ostring_stream()
{
	osstr = new raw_ostringstring();
}

ostring_stream::~ostring_stream()
{
	if (osstr)
	{
		delete osstr;
		osstr = NULL;
	}
}

std::string ostring_stream::str() const
{
	return osstr->s.rdbuf()->str();
}


ostring_stream::__ostream_type&
ostring_stream::operator<<(long __n)
{
	(osstr->s) << __n;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(unsigned long __n)
{
	(osstr->s) << __n;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(bool __n)
{
	(osstr->s) << __n;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(short __n)
{
	(osstr->s) << __n;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(unsigned short __n)
{
	(osstr->s) << __n;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(int __n)
{
	(osstr->s) << __n;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(unsigned int __n)
{
	(osstr->s) << __n;
	return (*this);
}

#ifdef _GLIBCXX_USE_LONG_LONG
ostring_stream::__ostream_type&
ostring_stream::operator<<(long long __n)
{
	(osstr->s) << __n;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(unsigned long long __n)
{
	(osstr->s) << __n;
	return (*this);
}
#endif

ostring_stream::__ostream_type&
ostring_stream::operator<<(double __f)
{
	(osstr->s) << __f;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(float __f)
{
	(osstr->s) << __f;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(long double __f)
{
	(osstr->s) << __f;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(const void* __p)
{
	(osstr->s) << __p;
	return (*this);
}

// strings
ostring_stream::__ostream_type&
ostring_stream::operator<<(const std::string& __s)
{
	(osstr->s) << __s;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(char* __s)
{
	(osstr->s) << __s;
	return (*this);
}

ostring_stream::__ostream_type&
ostring_stream::operator<<(const char* __s)
{
	(osstr->s) << __s;
	return (*this);
}

} //namespace notemplate
} //namespace log4cplus
