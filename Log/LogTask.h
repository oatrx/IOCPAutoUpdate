#ifndef _LOGINTERFACE_H
#define _LOGINTERFACE_H

#include <thread>

class LogTask
{
protected:
	void log_write();
    static void start(LogTask* plog);
	virtual void run() = 0;
    LogTask();
    LogTask(const LogTask&) = delete;
    LogTask(const LogTask&&) = delete;
    LogTask& operator=(const LogTask&) = delete;
    LogTask& operator=(const LogTask&&) = delete;
    virtual ~LogTask();
private:
	std::thread* m_pThread;
};

#endif // _LOGINTERFACE_H
