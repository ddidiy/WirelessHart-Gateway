/*
 * notemplate_stream.h
 * The purpose is to avoid size overhead of std::basic_stringstrim<char> (a gcc template instanciate)
 *  through inclusion of <nlib/log.h>.
 *
 *
 *  Created on: Jan 19, 2009
 *      Author: nicu.dascalu
 */

#ifndef NOTEMPLATE_STREAMS_H_
#define NOTEMPLATE_STREAMS_H_

#include <string>

namespace log4cplus {
namespace notemplate {

/**
 * decouples from std:ostringstream
 */
class raw_ostringstring;

class ostring_stream
{
public:
	typedef ostring_stream __ostream_type;

public:
	ostring_stream();
	~ostring_stream();

	std::string str() const;

	__ostream_type&
	operator<<(long __n);

	__ostream_type&
	operator<<(unsigned long __n);

	__ostream_type&
	operator<<(bool __n);

	__ostream_type&
	operator<<(short __n);

	__ostream_type&
	operator<<(unsigned short __n);

	__ostream_type&
	operator<<(int __n);

	__ostream_type&
	operator<<(unsigned int __n);

#ifdef _GLIBCXX_USE_LONG_LONG
	__ostream_type&
	operator<<(long long __n);

	__ostream_type&
	operator<<(unsigned long long __n);
#endif

	__ostream_type&
	operator<<(double __f);

	__ostream_type&
	operator<<(float __f);

	__ostream_type&
	operator<<(long double __f);

	__ostream_type&
	operator<<(const void* __p);


	// strings
	__ostream_type&
	operator<<(const std::string& __s);

	__ostream_type&
	operator<<(char* __s);

	__ostream_type&
	operator<<(const char* __s);

private:
	raw_ostringstring* osstr;
};

} //namespace notemplate
} //namespace log4cplus

#endif /* NOTEMPLATE_STREAMS_H_ */
