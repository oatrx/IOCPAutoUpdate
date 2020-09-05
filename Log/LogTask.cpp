#include <stdafx.h>
#include "LogTask.h"

LogTask::LogTask()
	:m_pThread(nullptr)
{
}


LogTask::~LogTask()
{
}

void LogTask::log_write_thread()
{
	m_pThread = new std::thread(start, this);
    m_pThread->detach();
}

void LogTask::start(LogTask* pLog)
{
	pLog->run();
}
