#define __artd_Logger_cpp 1

#include "artd/Logger.h"
#include <string>
#include <mutex>

#include <iostream>
#include <iomanip>
#include <fstream>

#include <unordered_map>

#if defined (_MSC_VER) && (_DEBUG)
#include <windows.h> // windows header for debug prints
#endif

namespace artd {

Logger::Logger() : activeLevel_(log_level::info) {

}

static const char* levelName(unsigned int level) {
	switch (level) {
		case Logger::log_level::trace:
			return("TRACE");
		case Logger::log_level::debug:
			return("DEBUG");
		case Logger::log_level::info:
			return("INFO");
		case Logger::log_level::warning:
			return("WARN");
		case Logger::log_level::error:
			return("ERROR");
		case Logger::log_level::fatal:
			return("FATAL");
		default:
			return("UNKNOWN");
	}
}

std::ostream& LineLogger::_log_() {
	level_ = Logger::log_level::maxLevel;
	return(*this);
}

std::ostream& LineLogger::_log_(Logger::log_level level, const char* methodPath, const char* file, int line)
{
	methodPath_ = methodPath;
	file_ = file;
	line_ = line;
	level_ = level;
	return(*this);
}



// ***** LogManager things
namespace loggerns {

class LoggerImpl
	: public Logger
{
public:
	std::string mKey;
	LoggerImpl(const char* key) : mKey(key) {

	}
	void setActiveLevel(Logger::log_level level) {
		activeLevel_ = level;
	}
};

class LogManagerImpl
	: public LogManager
{
	LogManagerImpl() {
		printf("Creating C++ Logger\n");
	}
public:

	static inline LogManagerImpl& instance() {
		static LogManagerImpl me;
		return(me);
	}

	// For now this has no possibility of recursion.
	// std::recursive_mutex msgLock;

	std::mutex msgLock;

	Logger::log_level globalLevel = Logger::log_level::info;
	std::ofstream logStream_;
	std::string logPath = "";

	std::unordered_map<std::string, LoggerImpl*> mLoggers;

	LoggerImpl* getLogger(const char* key) {
		auto found = mLoggers.find(key);

		if (found == mLoggers.end()) {
			LoggerImpl* newLogger = new LoggerImpl(key);

			newLogger->setActiveLevel(globalLevel);
			mLoggers.insert(std::make_pair(newLogger->mKey, newLogger));
			return(newLogger);
		}
		return(found->second);
	}
	void init(int levelToActivate, const char* logFilePath) {

		if (logFilePath != nullptr) {
			logPath = logFilePath;
			logStream_.open(logPath);
			if (!logStream_.is_open()) {
				std::cerr << "unable to open log file: " << logPath;
			}
		}
		globalLevel = (Logger::log_level)levelToActivate;

		for (auto it = mLoggers.begin(); it != mLoggers.end(); ++it) {
			LoggerImpl* logger = it->second;
			logger->setActiveLevel(globalLevel);

		}
	}
	bool stdoutEnabled = true;

	void enableStdOut(bool on) {
		stdoutEnabled = on;
	}
};

} // end loggerns

using namespace loggerns;

LineLogger::~LineLogger() {

	::std::string ss = this->str();

	const char* pFileName = nullptr;
	if (file_) {
		pFileName = ::strrchr(file_, '\\');
	}

	LogManagerImpl& lm = LogManagerImpl::instance();

#if defined (_MSC_VER) && (_DEBUG)
	if (::IsDebuggerPresent())
	{
		char formatBuff[2048];
		int len;
		if (file_) {
			len = snprintf(formatBuff, 2046, "%s(%d): %s \n     %s : ", file_, line_, levelName(level_), methodPath_);
		}
		else { // log(print) case just print the message part.
			formatBuff[0] = 0;
			len = 0;
		}

		const std::lock_guard<std::mutex> lock(lm.msgLock);

		::OutputDebugStringA(formatBuff);
		::OutputDebugStringA(ss.c_str());
		::OutputDebugStringA("\n");
	}
	else {


	}
#else
	if (file_ && pFileName == nullptr) {
		pFileName = ::strrchr(file_, '/');
	}
#endif

	if (pFileName == nullptr) {
		pFileName = file_;
	}
	else {
		++pFileName;
	}

	const char* cstr = ss.c_str();

	{
		const std::lock_guard<std::mutex> lock(lm.msgLock);

		if (!file_) {  // the log print case !!
			std::cout << cstr << std::endl;
			if (lm.logStream_.is_open()) {
				lm.logStream_ << cstr << std::endl;
			}
		}
		else {
			if (lm.logStream_.is_open()) {
				lm.logStream_ << levelName(level_) << " " << pFileName << "(" << line_ << ") " << methodPath_ << " : " << cstr << std::endl;
				if (level_ >= Logger::log_level::warning) {
					lm.logStream_.flush(); // TODO: too slow ???
				}
			}
			if (lm.stdoutEnabled) {
            #if 1 // intellij clickable format
                std::cout << levelName(level_) << " " << methodPath_ << " : " << cstr << " (" << file_ << ":" << line_ << ")" << std::endl;
            #else
                std::cout << levelName(level_) << " " << pFileName << "(" << line_ << ") " << methodPath_ << " : " << cstr << std::endl;
            #endif
			}
		}
	}
}


Logger* LogManager::getLogger(const char* key) {
	return(loggerns::LogManagerImpl::instance().getLogger(key));
}

static int levelNameToLevel(const char* levelName) {

	// TODO: more elegant way to do this
	static std::string traceName = "trace";
	static std::string debugName = "debug";
	static std::string infoName = "info";
	static std::string warningName = "warning";
	static std::string errorName = "error";
	static std::string fatalName = "fatal";

	if (traceName == levelName) {
		return(Logger::log_level::trace);
	}
	else if (debugName == levelName) {
		return(Logger::log_level::debug);
	}
	else if (infoName == levelName) {
		return(Logger::log_level::info);
	}
	else if (warningName == levelName) {
		return(Logger::log_level::warning);
	}
	else if (errorName == levelName) {
		return(Logger::log_level::error);
	}
	else if (fatalName == levelName) {
		return(Logger::log_level::fatal);
	}
	else {
		return(Logger::log_level::info);
	}
}

void LogManager::init(int levelToActivate, const char* logFilePath) {
	if (logFilePath != nullptr && logFilePath[0] == '0') {
		logFilePath = nullptr;
	}
	loggerns::LogManagerImpl::instance().init(levelToActivate, logFilePath);
}

void LogManager::init(const char* levelToActivate, const char* logFilePath) {
	init(levelNameToLevel(levelToActivate), logFilePath);
}
void LogManager::enableStdOut(bool on) {
	loggerns::LogManagerImpl::instance().enableStdOut(on);
}

} // namespace artd