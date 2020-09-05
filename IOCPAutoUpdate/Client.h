#pragma once
#if defined IOCPAUTOUPDATE_EXPORTS || defined IOCPUPDATE_EXPORTS
#define IOCPAUTOUPDATE_API __declspec(dllexport)
#else
#define IOCPAUTOUPDATE_API __declspec(dllimport)
#endif
#include <stdint.h>
class SocketClient;
class IOCPAUTOUPDATE_API Client
{
public:
	Client();
	virtual ~Client();
	// ////////////////没有弹出框的情况//////////////////////////////////
	/**
	* 入参配置的 appName
	* 返回值：0 有新版本可以下载, 1 有新版文件，但是本地有与新版本文件名相同的本地文件存在。2 没有新版本
	*		  3 socket库启动失败, 4 连接服务器失败， 5 登录验证失败, 6 网络异常
	*/
	int32_t Authenticate(const char* pAppName);

	// 成功下载返回true, 失败返回false。这个函数是阻塞的
	bool DownLoad(const char* pAppName, const bool bDownLoad = true, const bool bShowDlg = false);

	// 获取文件大小，以M为单位，需要在Authenticate调用成功后调用
	double GetFileSize();

	// 需要另起线程调用，还有剩多少数据未下载的百分比。如果完全还没下载数据就返回100，下载完成返回0;
	int32_t GetDownLoadLeftPercent();
	// socket网络错误，常见网络超时或者断网, true出错
	bool IsSocketError();

	// 停止下载线程
	void StopDownLoad();
	
	// /////////////////////// 有C++ MessageBox,控件颜色不能调整/////////////////////////////////
	bool DownLoadFile(const char* pAppName, const bool bShowDlg = true, const HWND hMainWnd = nullptr);
	// 下载成功，获取到的是新下载下来的文件路径
	void GetFullFilePath(char* pszFilePath, int nbufFilePathLen);
	// 下载成功项目名称
	void GetProgramName(char* pszProgramName, int nbufProgramNameLen);
	// 获取文件的版本号。下载成功后则是新下载文件的版本号，否则是之前读取配置时候的文件版本号
	uint32_t GetFileVersion();
private:
	static DWORD WINAPI WorkerThread(LPVOID lpParam);
	SocketClient* m_pSockClient;
};

