#pragma once
#include <winsock2.h>
#include <stdint.h>
#include <string>
#if defined IOCPAUTOUPDATE_EXPORTS || defined IOCPUPDATE_EXPORTS
#define IOCPAUTOUPDATE_API __declspec(dllexport)
#else
#define IOCPAUTOUPDATE_API __declspec(dllimport)
#endif

#define SEND_BUFF_LEN	1400
#define INFO_LEN		128

// 缓冲区长度 (1024*8)
// 之所以为什么设置8K，也是一个江湖上的经验值
// 如果确实客户端发来的每组数据都比较少，那么就设置得小一些，省内存
#define MAX_BUFFER_LEN        8192
// 用户名
typedef struct _USER {
	char szUserName[INFO_LEN];				// 用户名
	char szPassword[INFO_LEN];				// 用户密码
}USER;

// 客户端应用程序
typedef struct _tagClientProgramInfo
{
	char szProgramName[INFO_LEN];		// 程序起的名字
	char szProgramVersion[INFO_LEN];	// 程序版本号
	char szFilePath[INFO_LEN];			// 程序文件的下载路径
	char szFileName[INFO_LEN];			// 程序文件名称(包括后缀)
	uint32_t dwFileSize;				// 程序文件的大小
	uint32_t dwLeftLen;					// 接收时候，剩余的大小
	bool bStopDownLoad;					// 停止下载
	bool bSocketError;					// 连接出错
	bool bDownLoadFinish;				// 收到下载完成消息
}CLIENTPROGRAMINFO;

class /*IOCPAUTOUPDATE_API*/ SocketClient
{
public:
	SocketClient();
	virtual ~SocketClient();

	// 加载socket库
	bool LoadSockLib();

	// 卸载socket库
	void UnloadSockLib();

	// 读取配置信息
	bool ReadConfig();

	//客户端初始化，连接服务器
	bool start();

	// 停止客户端，关闭socket
	void stop();

	//1 登录
	bool Login();

	//2 检查版本信息.
	// 返回值：-1 发送下载命令失败，没有更新；-2 更新文件已下载；0 有新版本可供下载。
	int32_t ProgramVer();

	//3 是否需要更新,入参true下载，false不下载。
	bool DownLoad(bool bDownLoad);

	// 网络连接是否出错，在下载过程中判断网络出错。
	bool IsSocketError();

	// 下载是否完成
	bool IsDownLoadFinish();

	// 获取下载文件的完整路径
	std::string GetFullFilePath();

	// 获取下载剩余的百分比
	int32_t GetDownLoadLeftPercent();
	
	// 获取文件尺寸M 为单位
	double GetFileSize();

	// 获取剩余下载数据量，byte为单位
	int32_t GetDownLoadLeftSize();

	// 停止下载
	void StopDownLoad(bool bStopDownLoad = true);

private:
	// 下载完成确认信息
	bool DownLoadFinish();

	// 下载文件数据
	bool GetFile();

	//原始的数据传输函数
	bool SendData(uint32_t dwCmdID, void* pData, uint32_t nLen);
	bool RecvData(void*  pData, uint32_t nLen);
	bool SendRaw(void*  pData, uint32_t nLen);
private:
	std::string						m_strIP;						// 服务器端的IP地址
	unsigned short					m_nPort;						// 服务器端的监听端口
	bool							m_bLibLoaded;					// socket库是否被加载
	SOCKET							m_hClientSock;					// socket套接字
	char							m_szSendBuf[SEND_BUFF_LEN];		// 用于拼接字符串
	USER							m_User;							// 用户信息
	CLIENTPROGRAMINFO				m_ClientProgramInfo;			// 软件信息
};

