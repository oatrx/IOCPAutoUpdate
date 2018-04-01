#include "stdafx.h"
#include "Header.h"
#include "IOCPServer.h"
#include "Log.h"
#include "windows.h"
#include "tchar.h"
#include <shlwapi.h>
#include <MessageDisplay.h>
//#include "vld.h"
//#pragma comment(lib, "vld.lib")
using namespace std;
#pragma warning(disable:4996)
// ÿһ���������ϲ������ٸ��߳�(Ϊ������޶ȵ��������������ܣ���������ĵ�)
#define WORKER_THREADS_PER_PROCESSOR 2
// ͬʱͶ�ݵ�Accept���������(���Ҫ����ʵ�ʵ�����������)
#define MAX_POST_ACCEPT              8
// ���ݸ�Worker�̵߳��˳��ź�
#define EXIT_CODE                    NULL

// Ĭ�϶˿�
#define DEFAULT_PORT          "12345"
// Ĭ��IP��ַ
#define DEFAULT_IP            _T("127.0.0.1")

// �ͷ�ָ��;����Դ�ĺ�
// �ͷ�ָ���
#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}
// �ͷž����
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}
// �ͷ�Socket��
#define RELEASE_SOCKET(x)               {if(x !=INVALID_SOCKET) { closesocket(x);x=INVALID_SOCKET;}}

#pragma comment(lib,"ws2_32.lib")

IOCPServer::IOCPServer(MessageDisplay* pMessageDlg)
	:m_nThreads(0),
	m_phWorkerThreads(NULL),
	m_hIOCompletionPort(NULL),
	m_hShutdownEvent(NULL),
	m_bLibLoaded(false),
	m_nPort(0),
	m_pListenContext(NULL),
	m_pMessageDlg(pMessageDlg)
{
	// ��ʼ���߳��ٽ���
	InitializeCriticalSection(&m_csClientSockContext);
	Log::getInstance().setLogLevel(LEVEL_INFOR);
}


IOCPServer::~IOCPServer()
{
	Stop();
	// ɾ���ͻ����б��ٽ���
	DeleteCriticalSection(&m_csClientSockContext);
	Log::getInstance().removeInstance();
}

bool IOCPServer::LoadSockLib()
{
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// ����(һ�㶼�����ܳ���)
	if (NO_ERROR != nResult)
	{
		Loger(LEVEL_ERROR, perro("WSAStartup", WSAGetLastError()).c_str());
		return m_bLibLoaded = false;
	}
	return m_bLibLoaded = true;
}

void IOCPServer::UnloadSockLib()
{
	if (m_bLibLoaded)
	{
		m_bLibLoaded = false;
		if (SOCKET_ERROR == WSACleanup())
			Loger(LEVEL_ERROR, perro("WSACleanup", WSAGetLastError()).c_str());
	}
}

bool IOCPServer::Start()
{
	// ��ȡ������Ϣ
	if (false == ReadConfig())
	{
		m_pMessageDlg->ShowMessage("��������ȡ�����ļ�ʧ�ܣ�");
		return false;
	}

	// ����ϵͳ�˳����¼�֪ͨ
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == m_hShutdownEvent)
	{
		Loger(LEVEL_ERROR, perro("��ʼ���˳��¼�ʧ��.", WSAGetLastError()).c_str());
		return false;
	}

	// ��ʼ��IOCP
	if (false == _InitializeIOCP())
	{
		Loger(LEVEL_ERROR, perro("��ʼ��IOCPʧ��.", WSAGetLastError()).c_str());
		return false;
	}
	else
	{
		Loger(LEVEL_INFOR, "��ʼ��IOCP���.");
	}

	// ��ʼ��Socket
	if (false == _InitializeListenSocket())
	{
		Loger(LEVEL_ERROR, perro("_InitializeListenSocket", WSAGetLastError()).c_str());
		Stop();
		return false;
	}
	else
	{
		Loger(LEVEL_INFOR, "Listen Socket��ʼ�����.");
	}

	//this->_ShowMessage(_T("ϵͳ׼���������Ⱥ�����....\n"));
	return true;
}

void IOCPServer::Stop()
{
	// ����ر���Ϣ֪ͨ
	if (NULL != m_hShutdownEvent && INVALID_HANDLE_VALUE != m_hShutdownEvent)
		SetEvent(m_hShutdownEvent);

	if (NULL != m_phWorkerThreads)
	{
		for (int i = 0; i < m_nThreads; i++)
		{
			// ֪ͨ���е���ɶ˿ڲ����˳�
			PostQueuedCompletionStatus(m_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
		}
		// �ȴ����еĿͻ�����Դ�˳�
		WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);
	}

	// ����ͻ����б���Ϣ
	this->_ClearContextList();

	// �ͷ�������Դ
	this->_DeInitialize();
}

string IOCPServer::GetLocalIP()
{
	// ��ñ���������
	char hostname[MAX_PATH] = { 0 };
	gethostname(hostname, MAX_PATH);
	struct hostent FAR* lpHostEnt = gethostbyname(hostname);
	if (lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}

	// ȡ��IP��ַ�б��еĵ�һ��Ϊ���ص�IP(��Ϊһ̨�������ܻ�󶨶��IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	// ��IP��ַת�����ַ�����ʽ
	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);
	m_strIP = string(inet_ntoa(inAddr));

	return m_strIP;
}

DWORD WINAPI IOCPServer::_WorkerThread(LPVOID lpParam)
{
	THREADPARAMS* pParam = (THREADPARAMS*)lpParam;
	IOCPServer* pIOCPServer = (IOCPServer*)pParam->pIOCPServer;
	int nThreadNo = (int)pParam->nThreadNo;

	trace(_T("�������߳�������ID: %d.\n"), nThreadNo);

	OVERLAPPED           *pOverlapped = NULL;
	PER_SOCKET_CONTEXT   *pSocketContext = NULL;
	DWORD                dwBytesTransfered = 0;

	// ѭ����������֪�����յ�Shutdown��ϢΪֹ
	while (WAIT_OBJECT_0 != WaitForSingleObject(pIOCPServer->m_hShutdownEvent, 0))
	{
		BOOL bReturn = GetQueuedCompletionStatus(
			pIOCPServer->m_hIOCompletionPort,
			&dwBytesTransfered,
			(PULONG_PTR)&pSocketContext,
			&pOverlapped,
			INFINITE);

		// ����յ������˳���־����ֱ���˳�
		if (EXIT_CODE == (DWORD)pSocketContext)
		{
			break;
		}

		// �ж��Ƿ�����˴���
		if (!bReturn)
		{
			DWORD dwErr = GetLastError();

			// ��ʾһ����ʾ��Ϣ
			if (!pIOCPServer->HandleError(pSocketContext, dwErr))
			{
				break;
			}

			continue;
		}
		else
		{
			// ��ȡ����Ĳ���
			PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_Overlapped);

			// �ж��Ƿ��пͻ��˶Ͽ���
			if ((0 == dwBytesTransfered) 
				&& (ACCEPT_LOGIN == pIoContext->m_OpType 
				|| LOGIN_RET == pIoContext->m_OpType
				|| PROGRAM_VER == pIoContext->m_OpType
				|| PROGRAM_VER_RET == pIoContext->m_OpType
				|| DOWNLOAD == pIoContext->m_OpType
				|| DOWNLOAD_DATA == pIoContext->m_OpType
				|| DOWNLOAD_END == pIoContext->m_OpType))
			{
				Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d %s", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port), perro("�Ͽ����ӣ�", WSAGetLastError()).c_str());
				pIOCPServer->m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d �Ͽ����ӣ�", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
				// �ͷŵ���Ӧ����Դ
				pIOCPServer->_RemoveContext(pSocketContext);
				continue;
			}
			else
			{
				switch (pIoContext->m_OpType)
				{
					// Accept  
				case ACCEPT_LOGIN:
				{
					// Ϊ�����Ӵ���ɶ��ԣ�������ר�ŵ�_DoAcceptLogin�������д�����������,��ȡ��Login�����ݽ���У��
					pIOCPServer->_DoAccpetLogin(pSocketContext, pIoContext);
				}
				break;
				// SEND
				case LOGIN_RET:
				{
					if (pIoContext->m_bLoginSuss)
						// Ͷ���հ汾��Ϣ������,Ͷ�������ʱ��������ر�socket
						pIOCPServer->_DoPostRecv(pSocketContext, pIoContext, PROGRAM_VER);
					else
					{
						Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d �û�У��ʧ�ܣ�", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
						pIOCPServer->m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d �û�У��ʧ�ܣ�", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
						pIOCPServer->_RemoveContext(pSocketContext);
					}
				}
				break;
				// RECV - SEND
				case PROGRAM_VER:
				{
					// ������յ��İ汾��Ϣ
					pIOCPServer->_DoProgramVersion(pSocketContext, pIoContext);
				}
				break;
				// SEND END
				case PROGRAM_VER_RET:
				{
					if (pIoContext->m_bNewVersionVerify)
						// Ͷ��������ȷ����Ϣ������
						pIOCPServer->_DoPostRecv(pSocketContext, pIoContext, DOWNLOAD);
					else
					{
						Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d ������û���°汾��", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
						pIOCPServer->m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d ������û���°汾��", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
						pIOCPServer->_RemoveContext(pSocketContext);
					}
				}
				break;
				// RECV - SEND
				case DOWNLOAD:
				{
					// ��������ȷ����Ϣ��Ͷ���ļ�����
					if (false == pIOCPServer->_DoDownLoad(pSocketContext, pIoContext))
					{
						Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d �������°汾��", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
						pIOCPServer->m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d �������°汾��", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
						pIOCPServer->_RemoveContext(pSocketContext);
					}
				}
				break;
				// SEND
				case DOWNLOAD_DATA:
				{
					// �������ݽ���
					Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d ������ɣ�", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
					pIOCPServer->m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d ������ɣ�", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
					// Ͷ���ļ����������Ϣʧ��
					if(false == pIOCPServer->_DoDownLoadFinish(pSocketContext, pIoContext))
						pIOCPServer->_RemoveContext(pSocketContext);
				}
				break;
				case DOWNLOAD_END:
				{
					// �ر�socket
					pIOCPServer->_RemoveContext(pSocketContext);
				}
				break;
				default:
					// ��Ӧ��ִ�е�����
					trace(_T("_WorkThread�е� pIoContext->m_OpType �����쳣.\n"));
					break;
				} //switch
			}//if
		}//if

	}//while

	trace(_T("�������߳� %d ���˳�.\n"), nThreadNo);

	// �ͷ��̲߳���
	RELEASE(lpParam);

	return 0;
}

////////////////////////////////
// ��ʼ����ɶ˿�
bool IOCPServer::_InitializeIOCP()
{
	// ������һ����ɶ˿�
	m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hIOCompletionPort)
	{
		Loger(LEVEL_ERROR, perro("CreateIoCompletionPort", WSAGetLastError()).c_str());
		return false;
	}

	// ���ݱ����еĴ�����������������Ӧ���߳���
	m_nThreads = WORKER_THREADS_PER_PROCESSOR * _GetNoOfProcessors();

	// Ϊ�������̳߳�ʼ�����
	m_phWorkerThreads = new HANDLE[m_nThreads];

	// ���ݼ�����������������������߳�
	DWORD nThreadID;
	for (int i = 0; i < m_nThreads; i++)
	{
		PTHREADPARAMS pThreadParams = new THREADPARAMS;
		pThreadParams->pIOCPServer = this;
		pThreadParams->nThreadNo = i + 1;
		m_phWorkerThreads[i] = ::CreateThread(0, 0, _WorkerThread, (void *)pThreadParams, 0, &nThreadID);
	}

	trace(" ���� _WorkerThread %d ��.\n", m_nThreads);
	return true;
}


/////////////////////////////////////////////////////////////////
// ��ʼ��Socket
bool IOCPServer::_InitializeListenSocket()
{
	// AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	GUID GuidTransmitFile = WSAID_TRANSMITFILE;

	// ��������ַ��Ϣ�����ڰ�Socket
	struct sockaddr_in ServerAddress;

	// �������ڼ�����Socket����Ϣ
	m_pListenContext = new PER_SOCKET_CONTEXT;

	// ��Ҫʹ���ص�IO�������ʹ��WSASocket������Socket���ſ���֧���ص�IO����
	m_pListenContext->m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_pListenContext->m_Socket)
	{
		Loger(LEVEL_ERROR, perro("��ʼ��socketʧ�ܣ�������룡", WSAGetLastError()).c_str());
		return false;
	}
	else
	{
		trace("WSASocket() ���.\n");
	}

	// ��Listen Socket������ɶ˿���
	if (NULL == CreateIoCompletionPort((HANDLE)m_pListenContext->m_Socket, m_hIOCompletionPort, (DWORD)m_pListenContext, 0))
	{
		Loger(LEVEL_ERROR, perro("�� Listen Socket����ɶ˿�ʧ�ܣ�", WSAGetLastError()).c_str());
		RELEASE_SOCKET(m_pListenContext->m_Socket);
		return false;
	}
	else
	{
		trace("Listen Socket����ɶ˿� ���.\n");
	}

	// ����ַ��Ϣ
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	// ������԰��κο��õ�IP��ַ�����߰�һ��ָ����IP��ַ 
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                      
	ServerAddress.sin_addr.s_addr = inet_addr(m_strIP.c_str());
	ServerAddress.sin_port = htons(m_nPort);

	// �󶨵�ַ�Ͷ˿�
	if (SOCKET_ERROR == ::bind(m_pListenContext->m_Socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)))
	{
		Loger(LEVEL_ERROR, perro("bind����ִ�д���", WSAGetLastError()).c_str());
		return false;
	}
	else
	{
		trace("bind() ���.\n");
	}

	// ��ʼ���м���
	if (SOCKET_ERROR == listen(m_pListenContext->m_Socket, SOMAXCONN))
	{
		Loger(LEVEL_ERROR, perro("listen����ִ�д���", WSAGetLastError()).c_str());
		return false;
	}
	else
	{
		trace("Listen() ���.\n");
	}

	// ʹ��AcceptEx��������Ϊ���������WinSock2�淶֮���΢�������ṩ����չ����
	// ������Ҫ�����ȡһ�º�����ָ�룬
	// ��ȡAcceptEx����ָ��
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		m_pListenContext->m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx),
		&dwBytes,
		NULL,
		NULL))
	{
		Loger(LEVEL_ERROR, perro("WSAIoctl δ�ܻ�ȡAcceptEx����ָ�룡", WSAGetLastError()).c_str());
		return false;
	}

	// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
	if (SOCKET_ERROR == WSAIoctl(
		m_pListenContext->m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&m_lpfnGetAcceptExSockAddrs,
		sizeof(m_lpfnGetAcceptExSockAddrs),
		&dwBytes,
		NULL,
		NULL))
	{
		Loger(LEVEL_ERROR, perro("WSAIoctl δ�ܻ�ȡGetAcceptExSockAddrs����ָ�룡", WSAGetLastError()).c_str());
		return false;
	}

	// ��ȡTransmitFile����ָ�룬Ҳ��ͬ��
	dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		m_pListenContext->m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidTransmitFile,
		sizeof(GuidTransmitFile),
		&m_lpfnTransmitFile,
		sizeof(m_lpfnTransmitFile),
		&dwBytes,
		NULL,
		NULL))
	{
		Loger(LEVEL_ERROR, perro("WSAIoctl δ�ܻ�ȡTransmitFile����ָ�룡", WSAGetLastError()).c_str());
		return false;
	}


	// ΪAcceptEx ׼��������Ȼ��Ͷ��AcceptEx I/O����
	for (int i = 0; i < MAX_POST_ACCEPT; i++)
	{
		// �½�һ��IO_CONTEXT
		PER_IO_CONTEXT* pAcceptIoContext = m_pListenContext->GetNewIoContext();

		if (false == this->_PostAccept(pAcceptIoContext))
		{
			m_pListenContext->RemoveContext(pAcceptIoContext);
			return false;
		}
	}

	trace(_T("Ͷ�� %d ��AcceptEx�������"), MAX_POST_ACCEPT);
	m_pMessageDlg->ShowMessage("������������");
	return true;

}

////////////////////////////////////////////////////////////
//	����ͷŵ�������Դ
void IOCPServer::_DeInitialize()
{
	// �ر�ϵͳ�˳��¼����
	RELEASE_HANDLE(m_hShutdownEvent);

	// �ͷŹ������߳̾��ָ��
	if (NULL != m_phWorkerThreads)
	{
		for (int i = 0; i < m_nThreads; i++)
		{
			RELEASE_HANDLE(m_phWorkerThreads[i]);
		}

		RELEASE(m_phWorkerThreads);
	}

	// �ر�IOCP���
	RELEASE_HANDLE(m_hIOCompletionPort);

	// �رռ���Socket
	RELEASE(m_pListenContext);

}

////////////////////////////////////////////////////////////
// ���пͻ��������ʱ�򣬽��д���
// �����е㸴�ӣ���Ҫ�ǿ������Ļ����Ϳ����׵��ĵ���....
// ������������Ļ�����ɶ˿ڵĻ������������һ�����
// ��֮��Ҫ֪�����������ListenSocket��Context��������Ҫ����һ�ݳ������������Socket��
// ԭ����Context����Ҫ���������Ͷ����һ��Accept����
//
bool IOCPServer::_DoAccpetLogin(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);

	///////////////////////////////////////////////////////////////////////////
	// 1. ����ȡ������ͻ��˵ĵ�ַ��Ϣ
	// ��� m_lpfnGetAcceptExSockAddrs �����˰�~~~~~~
	// ��������ȡ�ÿͻ��˺ͱ��ض˵ĵ�ַ��Ϣ������˳��ȡ���ͻ��˷����ĵ�һ�����ݣ���ǿ����...
	this->m_lpfnGetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);

	// ��Ϣ���
	Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d ���ӽ�����", inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port));
	m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d ���ӽ�����", inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port));
	// У���¼ �û���,����
	sHEADER _header;
	ZeroMemory(&_header, sizeof(_header));
	memmove(&_header, pIoContext->m_wsaBuf.buf, sizeof(_header));
	sLOGINRET _loginRet;
	ZeroMemory(&_loginRet, sizeof(_loginRet));
	if (UPDATE_NET_CMD_LOGIN == _header.dwCmd)
	{
		sLOGIN _login;
		memmove(&_login, pIoContext->m_wsaBuf.buf + sizeof(_header), sizeof(_login));
		int nCnt = m_mapUser2pwd.count(_login.szUserName);
		if (nCnt> 0 && m_mapUser2pwd[_login.szUserName] == _login.szPassword)
		{
			_loginRet.dwSuccess = 1;
			// ��Ϣ���
			Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d ��¼�ɹ���", inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port));
			m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d ��¼�ɹ���", inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port));
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. ������Ҫע�⣬���ﴫ��������ListenSocket�ϵ�Context�����Context���ǻ���Ҫ���ڼ�����һ������
	// �����һ���Ҫ��ListenSocket�ϵ�Context���Ƴ���һ��Ϊ�������Socket�½�һ��SocketContext
	PER_SOCKET_CONTEXT* pNewSocketContext = new PER_SOCKET_CONTEXT;
	pNewSocketContext->m_Socket = pIoContext->m_sockAccept;
	memcpy(&(pNewSocketContext->m_ClientAddr), ClientAddr, sizeof(SOCKADDR_IN));

	// ����������ϣ������Socket����ɶ˿ڰ�(��Ҳ��һ���ؼ�����)
	if (false == this->_AssociateWithIOCP(pNewSocketContext))
	{
		RELEASE(pNewSocketContext);
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. �������������µ�IoContext�����������Socket��Ͷ�ݵ�һ��Recv��������
	PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->m_OpType = LOGIN_RET;
	pNewIoContext->m_bLoginSuss = (1 == _loginRet.dwSuccess) ? true : false; // ��¼�Ƿ�ɹ�
	pNewIoContext->m_sockAccept = pNewSocketContext->m_Socket;

	// �����֮�󣬾Ϳ��Կ�ʼ�����Socket��Ͷ����ɡ�Ͷ�ݵ�¼��֤���
	if (false == this->_PostSend(pNewIoContext, UPDATE_NET_CMD_LOGIN_RET, &_loginRet, sizeof(_loginRet)))
	{
		Loger(LEVEL_ERROR, "Ͷ���û���¼��ϢУ����ʧ�ܣ�");
		pNewSocketContext->RemoveContext(pNewIoContext);
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// 4. ���Ͷ�ݳɹ�����ô�Ͱ������Ч�Ŀͻ�����Ϣ�����뵽ContextList��ȥ(��Ҫͳһ���������ͷ���Դ)
	this->_AddToContextList(pNewSocketContext);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 5. ʹ�����֮�󣬰�Listen Socket���Ǹ�IoContext���ã�Ȼ��׼��Ͷ���µ�AcceptEx
	pIoContext->ResetBuffer();
	return this->_PostAccept(pIoContext);
}

bool IOCPServer::_PostAccept(PER_IO_CONTEXT* pAcceptIoContext)
{
	assert(INVALID_SOCKET != m_pListenContext->m_Socket);

	// ׼������
	DWORD dwBytes = 0;
	pAcceptIoContext->m_OpType = ACCEPT_LOGIN;
	WSABUF *p_wbuf = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pAcceptIoContext->m_Overlapped;

	// Ϊ�Ժ�������Ŀͻ�����׼����Socket( ������봫ͳaccept�������� ) 
	pAcceptIoContext->m_sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == pAcceptIoContext->m_sockAccept)
	{
		Loger(LEVEL_ERROR, perro("��������Accept��Socketʧ�ܣ�", WSAGetLastError()).c_str());
		return false;
	}

	// Ͷ��AcceptEx
	if (FALSE == m_lpfnAcceptEx(m_pListenContext->m_Socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, p_ol))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			Loger(LEVEL_ERROR, perro("Ͷ�� AcceptEx ����ʧ�ܣ�", WSAGetLastError()).c_str());
			return false;
		}
	}
	return true;
}

bool IOCPServer::_AssociateWithIOCP(PER_SOCKET_CONTEXT *pContext)
{
	// �����ںͿͻ���ͨ�ŵ�SOCKET�󶨵���ɶ˿���
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pContext->m_Socket, m_hIOCompletionPort, (DWORD)pContext, 0);
	if (NULL == hTemp)
	{
		Loger(LEVEL_ERROR, perro("ִ��CreateIoCompletionPort()�󶨿ͻ���socket����", WSAGetLastError()).c_str());
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////
// ���ͻظ���Ϣ��Ͷ�ݽ�����������
bool IOCPServer::_DoPostRecv(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext, OPERATION_TYPE opType)
{
	pIoContext->ResetBuffer();
	pIoContext->m_OpType = opType;
	// Ȼ��ʼͶ����һ��WSARecv����
	return _PostRecv(pIoContext);
}

////////////////////////////////////////////////////////////////////
// Ͷ�ݽ�����������
bool IOCPServer::_PostRecv(PER_IO_CONTEXT* pIoContext)
{
	// ��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;

	// ��ʼ����ɺ󣬣�Ͷ��WSARecv����
	int nBytesRecv = WSARecv(pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL);

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		Loger(LEVEL_ERROR, perro("Ͷ�ݽ�����������WSARecvʧ�ܣ�", WSAGetLastError()).c_str());
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////
// �յ��汾��Ϣ��У��汾��
bool IOCPServer::_DoProgramVersion(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	// ������������汾��
	sHEADER _header;
	ZeroMemory(&_header, sizeof(_header));
	memmove(&_header, pIoContext->m_wsaBuf.buf, sizeof(_header));
	sPROGRAMVERRET _programVerRet;
	ZeroMemory(&_programVerRet, sizeof(_programVerRet));
	if (UPDATE_NET_CMD_PROGRAM_VER == _header.dwCmd)
	{
		sPROGRAMVER _programVer;
		memmove(&_programVer, pIoContext->m_wsaBuf.buf + sizeof(_header), sizeof(_programVer));
		int nCnt = m_mapProName2Info.count(_programVer.szProgramName);
		// ����汾�űȷ������ϵĵ�
		if (nCnt >0 && 0 < strcmp(m_mapProName2Info[_programVer.szProgramName].szProgramVersion, _programVer.szVer))
		{
			_programVerRet.dwNewVer = 1;
			memmove(pIoContext->m_szProFullPath, m_mapProName2Info[_programVer.szProgramName].szFullPath, strlen(m_mapProName2Info[_programVer.szProgramName].szFullPath));
			// ��ȡ�ļ�����
			string strProgramPath(pIoContext->m_szProFullPath);
			int npos = strProgramPath.find_last_of("\\");
			string strFileName(strProgramPath.substr(npos + 1));
			memmove(_programVerRet.szFileName, strFileName.c_str(), strFileName.length());
			// ���ļ�
			pIoContext->m_hProFile = CreateFile(pIoContext->m_szProFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (INVALID_HANDLE_VALUE == pIoContext->m_hProFile)
			{
				// û���ҵ��ļ�,����û�а汾������Ϣ���ͻ���
				_programVerRet.dwNewVer = 0;
				Loger(LEVEL_ERROR, "�ͻ��ˣ�%s:%d %s", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port) , perro("�򿪴����ļ�ʧ�ܣ�", WSAGetLastError()).c_str());
				m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d �򿪴����ļ�ʧ�ܣ���������%s, �ļ�ȫ·����%s", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port), _programVer.szProgramName, pIoContext->m_szProFullPath);
				_programVerRet.dwFileSize = 0;
				pIoContext->m_nProFileSize = 0;
			}
			else
			{
				pIoContext->m_nProFileSize = GetFileSize(pIoContext->m_hProFile, NULL);
				_programVerRet.dwFileSize = pIoContext->m_nProFileSize;
				// ��Ϣ���
				Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d У�����°汾��", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
				m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d У�����°汾��", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
			}
		}
	}
	pIoContext->m_OpType = PROGRAM_VER_RET;
	pIoContext->m_bNewVersionVerify = (1 == _programVerRet.dwNewVer) ? true : false; // �Ƿ����°汾
	// У����ɣ�Socket��Ͷ����ɡ�Ͷ�ݰ汾У��Ľ����������
	if (false == this->_PostSend(pIoContext, UPDATE_NET_CMD_PROGRAM_VER_RET, &_programVerRet, sizeof(_programVerRet)))
	{
		Loger(LEVEL_ERROR, "Ͷ�ݰ汾У������Ϣʧ�ܣ�");
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////
// �յ�����ȷ����Ϣ�󣬴���
bool IOCPServer::_DoDownLoad(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	// ������������汾��
	sHEADER _header;
	ZeroMemory(&_header, sizeof(_header));
	memmove(&_header, pIoContext->m_wsaBuf.buf, sizeof(_header));
	if (UPDATE_NET_CMD_DOWNLOAD == _header.dwCmd)
	{
		sDOWNLOAD _downLoad;
		memmove(&_downLoad, pIoContext->m_wsaBuf.buf + sizeof(_header), sizeof(_downLoad));
		if (1 == _downLoad.dwDownLoad)
		{
			// ��Ϣ���
			Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d �����°汾��", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
			m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d �����°汾��", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
			pIoContext->m_OpType = DOWNLOAD_DATA;
			return _DoTransmitFile(pSocketContext, pIoContext);
		}
	}
	return false;
}

bool IOCPServer::_DoTransmitFile(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	if (m_lpfnTransmitFile(pIoContext->m_sockAccept, pIoContext->m_hProFile, 0, MAX_BUFFER_LEN, &(pIoContext->m_Overlapped), NULL, TF_USE_DEFAULT_WORKER) == FALSE)
	{
		if (ERROR_IO_PENDING != WSAGetLastError())
		{
			Loger(LEVEL_ERROR, perro("��������ʧ�ܣ�", WSAGetLastError()).c_str());
			CloseHandle(pIoContext->m_hProFile);
			return false;
		}
	}
	Loger(LEVEL_INFOR, "�������ݹ����У�");
	m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d ���ع����У�", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
	return true;
}

bool IOCPServer::_DoDownLoadFinish(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	sDOWNLOADRET downLoadRet;
	ZeroMemory(&downLoadRet, sizeof(downLoadRet));
	downLoadRet.dwDownLoadFinish = 1;
	pIoContext->m_OpType = DOWNLOAD_END;
	return _PostSend(pIoContext, UPDATE_NET_CMD_DOWNLOAD_RET, &downLoadRet, sizeof(downLoadRet));
}

////////////////////////////////////////////////////////////////////
// Ͷ�ݷ�����������
bool IOCPServer::_PostSend(PER_IO_CONTEXT* pIoContext, uint32_t dwCmdID, LPVOID pData, uint32_t dataLen)
{
	sHEADER header = { 0 };
	header.dwCmd = dwCmdID;
	header.dwLen = dataLen;
	pIoContext->ResetBuffer();
	pIoContext->m_wsaBuf.len = sizeof(header);
	memmove(pIoContext->m_wsaBuf.buf, &header, pIoContext->m_wsaBuf.len);
	if (NULL != pData)
	{
		memmove(pIoContext->m_wsaBuf.buf + pIoContext->m_wsaBuf.len, pData, dataLen);
		pIoContext->m_wsaBuf.len += dataLen;// ���ݳ���Ϊͷ�� + ����֮��
	}

	DWORD	flags = 0;		//��־
	DWORD	sendBytes = 0;	//�����ֽ���
	if (WSASend(pIoContext->m_sockAccept, &(pIoContext->m_wsaBuf), 1, &sendBytes, flags, &(pIoContext->m_Overlapped), NULL) == SOCKET_ERROR)
	{
		if (ERROR_IO_PENDING != WSAGetLastError())//�����ص�����ʧ��
		{
			Loger(LEVEL_ERROR, perro("WSASend", WSAGetLastError()).c_str())
		}
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////
// ���ͻ��˵������Ϣ�洢��������
void IOCPServer::_AddToContextList(PER_SOCKET_CONTEXT *pSocketContext)
{
	EnterCriticalSection(&m_csClientSockContext);

	m_vectorClientSockContext.push_back(pSocketContext);

	LeaveCriticalSection(&m_csClientSockContext);
}

////////////////////////////////////////////////////////////////
//	�Ƴ�ĳ���ض���Context
void IOCPServer::_RemoveContext(PER_SOCKET_CONTEXT *pSocketContext)
{
	EnterCriticalSection(&m_csClientSockContext);
	for (int i = 0; i < m_vectorClientSockContext.size(); i++)
	{
		if (pSocketContext == m_vectorClientSockContext.at(i))
		{
			Loger(LEVEL_INFOR, "�ͻ��ˣ�%s:%d ���ӹرգ�", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
			m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d ���ӹرգ�", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));
			std::vector<PER_SOCKET_CONTEXT*>::iterator it = m_vectorClientSockContext.begin() + i;
			m_vectorClientSockContext.erase(it);
			RELEASE(pSocketContext);
			break;
		}
	}
	LeaveCriticalSection(&m_csClientSockContext);
}

////////////////////////////////////////////////////////////////
// ��տͻ�����Ϣ
void IOCPServer::_ClearContextList()
{
	EnterCriticalSection(&m_csClientSockContext);

	for (int i = 0; i < m_vectorClientSockContext.size(); i++)
	{
		delete m_vectorClientSockContext.at(i);
	}

	m_vectorClientSockContext.clear();

	LeaveCriticalSection(&m_csClientSockContext);
}

bool IOCPServer::HandleError(PER_SOCKET_CONTEXT *pContext, const DWORD& dwErr)
{
	// ����ǳ�ʱ�ˣ����ټ����Ȱ�  
	if (WAIT_TIMEOUT == dwErr)
	{
		// ȷ�Ͽͻ����Ƿ񻹻���...��
		//һ�㲻������������ΪGetQueuedCompletionStatus����ʱ�������INFINITE,����ʽ�ͻ��ˣ�ͨ�����ַ�ʽ�޷�������߰ε������
		if (!_IsSocketAlive(pContext->m_Socket))
		{
			Loger(LEVEL_ERROR, "��⵽�ͻ����쳣�˳���");
			this->_RemoveContext(pContext);
			return true;
		}
		else
		{
			Loger(LEVEL_ERROR, "���������ʱ��������...");
			return true;
		}
	}

	// �����ǿͻ����쳣�˳���
	else if (ERROR_NETNAME_DELETED == dwErr)
	{
		//this->_ShowMessage(_T("��⵽�ͻ����쳣�˳���"));
		// ��Ϣ���
		Loger(LEVEL_ERROR, "�ͻ��ˣ�%s:%d �쳣�˳�,������ֹ��%s", inet_ntoa(pContext->m_ClientAddr.sin_addr), ntohs(pContext->m_ClientAddr.sin_port), perro("", WSAGetLastError()).c_str());
		m_pMessageDlg->ShowMessage("�ͻ��ˣ�%s:%d �쳣�˳�,������ֹ��", inet_ntoa(pContext->m_ClientAddr.sin_addr), ntohs(pContext->m_ClientAddr.sin_port));
		this->_RemoveContext(pContext);
		return true;
	}

	else
	{
		//this->_ShowMessage(_T("��ɶ˿ڲ������ִ����߳��˳���������룺%d"), dwErr);
		// �磺���߱��ε�
		Loger(LEVEL_ERROR, "��ɶ˿ڲ������ִ���.%s", perro("", WSAGetLastError()).c_str());
		this->_RemoveContext(pContext);
		return false;
	}

}

/////////////////////////////////////////////////////////////////////
// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ���������һ����Ч��Socket��Ͷ��WSARecv����������쳣
// ʹ�õķ����ǳ��������socket�������ݣ��ж����socket���õķ���ֵ
// ��Ϊ����ͻ��������쳣�Ͽ�(����ͻ��˱������߰ε����ߵ�)��ʱ�򣬷����������޷��յ��ͻ��˶Ͽ���֪ͨ��
bool IOCPServer::_IsSocketAlive(SOCKET s)
{
	int nByteSent = send(s, "", 0, 0);
	if (-1 == nByteSent) return false;
	return true;
}

///////////////////////////////////////////////////////////////////
// ��ñ����д�����������
int IOCPServer::_GetNoOfProcessors()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;

}

// ��ȡ������Ϣ
bool IOCPServer::ReadConfig()
{
	const uint32_t nLen = 128;
	char szBuf[nLen] = { 0 };
	char szConfigPath[nLen] = { 0 };
	// ��������ļ��Ƿ����
	int _res = SearchPath(".\\", "UpdateConfigServer.ini", NULL, nLen, szConfigPath, NULL);
	if (_res == 0)
	{
		Loger(LEVEL_ERROR, perro("û���ҵ������ļ���UpdateConfigServer.ini", WSAGetLastError()).c_str());
		return false;
	}
	// ��ȡip��ַ�Ͷ˿ں�
	GetPrivateProfileString("TCP\\IP", "IP", DEFAULT_IP, szBuf, nLen, szConfigPath);
	m_strIP = szBuf;
	GetPrivateProfileString("TCP\\IP", "PORT", DEFAULT_PORT, szBuf, nLen, szConfigPath);
	m_nPort = atoi(szBuf);

	char szAppName[nLen] = { 0 };
	char szPwd[nLen] = { 0 };
	// ��ȡ����֮ǰ����ԭ�е��������
	m_mapUser2pwd.clear();
	// ��ȡ�û�����
	for (int i = 1; ; ++i)
	{
		sprintf(szAppName,"USER_%d",i);
		if (0 == GetPrivateProfileString(szAppName, "NAME", "", szBuf, nLen, szConfigPath))
		{
			// û�и�����
			if(0x02 == GetLastError())
				break;
		}
		// û���������룬��Ϊ��
		GetPrivateProfileString(szAppName, "PWD", "", szPwd, nLen, szConfigPath);
		m_mapUser2pwd[szBuf] = szPwd;
	}

	// ��ȡ��������֮ǰ����ԭ�е��������
	m_mapProName2Info.clear();
	// ��ȡ��������
	PROGRAMINFO programInfo = { 0 };
	for (int i = 1; ; ++i)
	{
		sprintf(szAppName, "PROGRAM_%d", i);
		// ��������
		if (0 == GetPrivateProfileString(szAppName, "NAME", "", programInfo.szProgramName, PROGRAM_INFO_LEN, szConfigPath))
		{
			// û�и�����
			if (0x02 == GetLastError())
				break;
		}
		// �ļ�·��
		if (0 == GetPrivateProfileString(szAppName, "FULL_PATH", "", programInfo.szFullPath, PROGRAM_INFO_LEN, szConfigPath))
		{
			// û�����ó����ļ�·��
			if (0x02 == GetLastError())
				continue;
		}

		// ���·���ϵ��ļ��Ƿ����
		if (!PathFileExists(programInfo.szFullPath))
		{
			// ��������ڣ��Ͳ����뵽map��
			continue;
		}

		// �汾��
		if (0 == GetPrivateProfileString(szAppName, "VERSION", "", programInfo.szProgramVersion, PROGRAM_INFO_LEN, szConfigPath))
		{
			// û�����ð汾��
			if (0x02 == GetLastError())
				continue;
		}

		m_mapProName2Info[programInfo.szProgramName] = programInfo;
	}
	return true;
}
