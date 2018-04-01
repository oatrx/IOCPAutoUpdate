#include "stdafx.h"
#include "LogTask.h"

LogTask::LogTask()
	:m_pThread(NULL)
{
}


LogTask::~LogTask()
{
	if (m_pThread != NULL)
	{
		m_pThread->join();
		delete m_pThread;
		m_pThread = NULL;
	}
}

void LogTask::log_write()
{
	m_pThread = new std::thread(start, this);
}

void LogTask::start(LogTask* pLog)
{
	pLog->run();
}
