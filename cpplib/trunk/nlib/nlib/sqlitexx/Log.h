#ifndef NLIB_LOG_H_ //we replace the "log.h" from nlib
#define NLIB_LOG_H_

#include <string>
#include <sstream>
#include <ostream>


//LOG_LEVEL_APP      = 1  # app log level: 4 dbg, 3 inf, 2 warn, 1 err

enum LogLevel
{
	LL_ERROR = 1,
	LL_WARN,
	LL_INFO,
	LL_DEBUG,
	LL_MAX_LEVEL
};


bool InitLogEnv(const char *pszIniFile);
bool IsLogEnabled(enum LogLevel);

void mhlog(enum LogLevel debugLevel, const std::ostream& message ) ;
void mhlog(enum LogLevel debugLevel, const char* message) ;

#define LOG_DEBUG(message) \
	mhlog(LL_DEBUG, std::stringstream().flush() <<message)

#define LOG_INFO(message) \
	mhlog(LL_INFO, std::stringstream().flush() <<message)

#define LOG_WARN(message) \
	mhlog(LL_WARN, std::stringstream().flush() <<message)

#define LOG_ERROR(message) \
	mhlog(LL_ERROR, std::stringstream().flush() <<message)

#define LOG_DEF(name) inline void __NOOP(){}
#define LOG_INFO_ENABLED() IsLogEnabled(LL_INFO)
#define LOG_DEBUG_ENABLED() IsLogEnabled(LL_DEBUG)

#endif

