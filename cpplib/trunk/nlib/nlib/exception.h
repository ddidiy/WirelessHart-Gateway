#ifndef NLIB_EXCEPTION_H_
#define NLIB_EXCEPTION_H_

#include <exception>
#include <sstream>
#include <string>

namespace nlib {

class Exception;

namespace detail {
void InitException(Exception& ex, const char* exceptionClass, const char* file, const int line);
}// namespace detail


/**
 * @brief The base class for all exceptions.
 */
class Exception : public std::exception
{
public:
	Exception() throw()
	{
	}
	
	Exception(const std::string& message_) throw() :
		message(message_)
	{
	}

	Exception(const std::string& message_, Exception& innerException) throw() :
		message(message_), whatStr(innerException.whatStr)
	{
	}

	~Exception() throw()
	{
	}

	virtual const char* what() const throw()
	{
		return whatStr.c_str();
	}

	const std::string& Message() const throw()
	{
		return message;
	}

private:
	std::string message;
	std::string whatStr;

	friend void detail::InitException(Exception& ex, const char* exceptionClass, const char* file, const int line);

};

class ArgumentException : public Exception
{
public:
	ArgumentException(const std::string& p_rMmessage) : Exception(p_rMmessage)
	{
	}
};


namespace detail {
inline void InitException(Exception& ex, const char* exceptionClass, const char* thrownInFile, int thrownOnLine)
{
	std::ostringstream stream;
	stream << ex.Message() << " EXCEPTION='" << exceptionClass << "'" << " THROWN ON [" << thrownInFile << ":"
			<< thrownOnLine << "]";

	if (!ex.whatStr.empty())
	{
		stream << std::endl;
		stream << ex.whatStr;
	}

	ex.whatStr = stream.str();
}
}// namespace detail

#define THROW_EXCEPTION0(ExceptionClass) \
	{ExceptionClass ex; nlib::detail::InitException(ex, #ExceptionClass, __FILE__, __LINE__); throw ex;}

#define THROW_EXCEPTION1(ExceptionClass, Param1) \
	{ExceptionClass ex(Param1); nlib::detail::InitException(ex, #ExceptionClass, __FILE__, __LINE__); throw ex;}

#define THROW_EXCEPTION2(ExceptionClass, Param1, Param2) \
	{ExceptionClass ex(Param1, Param2); nlib::detail::InitException(ex, #ExceptionClass, __FILE__, __LINE__); throw ex;}

#define THROW_EXCEPTION3(ExceptionClass, Param1, Param2, Param3) \
	{ExceptionClass ex(Param1, Param2, Param3); nlib::detail::InitException(ex, #ExceptionClass, __FILE__, __LINE__); throw ex;}

} // namespace nlib

#endif /*NLIB_EXCEPTION_H_*/
