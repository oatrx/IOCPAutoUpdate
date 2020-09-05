#include "stdafx.h"
#include <cassert>
#include "Client.h"
#include "SocketClient.h"
#include "DlgDownLoad.h"
#include "DlgTips.h"
Client::Client()
	:m_pSockClient(NULL)
{
	if (NULL == m_pSockClient)
	{
		m_pSockClient = new(std::nothrow) SocketClient;
	}
	// 加载sock库
	if (NULL != m_pSockClient)
		m_pSockClient->LoadSockLib();
}


Client::~Client()
{
	if (NULL != m_pSockClient)
	{
		m_pSockClient->UnloadSockLib();
		delete m_pSockClient;
		m_pSockClient = NULL;
	}
}

int32_t Client::Authenticate(const char* pAppName)
{
	if (!m_pSockClient->IsLibLoaded())
		return 3;

	// 连接服务器失败
	if (!m_pSockClient->start(pAppName))
		return 4;

	//  登录
	if (!m_pSockClient->Login())
		return m_pSockClient->IsSocketError() ? 6 : 5;

	// 版本验证
	int res = m_pSockClient->ProgramVer();
	switch (res)
	{
		// 有新版本可以下载
	case 0:
		return 0;
		break;
		// 2 新版本检验没有新版本， 6 网络异常
	case -1:
		return m_pSockClient->IsSocketError() ? 6 : 2;
		break;
		// 有新版本，但是本地有同新版本名相同的文件存在
	case -2:
		return 1;
		break;
	}
	return -1;
}

bool Client::DownLoad(const char* pAppName,const bool bDownLoad /*= true*/, const bool bShowDlg /*= false*/)
{
	// 首先把退出码设为失败
	DWORD nExitCode = 1;
	if (bDownLoad)
	{
		DWORD nThreadID = 0;
		HANDLE hThreadDownLoad = ::CreateThread(0, 0, WorkerThread, (void *)m_pSockClient, 0, &nThreadID);
		// 进度条
		if (bShowDlg)
		{
			CDlgDownLoad dlg(m_pSockClient);
			dlg.DoModal();
		}
		WaitForSingleObject(hThreadDownLoad, INFINITE);
		GetExitCodeThread(hThreadDownLoad, &nExitCode);
		CloseHandle(hThreadDownLoad);
	}
	else
	{
		// 不下载
		m_pSockClient->DownLoad(false);
	}

	m_pSockClient->stop();
	// 正常下载
	if (0 == nExitCode)
	{
		m_pSockClient->UpdateProgramInfo(pAppName);
		return true;
	}
	else
		// 异常或者不下载
		return false;

}

double Client::GetFileSize()
{
	return m_pSockClient->GetFileSize();
}

int32_t Client::GetDownLoadLeftPercent()
{
	return m_pSockClient->GetDownLoadLeftPercent();
}

bool Client::IsSocketError()
{
	return m_pSockClient->IsSocketError();
}

void Client::StopDownLoad()
{
	m_pSockClient->StopDownLoad();
}

bool Client::DownLoadFile(const char* pAppName, const bool bShowDlg, const HWND hMainWnd)
{
	if (!m_pSockClient->IsLibLoaded())
		goto END;

	// 首先把退出码设为失败
	DWORD nExitCode = 1;
	// 连接服务器失败
	if (!m_pSockClient->start(pAppName))
		goto END;
	
	//  登录
	if (!m_pSockClient->Login())
		goto END;

	// 版本验证
	int res = m_pSockClient->ProgramVer();
	bool bLoad = false;
	if (-1 == res)
	{
		goto END;
	}
	else if (-2 == res)
	{
		int nRet = IDYES;
		if (bShowDlg)
			nRet = MessageBoxUpdate(hMainWnd, "新版本文件已经存在，重新下载吗?");
		if (IDYES == nRet)
			bLoad = true;
	}
	else
	{
		int nRet = IDYES;
		if (bShowDlg)
			nRet = MessageBoxUpdate(hMainWnd, "有新版本，现在下载吗?");
		if (IDYES == nRet)
			bLoad = true;
	}
	// 下载
	if (bLoad)
	{
		DWORD nThreadID = 0;
		HANDLE hThreadDownLoad = ::CreateThread(0, 0, WorkerThread, (void *)m_pSockClient, 0, &nThreadID);
		// 进度条
		if (bShowDlg)
		{
			CDlgDownLoad dlg(m_pSockClient);
			dlg.DoModal();
		}
		WaitForSingleObject(hThreadDownLoad, INFINITE);
		GetExitCodeThread(hThreadDownLoad, &nExitCode);
		CloseHandle(hThreadDownLoad);
	}
	else
	{
		// 不下载
		m_pSockClient->DownLoad(false);
	}

END:
	m_pSockClient->stop();
	
	// 正常下载
	if (0 == nExitCode)
	{
		m_pSockClient->UpdateProgramInfo(pAppName);
		return true;
	}
	else
		// 异常或者不下载
		return false;
}

void Client::GetFullFilePath(char* pszFilePath, int nbufFilePathLen)
{
	std::string strPath = m_pSockClient->GetFullFilePath();
	assert(nbufFilePathLen > strPath.length());
	int nlen = strPath.length() >= nbufFilePathLen ? nbufFilePathLen - 1 : strPath.length();
	memcpy(pszFilePath, strPath.c_str(), nlen);
	pszFilePath[nlen] = '\0';
	return;
}

void Client::GetProgramName(char* pszProgramName, int nbufProgramNameLen)
{
	std::string strProgramName = m_pSockClient->GetProgramName();
	assert(nbufProgramNameLen > strProgramName.length());
	int nlen = strProgramName.length() >= nbufProgramNameLen ? nbufProgramNameLen - 1 : strProgramName.length();
	memcpy(pszProgramName, strProgramName.c_str(), nlen);
	pszProgramName[nlen] = '\0';
	return;
}

uint32_t Client::GetFileVersion()
{
	return m_pSockClient->GetFileVersion();
}

DWORD WINAPI Client::WorkerThread(LPVOID lpParam)
{
	SocketClient* pSockClient = (SocketClient*)lpParam;
	if (pSockClient->DownLoad(true))
		return 0;
	// 异常
	return 1;
}

