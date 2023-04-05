#ifndef __artd_Logger_h
#define __artd_Logger_h

// define exports for dynamic libraries
// This is here because we want to be able to use the logger stand alone with no other artd code.

#ifndef ARTD_SHARED_LIBRARY_EXPORT
    #ifdef _MSC_VER
        #define ARTD_SHARED_LIBRARY_EXPORT __declspec(dllexport)
        #define ARTD_SHARED_LIBRARY_IMPORT __declspec(dllimport)
    #else
        #define ARTD_SHARED_LIBRARY_EXPORT __attribute ((visibility ("default")))
        #define ARTD_SHARED_LIBRARY_IMPORT
    #endif
#endif // ARTD_SHARED_LIBRARY_EXPORT

#ifdef BUILDING_artd_lib_logger
	#define ARTD_API_LIB_LOGGER ARTD_SHARED_LIBRARY_EXPORT
#else
	#define ARTD_API_LIB_LOGGER ARTD_SHARED_LIBRARY_IMPORT
#endif

#include <sstream>
#include <ostream>

// Note: to compile in logging for a level make sure 
// the item for the level below is NOT defined.
// If it is defined it must be defined as AD_LOG_NOOP

#define AD_ENABLE_FULL_LOGGING

#ifndef ARTD_DEBUG // release compile

#ifndef AD_ENABLE_FULL_LOGGING

#define AD_LOG_trace   AD_LOG_NOOP
#define AD_LOG_info    AD_LOG_NOOP
#define AD_LOG_debug   AD_LOG_NOOP
#define AD_LOG_warning AD_LOG_NOOP

#endif // AD_ENABLE_FULL_LOGGING

// for now we leave these serious levels here for the release compile
// #define AD_LOG_error   AD_LOG_NOOP
//#define AD_LOG_fatal   AD_LOG_NOOP

#else // debug compile 

#endif


namespace artd {

class Logger {
protected:
	Logger();
public:
	enum log_level {
		trace = 0,
		debug,
		info,
		warning,
		error,
		fatal,
		print,
		maxLevel = print
	};
protected:
	log_level activeLevel_;
public:
	inline bool isActive(log_level level) {
		return(level >= activeLevel_);
	}
};

class LogManager {
public:
	static ARTD_API_LIB_LOGGER void init(int levelToActivate, const char* logFilePath = nullptr);

	static ARTD_API_LIB_LOGGER void init(const char* levelName, const char* logFilePath = nullptr);

	static inline void init(const std::string& levelToActivate, const std::string& logFilePath) {
		init(levelToActivate.c_str(), logFilePath.c_str());
	}

	static ARTD_API_LIB_LOGGER void enableStdOut(bool on);
	static ARTD_API_LIB_LOGGER Logger* getLogger(const char* key);
};


class LineLogger
	: protected ::std::stringstream
{
	Logger::log_level level_;
	const char* methodPath_;
	const char* file_;
	Logger& logger_;
	int line_;

	ARTD_API_LIB_LOGGER ::std::ostream& _log_(Logger::log_level level, const char* methodPath, const char* file, int line);
	ARTD_API_LIB_LOGGER ::std::ostream& _log_();  // just prints input directly to log

public:


	inline LineLogger(Logger& logger) : level_((Logger::log_level)0), methodPath_(nullptr), file_(nullptr), logger_(logger), line_(0) {} // for now 

	ARTD_API_LIB_LOGGER ~LineLogger();

	inline std::ostream& log_trace(const char* methodPath, const char* file, int line) {
		return(_log_(Logger::log_level::trace, methodPath, file, line));
	}
	inline std::ostream& log_debug(const char* methodPath, const char* file, int line) {
		return(_log_(Logger::log_level::debug, methodPath, file, line));
	}
	inline std::ostream& log_info(const char* methodPath, const char* file, int line) {
		return(_log_(Logger::log_level::info, methodPath, file, line));
	}
	inline std::ostream& log_warning(const char* methodPath, const char* file, int line) {
		return(_log_(Logger::log_level::warning, methodPath, file, line));
	}
	inline std::ostream& log_error(const char* methodPath, const char* file, int line) {
		return(_log_(Logger::log_level::error, methodPath, file, line));
	}
	inline std::ostream& log_fatal(const char* methodPath, const char* file, int line) {
		return(_log_(Logger::log_level::fatal, methodPath, file, line));
	}
	inline std::ostream& log_print() {
		return(_log_());
	}

	class NoopStream {
	public:
		template<class AnyObj>
		inline NoopStream& operator << (const AnyObj& v) {
			return(*this);
		}
		inline NoopStream& endl(NoopStream& os) {
			return(*this);
		}
	};


	inline static NoopStream& log_NOOP() {
		static NoopStream  noopStream;
		return(noopStream);
	}

};


// TODO this needs to be set up so even with "<<" or a "+" concatenation no code will be generated in the unused case ? possible ? PK
// Otherwise might need to use a printf style argument system where all items are arguments and can have an empty macro so as not to 
// execute formatting in the release version.

// use the below macro like any ostream AD_LOG(info) << "a message, value = " << value;



// This one is only used at present for the GLError wrapper
#define AD_LOG_MFL(level,method,file,line) \
	if(_logger_().isActive(::artd::Logger::log_level::level)) \
		::artd::LineLogger(_logger_()).log_##level(method,file,line)

#define AD_LOG_OUT(level) \
	::artd::LineLogger(_logger_()).log_##level(&__PRETTY_FUNCTION__[0],&__FILE__[0],__LINE__)

#define AD_REALLY_LOG(level) \
	if(_logger_().isActive(::artd::Logger::log_level::level)) \
		::artd::LineLogger(_logger_()).log_##level(&__PRETTY_FUNCTION__[0],&__FILE__[0],__LINE__)

#define AD_LOG_NOOP if(false) ::artd::LineLogger::NoopStream()

#ifndef AD_LOG_trace
#define AD_LOG_trace   AD_REALLY_LOG(trace)
#endif

#ifndef AD_LOG_info
#define AD_LOG_info    AD_REALLY_LOG(info)
#endif

#ifndef AD_LOG_debug
#define AD_LOG_debug   AD_REALLY_LOG(debug)
#endif

#ifndef AD_LOG_warning
#define AD_LOG_warning AD_REALLY_LOG(warning)
#endif

#ifndef AD_LOG_error
#define AD_LOG_error   AD_REALLY_LOG(error)
#endif

#ifndef AD_LOG_fatal
#define AD_LOG_fatal   AD_REALLY_LOG(fatal)
#endif

#ifndef AD_LOG_print
#define AD_LOG_print \
    ::artd::LineLogger(_logger_()).log_print()
#endif

// this is what is actually used in the code  like 
// AD_LOG(error) << "text" << " for" << " a message";

#define AD_LOG(level) AD_LOG_##level

#if 0
inline Logger& _logger_() {
	static Logger* mylog = LogManager::getLogger(&__FUNCTION__[0]);
	return(*mylog);
}
#endif

}  // namespace artd

inline ::artd::Logger& _logger_() {
	static ::artd::Logger* mylog = ::artd::LogManager::getLogger(&__FUNCTION__[0]);
	return(*mylog);
}

#endif // __artd_Logger_h
