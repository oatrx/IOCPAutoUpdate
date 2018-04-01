#include "stdafx.h"
#include "Client.h"
#include "SocketClient.h"
#include "DlgDownLoad.h"
Client::Client()
	:m_pSockClient(NULL)
{
	
}


Client::~Client()
{
	if (NULL != m_pSockClient)
	{
		delete m_pSockClient;
		m_pSockClient = NULL;
	}
}

bool Client::ConnectServer(HWND hMainWnd)
{
	// 首先把退出码设为失败
	DWORD nExitCode = 1;

	m_pSockClient = new(std::nothrow) SocketClient;
	if (NULL == m_pSockClient)
		goto END;

	// 加载sock库
	if (!m_pSockClient->LoadSockLib())
		goto END;

	// 连接服务器失败
	if (!m_pSockClient->start())
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
		int nRet = MessageBox(hMainWnd, "新版本已经下载，重新现在吗?","提示信息", MB_YESNO);
		if (IDYES == nRet)
			bLoad = true;
	}
	else
	{
		int nRet = MessageBox(hMainWnd, "有新版本，现在下载吗?","提示信息", MB_YESNO);
		if (IDYES == nRet)
			bLoad = true;
	}
	// 下载
	if (bLoad)
	{
		DWORD nThreadID = 0;
		HANDLE hThreadDownLoad = ::CreateThread(0, 0, WorkerThread, (void *)m_pSockClient, 0, &nThreadID);
		// 进度条
		CDlgDownLoad dlg(m_pSockClient);
		dlg.DoModal();
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
	m_pSockClient->UnloadSockLib();
	
	// 正常下载
	if (0 == nExitCode)
		return true;
	else
		// 异常或者不下载
		return false;
}

std::string Client::GetFullFilePath()
{
	return m_pSockClient->GetFullFilePath();
}

DWORD WINAPI Client::WorkerThread(LPVOID lpParam)
{
	SocketClient* pSockClient = (SocketClient*)lpParam;
	if (pSockClient->DownLoad(true))
		return 0;
	// 异常
	return 1;
}

