#include <string>
#include <iostream>

#include "ServerLog.h"

ServerLog* ServerLog::sm_serverLogPtr = nullptr;

ServerLog::ServerLog()
{
	log4cxx::PropertyConfigurator::configure("log4cxx.properties");
	m_infoLogger = (log4cxx::Logger::getLogger("server"));
	m_errorLogger = (log4cxx::Logger::getLogger("server.error"));
}

ServerLog::~ServerLog()
{

}

ServerLog* ServerLog::get()
{
    if(sm_serverLogPtr == nullptr)
    {
        sm_serverLogPtr = new ServerLog();
    }
    return sm_serverLogPtr;
}


void ServerLog::WriteFormatDebugLog(std::string file, std::string function, int line,const char* lpszFormat, ...)
{
    std::lock_guard<std::mutex> lck(m_mtx);
    char buf[32];
    snprintf(buf,sizeof(buf), "%d", line);
    if(!file.empty())
    {
        std::string::size_type pos = file.find_last_of('/');
        file = file.substr(pos+1);
    }
    std::string temp_str = "  @<" + function + "> " + "@@<" + file+ ":" + std::string(buf)+">";

    va_list args;
    va_start(args, lpszFormat);
    char szBuffer[8196];
    vsprintf(szBuffer, lpszFormat, args);
    va_end(args);

    std::string final_str = szBuffer;
    final_str = final_str + temp_str;
    m_infoLogger->debug(final_str);
}

void ServerLog::WriteFormatInfoLog(std::string file, std::string function, int line,const char* lpszFormat, ...)
{
    std::lock_guard<std::mutex> lck(m_mtx);
    char buf[32];
    snprintf(buf,sizeof(buf),"%d",line);
    if(!file.empty())
    {
        std::string::size_type pos = file.find_last_of('/');
        file = file.substr(pos+1);
    }
    std::string temp_str = "  @<" + function + "> " + "@@<" + file+ ":" + std::string(buf)+">";
    va_list args;
    va_start(args, lpszFormat);
    char szBuffer[8196];
    vsprintf(szBuffer, lpszFormat, args);
    va_end(args);

    std::string final_str = szBuffer;
    final_str = szBuffer + temp_str;

    m_infoLogger->info(final_str);
}

void ServerLog::WriteFormatWarnLog(std::string file, std::string function, int line, const char* lpszFormat, ...)
{
    std::lock_guard<std::mutex> lck(m_mtx);
    char buf[32];
    snprintf(buf,sizeof(buf),"%d",line);
    if(!file.empty())
    {
        std::string::size_type pos = file.find_last_of('/');
        file = file.substr(pos+1);
    }
    std::string temp_str = "  @<" + function + "> " + "@@<" + file+ ":" + std::string(buf)+">";
    va_list args;
    va_start(args, lpszFormat);
    char szBuffer[8196];
    vsprintf(szBuffer, lpszFormat, args);
    va_end(args);

    std::string final_str = szBuffer;
    final_str = szBuffer + temp_str;

    m_errorLogger->warn(szBuffer);

}

void ServerLog::WriteFormatErrorLog(std::string file, std::string function, int line, const char* lpszFormat, ...)
{
    std::lock_guard<std::mutex> lck(m_mtx);
    char buf[32];
    snprintf(buf,sizeof(buf),"%d",line);
    if(!file.empty())
    {
        std::string::size_type pos = file.find_last_of('/');
        file = file.substr(pos+1);
    }
    std::string temp_str = "  @<" + function + "> " + "@@<" + file+ ":" + std::string(buf)+">";
    va_list args;
    va_start(args, lpszFormat);
    char szBuffer[8196];
    vsprintf(szBuffer, lpszFormat, args);
    va_end(args);

    std::string final_str = szBuffer;
    final_str = szBuffer + temp_str;    
    
    m_errorLogger->error(szBuffer);
}
