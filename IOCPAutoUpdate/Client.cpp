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
	// ���Ȱ��˳�����Ϊʧ��
	DWORD nExitCode = 1;

	m_pSockClient = new(std::nothrow) SocketClient;
	if (NULL == m_pSockClient)
		goto END;

	// ����sock��
	if (!m_pSockClient->LoadSockLib())
		goto END;

	// ���ӷ�����ʧ��
	if (!m_pSockClient->start())
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
		int nRet = MessageBox(hMainWnd, "�°汾�Ѿ����أ�����������?","��ʾ��Ϣ", MB_YESNO);
		if (IDYES == nRet)
			bLoad = true;
	}
	else
	{
		int nRet = MessageBox(hMainWnd, "���°汾������������?","��ʾ��Ϣ", MB_YESNO);
		if (IDYES == nRet)
			bLoad = true;
	}
	// ����
	if (bLoad)
	{
		DWORD nThreadID = 0;
		HANDLE hThreadDownLoad = ::CreateThread(0, 0, WorkerThread, (void *)m_pSockClient, 0, &nThreadID);
		// ������
		CDlgDownLoad dlg(m_pSockClient);
		dlg.DoModal();
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
	m_pSockClient->UnloadSockLib();
	
	// ��������
	if (0 == nExitCode)
		return true;
	else
		// �쳣���߲�����
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
	// �쳣
	return 1;
}

