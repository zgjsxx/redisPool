#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <mutex>

class ServerLog
{
public:
	ServerLog();
	~ServerLog();
public:
	static ServerLog* get();
public:
	void WriteFormatDebugLog(std::string file, std::string function, int line, const char* lpszFormat,...);
	void WriteFormatInfoLog(std::string file, std::string function, int line, const char* lpszFormat,...);
	void WriteFormatWarnLog(std::string file, std::string function, int line, const char* lpszFormat,...);
	void WriteFormatErrorLog(std::string file, std::string function, int line, const char* lpszFormat,...);

private:
	static ServerLog *sm_serverLogPtr;
	log4cxx::LoggerPtr m_infoLogger;
	log4cxx::LoggerPtr m_errorLogger;
	std::mutex m_mtx;
};

#define LOG_DEBUG(...) ServerLog::get()->WriteFormatDebugLog(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) ServerLog::get()->WriteFormatInfoLog(__FILE__, __FUNCTION__, __LINE__,__VA_ARGS__)
#define LOG_WARN(...) ServerLog::get()->WriteFormatWarnLog(__FILE__, __FUNCTION__, __LINE__,__VA_ARGS__)
#define LOG_ERROR(...) ServerLog::get()->WriteFormatErrorLog(__FILE__, __FUNCTION__, __LINE__,__VA_ARGS__)
