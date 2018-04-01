#pragma once
#include <winsock2.h>
#include <MSWSock.h>
#include <vector>
#include <string>
#include <cassert>
#include <map>
#if defined IOCPAUTOUPDATE_EXPORTS || defined IOCPUPDATE_EXPORTS
#define IOCPAUTOUPDATE_API __declspec(dllexport)
#else
#define IOCPAUTOUPDATE_API __declspec(dllimport)
#endif

// 缓冲区长度 (1024*8)
// 之所以为什么设置8K，也是一个江湖上的经验值
// 如果确实客户端发来的每组数据都比较少，那么就设置得小一些，省内存
#define MAX_BUFFER_LEN        8192 
// 默认的context 个数
#define DEFAULT_CLIENT_CONTEXT_LEN	32
// 软件信息长度
#define PROGRAM_INFO_LEN	128


//////////////////////////////////////////////////////////////////
// 在完成端口上投递的I/O操作的类型
typedef enum _OPERATION_TYPE
{
	ACCEPT_LOGIN,						// 标志投递的Accept操作,并获取第一组数据登录数据
	LOGIN_RET,							// 登录验证结果返回
	PROGRAM_VER,						// 原始版本信息
	PROGRAM_VER_RET,					// 版本信息校验返回
	DOWNLOAD,							// 下载确认
	DOWNLOAD_DATA,						// 下载返回
	DOWNLOAD_END,						// 传输文件数据
	NULL_POSTED							// 用于初始化，无意义
}OPERATION_TYPE;

//====================================================================================
//
//				单IO数据结构体定义(用于每一个重叠操作的参数)
//
//====================================================================================

typedef struct _PER_IO_CONTEXT
{
	OVERLAPPED		m_Overlapped;                               // 每一个重叠网络操作的重叠结构(针对每一个Socket的每一个操作，都要有一个)              
	SOCKET			m_sockAccept;                               // 这个网络操作所使用的Socket
	bool			m_bLoginSuss;								// 登录是否成功
	bool			m_bNewVersionVerify;						// 版本校验，是否有新版本
	WSABUF			m_wsaBuf;                                   // WSA类型的缓冲区，用于给重叠操作传参数的
	char			m_szBuffer[MAX_BUFFER_LEN];                 // 这个是WSABUF里具体存字符的缓冲区
	OPERATION_TYPE	m_OpType;                                   // 标识网络操作的类型(对应上面的枚举)
	HANDLE			m_hProFile;									// 程序文件的操作句柄
	uint32_t		m_nProFileSize;								// 程序文件大小
	char			m_szProFullPath[PROGRAM_INFO_LEN];			// 程序文件全路径

															   // 初始化
	_PER_IO_CONTEXT()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
		ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
		m_sockAccept = INVALID_SOCKET;
		m_bLoginSuss = false;
		m_bNewVersionVerify = false;
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = MAX_BUFFER_LEN;
		m_OpType = NULL_POSTED;
		m_hProFile = INVALID_HANDLE_VALUE;
		m_nProFileSize = 0;
		ZeroMemory(m_szProFullPath, sizeof(m_szProFullPath));
	}
	// 释放掉Socket
	~_PER_IO_CONTEXT()
	{
		if (m_sockAccept != INVALID_SOCKET)
		{
			closesocket(m_sockAccept);
			m_sockAccept = INVALID_SOCKET;
		}
		if (INVALID_HANDLE_VALUE != m_hProFile)
		{
			CloseHandle(m_hProFile);
			m_hProFile = INVALID_HANDLE_VALUE;
		}
		m_bLoginSuss = false;
		m_bNewVersionVerify = false;
	}
	// 重置缓冲区内容
	void ResetBuffer()
	{
		ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
		m_wsaBuf.len = MAX_BUFFER_LEN;
	}

} PER_IO_CONTEXT, *PPER_IO_CONTEXT;


//====================================================================================
//
//				单句柄数据结构体定义(用于每一个完成端口，也就是每一个Socket的参数)
//
//====================================================================================

typedef struct _PER_SOCKET_CONTEXT
{
	SOCKET      m_Socket;                                  // 每一个客户端连接的Socket
	SOCKADDR_IN m_ClientAddr;                              // 客户端的地址
	std::vector<_PER_IO_CONTEXT*> m_vectorIoContext;			// 客户端网络操作的上下文数据，
															// 也就是说对于每一个客户端Socket，是可以在上面同时投递多个IO请求的
	_PER_SOCKET_CONTEXT()
	{
		m_vectorIoContext.reserve(DEFAULT_CLIENT_CONTEXT_LEN);
		m_Socket = INVALID_SOCKET;
		memset(&m_ClientAddr, 0, sizeof(m_ClientAddr));
	}

	// 释放资源
	~_PER_SOCKET_CONTEXT()
	{
		if (m_Socket != INVALID_SOCKET)
		{
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
		}
		// 释放掉所有的IO上下文数据
		for (int i = 0; i < m_vectorIoContext.size(); i++)
		{
			delete m_vectorIoContext.at(i);
		}
		m_vectorIoContext.clear();
	}

	// 获取一个新的IoContext
	_PER_IO_CONTEXT* GetNewIoContext()
	{
		_PER_IO_CONTEXT* p = new _PER_IO_CONTEXT;

		m_vectorIoContext.push_back(p);

		return p;
	}

	// 从数组中移除一个指定的IoContext
	void RemoveContext(_PER_IO_CONTEXT* pContext)
	{
		assert(pContext != NULL);

		for (int i = 0; i < m_vectorIoContext.size(); i++)
		{
			if (pContext == m_vectorIoContext.at(i))
			{
				delete pContext;
				pContext = NULL;
				std::vector<_PER_IO_CONTEXT*>::iterator it = m_vectorIoContext.begin() + i;
				m_vectorIoContext.erase(it);
				break;
			}
		}
	}

} PER_SOCKET_CONTEXT, *PPER_SOCKET_CONTEXT;

// 工作者线程的线程参数
class IOCPServer;
typedef struct _tagThreadParams
{
	IOCPServer*	pIOCPServer;										// 类指针，用于调用类中的函数
	int         nThreadNo;										// 线程编号
} THREADPARAMS, *PTHREADPARAMS;

// 软件信息
typedef struct _tagProgramInfo
{
	char szProgramName[PROGRAM_INFO_LEN];		// 程序起的名字
	char szProgramVersion[PROGRAM_INFO_LEN];	// 程序版本号
	char szFullPath[PROGRAM_INFO_LEN];			// 程序文件的全路径
}PROGRAMINFO;


class MessageDisplay;
class /*IOCPAUTOUPDATE_API*/ IOCPServer
{
public:
	IOCPServer(MessageDisplay* pMessageDlg);
	virtual ~IOCPServer();

	// 加载socket库
	bool LoadSockLib();

	// 卸载socket库
	void UnloadSockLib();

	// 启动服务器
	bool Start();

	//	停止服务器
	void Stop();

	// 获得本机的IP地址
	std::string GetLocalIP();

private:
	// 线程函数，为IOCP请求服务的工作者线程
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);

	// 初始化IOCP
	bool _InitializeIOCP();

	// 初始化Socket
	bool _InitializeListenSocket();

	// 最后释放资源
	void _DeInitialize();

	// 在有客户端连入的时候，进行处理(新建一socket)
	bool _DoAccpetLogin(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext);

	// 投递Accept请求
	bool _PostAccept(PER_IO_CONTEXT* pAcceptIoContext);

	// 将句柄绑定到完成端口中(例如：将accept后的socket绑定到完成端口)
	bool _AssociateWithIOCP(PER_SOCKET_CONTEXT *pContext);

	// 投递接受数据的请求
	bool _DoPostRecv(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext,OPERATION_TYPE opType);

	// 投递接收数据请求
	bool _PostRecv(PER_IO_CONTEXT* pIoContext);

	// 收到版本信息，校验版本号
	bool _DoProgramVersion(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext);

	// 收到下载确认信息后，处理
	bool _DoDownLoad(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext);

	// 传输文件数据
	bool _DoTransmitFile(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext);

	// 下载完成
	bool _DoDownLoadFinish(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext);

	// 投递数据发送请求
	bool _PostSend(PER_IO_CONTEXT* pIoContext,uint32_t dwCmdID, LPVOID pDate, uint32_t dataLen);

	// 将客户端的相关信息存储到数组中
	void _AddToContextList(PER_SOCKET_CONTEXT *pSocketContext);

	// 将客户端的信息从数组中移除
	void _RemoveContext(PER_SOCKET_CONTEXT *pSocketContext);

	// 清空客户端信息
	void _ClearContextList();

	// 处理完成端口上的错误
	bool HandleError(PER_SOCKET_CONTEXT *pContext, const DWORD& dwErr);

		// 判断客户端Socket是否已经断开
	bool _IsSocketAlive(SOCKET s);

	// 获得本机的处理器数量
	int _GetNoOfProcessors();

public:
	// 读取配置信息
	bool ReadConfig();

private:
	int								m_nThreads;						// 生成的线程数量
	HANDLE*							m_phWorkerThreads;				// 工作者线程的句柄指针
	HANDLE							m_hIOCompletionPort;			// 完成端口的句柄
	HANDLE							m_hShutdownEvent;				// 用来通知线程系统退出的事件，为了能够更好的退出线程
	bool							m_bLibLoaded;					// socket库是否被加载
	CRITICAL_SECTION				m_csClientSockContext;               // 用于Worker线程同步的互斥量

	std::string						m_strIP;						// 服务器端的IP地址
	unsigned short					m_nPort;                       // 服务器端的监听端口

	std::map<std::string, std::string>	m_mapUser2pwd;				//  用户名和密码map
	std::map<std::string, PROGRAMINFO>	m_mapProName2Info;			// 软件名称和软件详细信息map

	std::vector<PER_SOCKET_CONTEXT*> m_vectorClientSockContext;          // 客户端Socket的Context信息        

	PER_SOCKET_CONTEXT*				m_pListenContext;              // 用于监听的Socket的Context信息
	
	MessageDisplay*					m_pMessageDlg;					// 信息展示

	// 函数指针在构造函数中初始为NULL，Run-time stack #2错误
	LPFN_ACCEPTEX					m_lpfnAcceptEx;					// AcceptEx的函数指针，用于调用这个扩展函数
	LPFN_GETACCEPTEXSOCKADDRS		m_lpfnGetAcceptExSockAddrs;		// GetAcceptExSockaddrs 的函数指针
	//
	LPFN_TRANSMITFILE				m_lpfnTransmitFile;				// TransmitFile 的函数指针。
};

