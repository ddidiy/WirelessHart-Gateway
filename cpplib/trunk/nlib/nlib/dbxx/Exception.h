#ifndef NLIB_DBXX_EXCEPTION_H_
#define NLIB_DBXX_EXCEPTION_H_

#include <nlib/exception.h>

namespace nlib {
namespace dbxx {

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

}// namespace dbxx
}// namspace nlib

#endif /*NLIB_DBXX_EXCEPTION_H_*/
