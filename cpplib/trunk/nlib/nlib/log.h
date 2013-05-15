/**
 * @brief Utility file for logging purpose.
 * Supported lggers: Log4cxx, Log4cplus, nivislog
 */

#ifndef NLIB_LOG_H_
#define NLIB_LOG_H_


#if !defined(_MSC_VER) && !defined(__noop)
	#define __noop ((void)0)
#endif



#if defined(CPPLIB_LOG_LOG4CXX_ENABLED)

	#pragma warning ( disable : 4005 4018 4244 4715 4995 4996 )

	#include <log4cxx/logger.h>
	#include <log4cxx/propertyconfigurator.h>

	#pragma warning ( default : 4005 4018 4244 4715 4995 4996 )

	using namespace log4cxx::helpers;
	using namespace log4cxx;

	#define LOG_INIT_UNITTEST()

	#define LOG_INIT(filePath)\
		PropertyConfigurator::configure(log4cxx::String(filePath));

	#define LOG_REINIT(filePath) __noop

    #define LOG_SHUTDOWN() __noop

	#define LOG_DEF(name)\
		static log4cxx::LoggerPtr __Logger()\
		{\
			static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(name);\
			return logger;\
		}

	#define LOG_DEBUG(message) LOG4CXX_DEBUG(__Logger(), message)
	#define LOG_INFO(message) LOG4CXX_INFO(__Logger(), message)
	#define LOG_WARN(message) LOG4CXX_WARN(__Logger(), message)
	#define LOG_ERROR(message) LOG4CXX_ERROR(__Logger(), message)
	#define LOG_FATAL(message) LOG4CXX_FATAL(__Logger(), message)


#elif defined(CPPLIB_LOG_LOG4CPLUS_ENABLED)

	#ifndef LOG4CPLUS_STATIC
		#define LOG4CPLUS_STATIC
	#endif

	#ifndef HAVE_SSTREAM
  	#define HAVE_SSTREAM
	#endif

	#include <log4cplus/logger.h>
	#include <log4cplus/configurator.h>
	#include <log4cplus/hierarchylocker.h> //used by reinit log configuration

#ifdef _MSC_VER
	#ifdef _DEBUG
		//#ifndef UNICODE
			#pragma comment( lib, "liblog4cplus-vc80-mt-sgd-1_0_2.lib" )
		//#endif
	#else
		#pragma comment( lib, "liblog4cplus-vc80-mt-s-1_0_2.lib" )
	#endif
#endif // _MSC_VER

	#define LOG_INIT_UNITTEST()\
		log4cplus::BasicConfigurator().configure();

	#define LOG_INIT(filePath)\
		log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_STRING_TO_TSTRING(filePath));

	#define LOG_REINIT(filePath)\
	log4cplus::Hierarchy &h=log4cplus::Logger::getDefaultHierarchy();\
	h.disableAll();\
	h.shutdown();\
	log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_STRING_TO_TSTRING(filePath));\
	h.enableAll();

    #define LOG_SHUTDOWN()\
        log4cplus::Logger::shutdown();

	#define LOG_DEF(name)\
		static log4cplus::Logger& __Logger()\
		{\
			static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(name));\
			return logger;\
		}

	#define LOG_DEF_BY_CLASS(class_)\
		static log4cplus::Logger& __Logger()\
		{\
			return class_::__Logger();\
		}


	#ifdef _DEBUG
		#define LOG_TRACE_ENABLED() __Logger().isEnabledFor(log4cplus::TRACE_LOG_LEVEL)
		#define LOG_TRACE(message) \
			do { \
				log4cplus::Logger& logger = __Logger(); \
				if(logger.isEnabledFor(log4cplus::TRACE_LOG_LEVEL)) { \
					log4cplus::tostringstream _log4cplus_buf; \
					_log4cplus_buf << message; \
					logger.forcedLog(log4cplus::TRACE_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
				} \
			} while(0);

	#else
		#define LOG_TRACE_ENABLED() false
		#define LOG_TRACE(message) __noop
	#endif

	#define LOG_DEBUG_ENABLED() __Logger().isEnabledFor(log4cplus::DEBUG_LOG_LEVEL)
	#define LOG_DEBUG(message) \
		do { \
			log4cplus::Logger& logger = __Logger(); \
			if(logger.isEnabledFor(log4cplus::DEBUG_LOG_LEVEL)) { \
				log4cplus::tostringstream _log4cplus_buf; \
				_log4cplus_buf << message; \
				logger.forcedLog(log4cplus::DEBUG_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
			} \
		} while(0);


	#define LOG_INFO_ENABLED() __Logger().isEnabledFor(log4cplus::INFO_LOG_LEVEL)
	#define LOG_INFO(message) \
			do { \
				log4cplus::Logger& logger = __Logger(); \
				if(logger.isEnabledFor(log4cplus::INFO_LOG_LEVEL)) { \
					log4cplus::tostringstream _log4cplus_buf; \
					_log4cplus_buf << message; \
					logger.forcedLog(log4cplus::INFO_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
				} \
			} while(0);

	#define LOG_WARN_ENABLED() __Logger().isEnabledFor(log4cplus::WARN_LOG_LEVEL)
	#define LOG_WARN(message) \
			do { \
				log4cplus::Logger& logger = __Logger(); \
				if(logger.isEnabledFor(log4cplus::WARN_LOG_LEVEL)) { \
					log4cplus::tostringstream _log4cplus_buf; \
					_log4cplus_buf << message; \
					logger.forcedLog(log4cplus::WARN_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
				} \
			} while(0);

	#define LOG_ERROR_ENABLED() __Logger().isEnabledFor(log4cplus::ERROR_LOG_LEVEL)
	#define LOG_ERROR(message) \
			do { \
				log4cplus::Logger& logger = __Logger(); \
				if(logger.isEnabledFor(log4cplus::ERROR_LOG_LEVEL)) { \
					log4cplus::tostringstream _log4cplus_buf; \
					_log4cplus_buf << message; \
					logger.forcedLog(log4cplus::ERROR_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
				} \
			} while(0);

	#define LOG_FATAL_ENABLED() __Logger().isEnabledFor(log4cplus::FATAL_LOG_LEVEL)
	#define LOG_FATAL(message) \
			do { \
				log4cplus::Logger& logger = __Logger(); \
				if(logger.isEnabledFor(log4cplus::FATAL_LOG_LEVEL)) { \
					log4cplus::tostringstream _log4cplus_buf; \
					_log4cplus_buf << message; \
					logger.forcedLog(log4cplus::FATAL_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
				} \
			} while(0);

	#define LOG_CALL(methodName) nivis::detail::FunctionCall<log4cplus::Logger> call(__Logger(), methodName);

	namespace log4cplus
	{
		class ReloadPropertyConfigurator : public PropertyConfigurator
		{
		public:
			static void doConfigure(const tstring& file)
			{
				ReloadPropertyConfigurator(file).reconfigure();
			}
		private:
			ReloadPropertyConfigurator(const tstring& file) : PropertyConfigurator(file), lock(NULL)
			{
			}

			void reconfigure()
			{
				// Lock the Hierarchy
				HierarchyLocker theLock(h);
				lock = &theLock;
				// reconfigure the Hierarchy
				theLock.resetConfiguration();
				reconfigure();

				// release the lock
				lock = NULL;
			}

			virtual Logger getLogger(const log4cplus::tstring& name)
			{
				if(lock)
				{
					return lock->getInstance(name);
				}
				else
				{
					return PropertyConfigurator::getLogger(name);
				}
			}
			virtual void addAppender(Logger &logger, log4cplus::SharedAppenderPtr& appender)
			{

				if(lock)
				{
					lock->addAppender(logger, appender);
				}
				else
				{
					PropertyConfigurator::addAppender(logger, appender);
				}
			}

		private:
			HierarchyLocker* lock;
		};
	}

#elif defined(CPPLIB_LOG_NIVIS_ENABLED)

	#include "detail/nivislog.h"

	#define LOG_INIT_UNITTEST()
	#define LOG_INIT(filePath)\
		nivis::log::File::Instance().Open(filePath);

	#define LOG_REINIT(filePath) __noop

    #define LOG_SHUTDOWN() __noop

	#define LOG_DEF(name)\
		static nivis::log::ClassLogger& __Logger()\
		{\
			static nivis::log::ClassLogger logger(name);\
			return logger;\
		}

	#define LOG_TRACE(message) NLOG_LOG("TRACE", __Logger(), message)
	#define LOG_DEBUG(message) NLOG_LOG("DEBUG", __Logger(), message)
	#define LOG_INFO(message) NLOG_LOG("INFO", __Logger(), message)
	#define LOG_WARN(message) NLOG_LOG("WARN", __Logger(), message)
	#define LOG_ERROR(message) NLOG_LOG("ERROR", __Logger(), message)
	#define LOG_FATAL(message) NLOG_LOG("FATAL", __Logger(), message)

	#define LOG_CALL(methodName) nivis::detail::FunctionCall<nlib::ClassLogger> call(__Logger(), methodName);

#elif defined(ACCESS_NODE_LOG_ENABLED)

	#include <string>
	#include <sstream>
	#include <iostream>
	#include <map>
	
	enum LogLevel
	{
		LL_DEBUG = 4,
		LL_INFO = 3,
		LL_WARN = 2,
		LL_ERROR = 1
	};
	extern void EnableLog(int);
	extern bool IsLogEnabled(int);
	extern void mhlog(enum LogLevel level, const std::ostream& message);
	extern void mhlog(enum LogLevel level, const char* message);

	extern void EnableLog_APP(int);
	extern void mhlog_APP(enum LogLevel level, const std::ostream& message);
	extern void mhlog_APP(enum LogLevel level, const char* message);
	extern bool IsLogEnabled_APP(int);

	extern void SetLogFile(std::string);
	

	#define LOG_INIT(filePath) SetLogFile(filePath)
    #define LOG_REINIT(filePath) SetLogFile(filePath)
    #define LOG_SHUTDOWN() __noop

	#define LOG_DEF(name) inline void __NOOP(){}
	#define LOG_DEF_BY_CLASS(class_) LOG_DEF(class_)

	#define LOG_TRACE_ENABLED() IsLogEnabled(LL_TRACE)
	#define LOG_TRACE(message) __noop
		
	#define LOG_DEBUG_ENABLED() IsLogEnabled(LL_DEBUG)
	#define LOG_DEBUG(message) \
		do	\
		{	\
			if(IsLogEnabled(LL_DEBUG))		\
			{	mhlog(LL_DEBUG, std::stringstream().flush() <<message);	\
			}	\
		}while(0)

	#define LOG_INFO_ENABLED() IsLogEnabled(LL_INFO)
	#define LOG_INFO(message) \
		do	\
		{	\
			if(IsLogEnabled(LL_INFO))		\
			{	mhlog(LL_INFO, std::stringstream().flush() <<message);	\
			}	\
		}while(0)

	#define LOG_WARN_ENABLED() IsLogEnabled(LL_WARN)
	#define LOG_WARN(message) \
		do	\
		{	if(IsLogEnabled(LL_WARN))		\
		{	mhlog(LL_WARN, std::stringstream().flush() <<message);	\
		}	\
		}while(0)

	#define LOG_ERROR_ENABLED() IsLogEnabled(LL_ERROR)
	#define LOG_ERROR(message) \
		do	\
		{	\
			if(IsLogEnabled(LL_ERROR))		\
			{	mhlog(LL_ERROR, std::stringstream().flush() <<message);	\
			}	\
		}while(0)

	#define LOG_FATAL_ENABLED() IsLogEnabled(LL_FATAL)
	#define LOG_FATAL(message) __noop

	#define LOG_CALL(methodName) __noop

   #define LOG_SHUTDOWN_APP() __noop

	#define LOG_DEF_APP(name) inline void __NOOP(){}
	#define LOG_DEF_BY_CLASS_APP(class_) LOG_DEF_APP(class_)

	#define LOG_TRACE_ENABLED_APP() IsLogEnabled_APP(LL_TRACE)
	#define LOG_TRACE_APP(message) __noop
		
	#define LOG_DEBUG_ENABLED_APP() IsLogEnabled_APP(LL_DEBUG)
	#define LOG_DEBUG_APP(message) 			\
		do	\
		{	\
			if(IsLogEnabled_APP(LL_DEBUG))		\
			{	mhlog_APP(LL_DEBUG, std::stringstream().flush() <<message);	\
			}	\
		}while(0)

	#define LOG_INFO_ENABLED_APP() IsLogEnabled_APP(LL_INFO)
	#define LOG_INFO_APP(message) \
		do	\
		{	\
			if(IsLogEnabled_APP(LL_INFO))		\
			{	mhlog_APP(LL_INFO, std::stringstream().flush() <<message);	\
			}	\
		}while(0)

	#define LOG_WARN_ENABLED_APP() IsLogEnabled_APP(LL_WARN)
	#define LOG_WARN_APP(message) \
		do	\
		{	\
			if(IsLogEnabled_APP(LL_WARN))		\
			{	mhlog_APP(LL_WARN, std::stringstream().flush() <<message);	\
			}	\
		}while(0)

	#define LOG_ERROR_ENABLED_APP() IsLogEnabled_APP(LL_ERROR)
	#define LOG_ERROR_APP(message) \
		do	\
		{	\
			if(IsLogEnabled_APP(LL_ERROR))		\
			{	mhlog_APP(LL_ERROR, std::stringstream().flush() <<message);	\
			}	\
		}while(0)

	#define LOG_FATAL_ENABLED_APP() IsLogEnabled_APP(LL_FATAL)
	#define LOG_FATAL_APP(message) __noop

	#define LOG_CALL_APP(methodName) __noop

	
#else // log disabled

	#define LOG_INIT(filePath) __noop
    #define LOG_REINIT(filePath) __noop
    #define LOG_SHUTDOWN() __noop

	#define LOG_DEF(name) inline void __NOOP(){}
	#define LOG_DEF_BY_CLASS(class_) LOG_DEF(class_)

	#define LOG_TRACE_ENABLED() false
	#define LOG_TRACE(message) __noop

	#define LOG_DEBUG_ENABLED() false
	#define LOG_DEBUG(message) __noop

	#define LOG_INFO_ENABLED() false
	#define LOG_INFO(message) __noop

	#define LOG_WARN_ENABLED() false
	#define LOG_WARN(message) __noop

	#define LOG_ERROR_ENABLED() false
	#define LOG_ERROR(message) __noop

	#define LOG_FATAL_ENABLED() false
	#define LOG_FATAL(message) __noop

	#define LOG_CALL(methodName) __noop

#endif //LOG_DEFINED



#endif /*NLIB_LOG_H_*/
