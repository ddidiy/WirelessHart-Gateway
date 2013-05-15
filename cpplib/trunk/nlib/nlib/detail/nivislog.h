#ifndef NIVISLOG_H_
#define NIVISLOG_H_

#include <fstream>
#include <sstream>

#define NLOG_LOG(level, classLogger, message)\
		{\
			std::ostringstream stream; stream << message;\
			classLogger.Log(level, stream.str());\
		}

namespace nlib {

/**
 * Holds the log file & provides write metods.
 */
class File
{
public:
	static File& Instance()
	{
		static File logFile;
		return logFile;
	}

	void Open(const std::string& logPath)
	{
		file.open(logPath.c_str(), std::ios_base::app);

		//FIXME: Ciprian Sorlea: Make this to compile
		//assert("Cannot open logfile!" && file.is_open());
	}

	void Close()
	{
		file.close();
	}

	void Write(const std::string& line)
	{
		if (file.is_open())
		{
			file << line << std::endl;
		}
	}

private:
	std::ofstream file;
};

/**
 *
 */
class ClassLogger
{
public:
	ClassLogger(const std::string& logger) :
		loggerName(logger)
	{
	}

	void Log(const std::string& level, const std::string& message)
	{
		std::stringstream stream;
		stream << level;

		SYSTEMTIME time;
::		GetLocalTime(&time);
		stream << " " << time.wHour << ":" << time.wMinute
		<< ":" << time.wSecond << ":" << time.wMilliseconds;

		stream << " " << loggerName;
		stream << " - " << message;

		File::Instance().Write(stream.str());
	}
private:
	std::string loggerName;
};

}// namespace nlib


#endif /*NIVISLOG_H_*/
