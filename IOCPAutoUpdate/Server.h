#pragma once
#if defined IOCPAUTOUPDATE_EXPORTS || defined IOCPUPDATE_EXPORTS
#define IOCPAUTOUPDATE_API __declspec(dllexport)
#else
#define IOCPAUTOUPDATE_API __declspec(dllimport)
#endif
class MessageDisplay;
class IOCPServer;
class IOCPAUTOUPDATE_API Server
{
public:
	Server();
	virtual ~Server();
	bool start(MessageDisplay* pMessageDlg);
	void stop();
private:
	IOCPServer* m_pIOCPServer;
};

