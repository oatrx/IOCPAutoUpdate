#include "stdafx.h"
#include "Log.h"
#include <cstdarg>
#include <cstring>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <sstream>
using namespace std;
using namespace std::chrono;
#define LOG_BUF_SIZE 2048
#define TIME_BUF_SIZE 64
Log* Log::m_log = nullptr;
int Log::m_counter = 0;

Log::Log()
	:LogTask(),
	m_bExit(false),
	m_fLog(nullptr),
	m_logLevel(LEVEL_ERROR)
{

}

Log::~Log()
{
	m_bExit = true;
	if (m_fLog)
	{
		fclose(m_fLog);
	}
}

std::string Log::getSysTime()
{
	time_t tt = system_clock::to_time_t(system_clock::now());
	char timeBuf[TIME_BUF_SIZE];
	memset(timeBuf, 0, sizeof(timeBuf));
	struct tm* pTime = localtime(&tt);
	sprintf(timeBuf, "%4d/%02d/%02d %02d:%02d:%02d", pTime->tm_year + 1900, pTime->tm_mon + 1,
		pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
	return string(timeBuf);
}

Log& Log::getInstance()
{

	if (m_log == nullptr)
	{
		m_log = new Log();
		// write thread
		m_log->log_write();
	}
	return *m_log;
}

void Log::removeInstance()
{
	// wait data write end
	while (!m_logQue.empty())
	{
		milliseconds drua(10);
		this_thread::sleep_for(drua);
	}
	delete this;
}

void Log::setLogLevel(int level)
{
	m_logLevel = level;
	if (m_fLog == nullptr && m_logLevel > 0)
	{
		time_t tt = system_clock::to_time_t(system_clock::now());
		char timeBuf[TIME_BUF_SIZE];
		memset(timeBuf, 0, sizeof(timeBuf));
		struct tm* pTime = localtime(&tt);
		sprintf(timeBuf, "%4d-%02d-%02d", pTime->tm_year + 1900, pTime->tm_mon + 1,
			pTime->tm_mday);
		string strpath = "log_IOCPupdate_";
		strpath += timeBuf;
		strpath += ".txt";
		m_fLog = fopen(strpath.c_str(), "a+");
	}	
}

void Log::log(int level, const char* file, const char* func, int lineNo, const char* cFormat, ...)
{
	va_list args = NULL;
	ostringstream oss;
    oss << file << " [" << lineNo << "][" << func << "] "<< cFormat;
	va_start(args, cFormat);
	m_log->push_log(level, const_cast<char*>(oss.str().c_str()), args);
	va_end(args);
}

void Log::push_log(int level, char* cFormat, va_list vlist)
{
	if (level <= m_logLevel)
	{
		char levelstr[20];
		switch (level)
		{
		case LEVEL_DEBUG:
			strcpy(levelstr, "DEBUG");
			break;
		case LEVEL_ERROR:
			strcpy(levelstr, "ERROR");
			break;
		case LEVEL_INFOR:
		default:
			strcpy(levelstr, "INFOR");
			break;
		}
		char logbuf[LOG_BUF_SIZE];
		memset(logbuf, 0, sizeof(logbuf));
		string&& sysTime = getSysTime();
		sprintf(logbuf, "%s [%s] ", sysTime.c_str(), levelstr);
		int lens = strlen(logbuf);
		vsnprintf(logbuf + lens, LOG_BUF_SIZE - 1 - lens, cFormat, vlist);// last bit'\0'
		
		string LogString(logbuf);

		m_mutex.lock();
		m_logQue.push(LogString);
		m_mutex.unlock();
	}
}

void Log::run()
{
	while (!m_bExit)
	{
		string LogString;
		m_mutex.lock();
		if (!m_logQue.empty())
		{
			LogString = m_logQue.front();
			m_logQue.pop();
		}
		m_mutex.unlock();
		if (m_fLog && !LogString.empty())
		{
			fprintf(m_fLog, "%s\n", LogString.c_str());
			fflush(m_fLog);
			printf("%s\n", LogString.c_str());
		}
	}
}

#ifdef _WIN32
#include "windows.h"
std::string perro(char* pszTitle, long nError) {
	LPVOID pvErrMsg = NULL;
	if (nError < 0)
		nError = GetLastError();
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, nError,
		MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED),
		(char*)&pvErrMsg,
		0, NULL);
	char szText[256];
	sprintf(szText, "%s(errno=%d),%s\n", pszTitle, nError, (char*)pvErrMsg);
	LocalFree(pvErrMsg);
	return szText;
}

void trace(const TCHAR* format, ...)
{
	static const int nBufferLen = 1024;
	static int nCount;
	va_list pArg;
	TCHAR szMessageBuffer[nBufferLen] = { 0 };
	va_start(pArg, format);
	_vsnprintf(szMessageBuffer, nBufferLen - 1, format, pArg);
	va_end(pArg);
	OutputDebugString(szMessageBuffer);
}

#endif // _WIN32