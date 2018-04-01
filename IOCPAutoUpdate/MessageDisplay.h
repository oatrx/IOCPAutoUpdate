#pragma once

#if defined IOCPAUTOUPDATE_EXPORTS || defined IOCPUPDATE_EXPORTS
#define IOCPAUTOUPDATE_API __declspec(dllexport)
#else
#define IOCPAUTOUPDATE_API __declspec(dllimport)
#endif

#define MESSAGE_BUF_LEN	2048
class IOCPAUTOUPDATE_API MessageDisplay
{
public:
	MessageDisplay();
	virtual ~MessageDisplay();
	void ShowMessage(const char* cFormat, ...);
	virtual void AddMessage(const char* message) = 0;
private:
	char m_szBuff[MESSAGE_BUF_LEN];
};

