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
	// ����sock��
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

	// ���ӷ�����ʧ��
	if (!m_pSockClient->start(pAppName))
		return 4;

	//  ��¼
	if (!m_pSockClient->Login())
		return m_pSockClient->IsSocketError() ? 6 : 5;

	// �汾��֤
	int res = m_pSockClient->ProgramVer();
	switch (res)
	{
		// ���°汾��������
	case 0:
		return 0;
		break;
		// 2 �°汾����û���°汾�� 6 �����쳣
	case -1:
		return m_pSockClient->IsSocketError() ? 6 : 2;
		break;
		// ���°汾�����Ǳ�����ͬ�°汾����ͬ���ļ�����
	case -2:
		return 1;
		break;
	}
	return -1;
}

bool Client::DownLoad(const char* pAppName,const bool bDownLoad /*= true*/, const bool bShowDlg /*= false*/)
{
	// ���Ȱ��˳�����Ϊʧ��
	DWORD nExitCode = 1;
	if (bDownLoad)
	{
		DWORD nThreadID = 0;
		HANDLE hThreadDownLoad = ::CreateThread(0, 0, WorkerThread, (void *)m_pSockClient, 0, &nThreadID);
		// ������
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
		// ������
		m_pSockClient->DownLoad(false);
	}

	m_pSockClient->stop();
	// ��������
	if (0 == nExitCode)
	{
		m_pSockClient->UpdateProgramInfo(pAppName);
		return true;
	}
	else
		// �쳣���߲�����
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

	// ���Ȱ��˳�����Ϊʧ��
	DWORD nExitCode = 1;
	// ���ӷ�����ʧ��
	if (!m_pSockClient->start(pAppName))
		goto END;
	
	//  ��¼
	if (!m_pSockClient->Login())
		goto END;

	// �汾��֤
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
			nRet = MessageBoxUpdate(hMainWnd, "�°汾�ļ��Ѿ����ڣ�����������?");
		if (IDYES == nRet)
			bLoad = true;
	}
	else
	{
		int nRet = IDYES;
		if (bShowDlg)
			nRet = MessageBoxUpdate(hMainWnd, "���°汾������������?");
		if (IDYES == nRet)
			bLoad = true;
	}
	// ����
	if (bLoad)
	{
		DWORD nThreadID = 0;
		HANDLE hThreadDownLoad = ::CreateThread(0, 0, WorkerThread, (void *)m_pSockClient, 0, &nThreadID);
		// ������
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
		// ������
		m_pSockClient->DownLoad(false);
	}

END:
	m_pSockClient->stop();
	
	// ��������
	if (0 == nExitCode)
	{
		m_pSockClient->UpdateProgramInfo(pAppName);
		return true;
	}
	else
		// �쳣���߲�����
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
	// �쳣
	return 1;
}

