#pragma once
#if defined IOCPAUTOUPDATE_EXPORTS || defined IOCPUPDATE_EXPORTS
#define IOCPAUTOUPDATE_API __declspec(dllexport)
#else
#define IOCPAUTOUPDATE_API __declspec(dllimport)
#endif
#include <string>
class SocketClient;
class IOCPAUTOUPDATE_API Client
{
public:
	Client();
	virtual ~Client();
	bool ConnectServer(HWND hMainWnd);
	std::string GetFullFilePath();
private:
	static DWORD WINAPI WorkerThread(LPVOID lpParam);
	SocketClient* m_pSockClient;
};

