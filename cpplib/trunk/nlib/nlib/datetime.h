#ifndef NLIB_DATETIME_H_
#define NLIB_DATETIME_H_ 

#include <string>
#include <stdexcept>

#include <boost/date_time/gregorian/gregorian_types.hpp> //no i/o just types
#include <boost/date_time/posix_time/posix_time_types.hpp> //no i/o just types

#include <boost/date_time/time_clock.hpp>
#include <boost/format.hpp>


namespace nlib { 
	// alais namespace as util for: util::seconds(3), and so on...
	namespace util = boost::posix_time;

	typedef boost::posix_time::ptime DateTime;
	typedef boost::posix_time::time_duration TimeSpan;
	const TimeSpan NOTIME(0, 0, 0);	
	

	/**
	 * @brief Convert a DateTime object into string (in ISO Time Format)
	 * @param dateTime - Date time in ISO format (e.g. 29 Feb 2007 14:35:34  -> 20070229T143534)
	 * @throw std::invalid_argument if the string is bad formated.
	 */
//	inline
//	std::string ToISOString(const DateTime& dateTime)
//	{
//		return boost::posix_time::to_iso_string_type<std::string::value_type>(dateTime);
//	}

	/**
	 * @brief Convert a string (in ISO Time Format) into the Time object.
	 * @param dateTime - Date time in ISO format (e.g. 20070229T143534 -> 29 Feb 2007 14:35:34 )
	 * @throw std::invalid_argument if the string is bad formated.
	 */
//	inline
//	DateTime FromISOString(const std::string& dateTime)
//	{
//		try
//		{
//			if ("not-a-date-time" == dateTime)
//				return DateTime();
//
//			return boost::posix_time::from_iso_string(dateTime);
//		}
//		catch (std::exception&)
//		{
//			std::ostringstream stream;
//			stream << "The provided string='" << dateTime << "' is not a valid ISO DateTime format!";
//			throw std::invalid_argument(stream.str());
//		}
//	}

	/**
	 * get current local time 
	 */ 
	inline 
	DateTime CurrentLocalTime()
	{
		return boost::date_time::second_clock<DateTime>::local_time();
	}

	/**
	 * get current universal time
	 */ 
	inline 
	DateTime CurrentUniversalTime()
	{
		return boost::date_time::second_clock<DateTime>::universal_time();
	}

	/**
	 * create a date time from components.
	 */ 
	inline 
	DateTime CreateTime(int year, int month, int day, int hour, int minute, int second)
	{
		DateTime::date_type date(year, month, day);
		DateTime::time_duration_type time(hour, minute, second);
		
		return DateTime(date, time);
	}

	/**
	 * @brief Convert a DateTime object into a string format.
	 * e.g. 20/Jan/2007 10:34
	 */
	
	inline
	std::string ToString(const DateTime& dateTime)
	{
		if(DateTime() == dateTime)
		{
			return "no time";
		}
		//HACK:[Ovidiu] - to avoid representing month as string
		int month = (int)dateTime.date().month();
		return boost::str(
				boost::format("%1$04d-%2$02d-%3$02d %4$02d:%5$02d:%6$02d")
				% dateTime.date().year()
				% month
				% dateTime.date().day()
				% dateTime.time_of_day().hours()
				% dateTime.time_of_day().minutes()
				% dateTime.time_of_day().seconds()
		);
	}
	
	inline
		std::string ToString(const TimeSpan& timeSpan)
	{
		return boost::str(
						boost::format("%1$02d:%2$02d:%3$02d")						
						% timeSpan.hours()
						% timeSpan.minutes()
						% timeSpan.seconds()
						);
	}

} // namespace nlib

#endif /*NLIB_DATEDIME_H_H*/
