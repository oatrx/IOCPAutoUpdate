#ifndef _LOG_H
#define _LOG_H
#include "LogTask.h"
#include <queue>
#include <string>
#include <mutex>

#define LEVEL_NORMAL	0
#define LEVEL_ERROR		1
#define LEVEL_DEBUG		2
#define LEVEL_INFOR		3



class Log :
	public LogTask
{
public:
	static Log& getInstance();
	void removeInstance();
	void setLogLevel(int level);
    void log(int level, const char* file, const char* func, int lineNo, const char* cFormat, ...);
protected:
	void push_log(int level, char* cFormat, va_list vlist);
	virtual void run() override;
private:
	Log();
	Log(const Log&) = delete;
	Log(const Log&&) = delete;
	Log& operator=(const Log&) = delete;
	Log& operator=(const Log&&) = delete;
	std::string getSysTime();
	virtual ~Log();

	bool m_bExit;
	FILE* m_fLog;
	static Log* m_log;
	static int m_counter;
	int m_logLevel;
	std::queue<std::string> m_logQue;
	std::mutex m_mutex;
};
#define Loger(level,cFormat,...) Log::getInstance().log(level, __FILE__, __FUNCTION__, __LINE__, cFormat, ##__VA_ARGS__);

#ifdef _WIN32
#include "tchar.h"
std::string perro(char* pszTitle, long nError = -1);
void trace(const TCHAR* format, ...);
#endif
#endif // _LOG_H
