#ifndef _LOG_H
#define _LOG_H
#include "LogTask.h"
#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#define LEVEL_NORMAL	0
#define LEVEL_ERROR		1
#define LEVEL_INFOR		2
#define LEVEL_DEBUG		3



class Log :
	public LogTask
{
public:
	static Log& getInstance();
    // 初始化日志打印, 不调用，不记录日子信息,
    // level 日志等级，pLogModuleName: 日志模块名称
    void init(int level, const char* pLogModuleName = nullptr);
    void log(int level, const char* file, const char* func, int lineNo, const char* cFormat, ...);
    void debugBin(const char* szInfo, uint8_t* szData = nullptr, uint32_t nDataLen = 0);
    void debugStr(const char* szInfo, const char *cFormat, ...);
    void info(const char* szInfo, const char *cFormat, ...);
protected:
    void hex2Str(char* outBuf, uint32_t nbufLen, uint8_t* szData, uint32_t nDataLen);
    void log2(int level,const char* cFormat, ...);
	void push_log(int level, char* cFormat, va_list vlist);
    void push_logstr(std::string& strLog);
	
	virtual void run() override;
private:
	void quit();
	std::string pop_log();
	Log();
	Log(const Log&) = delete;
	Log(const Log&&) = delete;
	Log& operator=(const Log&) = delete;
	Log& operator=(const Log&&) = delete;
	std::string getSysTime();
	virtual ~Log();

	bool m_bExit;
	FILE* m_fLog;
    static Log m_log;
	static int m_counter;
	int m_logLevel;
	std::queue<std::string> m_logQue;
	std::mutex m_mutex;
    std::condition_variable m_cv;
};
#define Loger(level,cFormat,...) Log::getInstance().log(level, __FILE__, /*__PRETTY_FUNCTION__*/__FUNCTION__, __LINE__, cFormat, ##__VA_ARGS__);
#define LogDebugBin(szInfo, szData, nDataLen)    Log::getInstance().debugBin(szInfo, szData, nDataLen);
#define LogDebugStr(szInfo, cFormat, ...) Log::getInstance().debugStr(szInfo, cFormat, ##__VA_ARGS__);
#define LogInfoStr(szInfo, cFormat, ...) Log::getInstance().info(szInfo, cFormat, ##__VA_ARGS__);
#define LogFunctionBegin() Loger(LEVEL_INFOR, "%s", "\tbegin!!!!")
#define LogFunctionEnd() Loger(LEVEL_INFOR, "%s", "\tend!!!!")
#ifdef _WIN32
#include "tchar.h"
std::string perro(const char* pszTitle, long nError = -1);
void trace(const char* format, ...);
#endif
#endif // _LOG_H
