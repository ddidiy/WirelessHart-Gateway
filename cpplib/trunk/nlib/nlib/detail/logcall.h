#ifndef NLIB_LOGCALL_H_
#define NLIB_LOGCALL_H_

namespace nlib {
namespace detail {

	/**
	 * @brief Used for automatically logging duration of scopped block!
	 */
	template <typename ClassLogger>
	class FunctionCall
	{
	public:
		FunctionCall(ClassLogger& logger_, const std::string& name, bool logOnlyLeave = true)
			: logger(logger_)
		{
			functionName = name;

			//startTicks = ::GetTickCount();
			if (!logOnlyLeave)
			{
				LOG_DEBUG(functionName << " BEGIN.");
			}
		}

		~FunctionCall()
		{
			//__int64 duration = (::GetTickCount() - startTicks);
			int duration = 0;

			LOG_DEBUG(functionName << " END duration=" << duration << " ms.");
		}

		ClassLogger& __Logger()
		{
			return logger;
		}

	private:
		ClassLogger& logger;
		std::string functionName;

		unsigned int startTicks;
	};

} // namespace detail
} // namespace nlib

#endif /*NLIB_LOGCALL_H_*/
