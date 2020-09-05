#include <stdafx.h>
#include "Log.h"
#include <cstdarg>
#include <cstring>
#include <chrono>
#include <cstdio>
#include <sstream>

#include <ctime>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#elif defined(linux)
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#endif

using namespace std;
using namespace std::chrono;
#define LOG_BUF_SIZE 20480
#define TIME_BUF_SIZE 64
Log Log::m_log;
int Log::m_counter = 0;


Log::Log()
	:LogTask(),
	m_bExit(false),
	m_fLog(nullptr),
    m_logLevel(LEVEL_NORMAL)
{
}

Log::~Log()
{
    // wait data write end
    while (!m_logQue.empty())
    {
        milliseconds drua(10);
        this_thread::sleep_for(drua);
    }
	if (m_fLog)
	{
		fclose(m_fLog);
		m_fLog = nullptr;
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
    return m_log;
}

void Log::init(int level, const char* pLogModuleName)
{
     std::unique_lock <std::mutex> lck(m_mutex);
	m_logLevel = level;
	if (m_fLog == nullptr && m_logLevel > 0)
	{
		time_t tt = system_clock::to_time_t(system_clock::now());
		char timeBuf[TIME_BUF_SIZE];
		memset(timeBuf, 0, sizeof(timeBuf));
		struct tm* pTime = localtime(&tt);
        sprintf(timeBuf, "%4d-%02d-%02d %02d-%02d-%02d", pTime->tm_year + 1900, pTime->tm_mon + 1,
            pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
        // 检查日子目录
#ifdef _WIN32
        if (_access("log", 0) == -1)
        {
            _mkdir("log");
        }
        _chdir("log");
#elif defined(linux)
        if (access("log", W_OK) == -1)
        {
            if(-1 == mkdir("log", 0755))
                perror("mkdir()");
        }
        if (-1 == chdir("log"))
        {
            perror("chdir()");
        }
#endif
        string strpath = "log_";
        if (pLogModuleName)
        {
            strpath += pLogModuleName;
            strpath += "_";
        }
		strpath += timeBuf;
		strpath += ".txt";
		m_fLog = fopen(strpath.c_str(), "a+");
        // 开启写入文件线程
        m_log.log_write_thread();
#ifdef _WIN32
        _chdir("..");
#elif defined(linux)
        chdir("..")
#endif
	}	
}

void Log::log(int level, const char* file, const char* func, int lineNo, const char* cFormat, ...)
{
    if (level <= LEVEL_NORMAL || level > LEVEL_DEBUG)
        return;
    va_list args;
	ostringstream oss;
    oss << file << ':' << lineNo << ':' << func << "() "<< cFormat;
	va_start(args, cFormat);
    push_log(level, const_cast<char*>(oss.str().c_str()), args);
    va_end(args);
}

void Log::log2(int level, const char *cFormat, ...)
{
    if (level <= LEVEL_NORMAL || level > LEVEL_DEBUG)
        return;
    va_list args;
    va_start(args, cFormat);
    push_log(level, (char*)cFormat, args);
    va_end(args);
}

void Log::debugBin(const char *szInfo, uint8_t *szData, uint32_t nDataLen)
{
    // 不输出debug信息
    if (m_logLevel < LEVEL_DEBUG)
        return;
    char szBuf[LOG_BUF_SIZE];
    hex2Str(szBuf ,sizeof(szBuf) , szData, nDataLen);
    log2(LEVEL_DEBUG, "%s %s",szInfo, szBuf);
}

void Log::debugStr(const char *szInfo, const char *cFormat,...)
{
    if (m_logLevel < LEVEL_DEBUG)
        return;
    va_list args;
    va_start(args, cFormat);
    char logbuf[LOG_BUF_SIZE];
    memset(logbuf, 0, sizeof(logbuf));
    string&& sysTime = getSysTime();
    sprintf(logbuf, "%s [DEBUG] %s", sysTime.c_str(), szInfo);
    size_t lens = strlen(logbuf);
    vsnprintf(logbuf + lens, LOG_BUF_SIZE - 1 - lens, cFormat, args);// last bit'\0'
    va_end(args);

    string LogString(logbuf);
    push_logstr(LogString);

}

void Log::info(const char *szInfo, const char *cFormat,...)
{
    if (m_logLevel < LEVEL_INFOR)
        return;
    va_list args;
    va_start(args, cFormat);
    char logbuf[LOG_BUF_SIZE];
    memset(logbuf, 0, sizeof(logbuf));
    string&& sysTime = getSysTime();
    sprintf(logbuf, "%s  %s", sysTime.c_str(), szInfo);
    size_t lens = strlen(logbuf);
    vsnprintf(logbuf + lens, LOG_BUF_SIZE - 1 - lens, cFormat, args);// last bit'\0'
    va_end(args);

    string LogString(logbuf);
    push_logstr(LogString);
}

void Log::hex2Str(char *outBuf, uint32_t nbufLen, uint8_t *szData, uint32_t nDataLen)
{
    memset(outBuf, 0, nbufLen);
    uint32_t i = 0, j = 0;
    for (; i < nDataLen && j < nbufLen - 4; ++i)
    {

        if (i % 24 == 0)
        {
            sprintf(outBuf + j, "%s", "\n");
            j += 1;
        }
        sprintf(outBuf + j, "%02X ", szData[i]);
        j += 3;
    }
    sprintf(outBuf + j, "%s", "\n");
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
        push_logstr(LogString);
    }
}

void Log::push_logstr(std::string& strLog)
{
    std::unique_lock <std::mutex> lck(m_mutex);
    m_logQue.push(strLog);
    m_cv.notify_all();
}

void Log::quit()
{
	std::unique_lock <std::mutex> lck(m_mutex);
	m_bExit = true;
	m_cv.notify_all();
}

std::string Log::pop_log()
{
	std::string LogString;
	std::unique_lock <std::mutex> lck(m_mutex);
	if (m_logQue.empty())
	{
		m_cv.wait(lck);
	}
	if (!m_logQue.empty())
	{
		LogString = m_logQue.front();
		m_logQue.pop();
	}
	return LogString;
}

void Log::run()
{

	while (!m_bExit)
	{
		std::string LogString = pop_log();
		if (m_fLog && !LogString.empty())
		{
			fprintf(m_fLog, "%s\n", LogString.c_str());
			fflush(m_fLog);
		}
        //fprintf(stderr,"%s\n", LogString.c_str());
	}
}

#ifdef _WIN32
#include "windows.h"
std::string perro(const char* pszTitle, long nError) {
    LPVOID pvErrMsg = NULL;
    if (nError < 0)
        nError = GetLastError();
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, nError,
        MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED),
        (char*)&pvErrMsg,
        0, NULL);
    char szText[256];
    sprintf(szText, "%s(errno=%ld),%s\n", pszTitle, nError, (char*)pvErrMsg);
    LocalFree(pvErrMsg);
    return szText;
}

void trace(const char* format, ...)
{
    static const int nBufferLen = 1024;
    va_list pArg;
    char szMessageBuffer[nBufferLen] = { 0 };
    va_start(pArg, format);
    _vsnprintf(szMessageBuffer, nBufferLen - 1, format, pArg);
    va_end(pArg);
    OutputDebugStringA(szMessageBuffer);
}

#endif // _WIN32
