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
	// ////////////////û�е���������//////////////////////////////////
	/**
	* ������õ� appName
	* ����ֵ��0 ���°汾��������, 1 ���°��ļ������Ǳ��������°汾�ļ�����ͬ�ı����ļ����ڡ�2 û���°汾
	*		  3 socket������ʧ��, 4 ���ӷ�����ʧ�ܣ� 5 ��¼��֤ʧ��, 6 �����쳣
	*/
	int32_t Authenticate(const char* pAppName);

	// �ɹ����ط���true, ʧ�ܷ���false�����������������
	bool DownLoad(const char* pAppName, const bool bDownLoad = true, const bool bShowDlg = false);

	// ��ȡ�ļ���С����MΪ��λ����Ҫ��Authenticate���óɹ������
	double GetFileSize();

	// ��Ҫ�����̵߳��ã�����ʣ��������δ���صİٷֱȡ������ȫ��û�������ݾͷ���100��������ɷ���0;
	int32_t GetDownLoadLeftPercent();
	// socket������󣬳������糬ʱ���߶���, true����
	bool IsSocketError();

	// ֹͣ�����߳�
	void StopDownLoad();
	
	// /////////////////////// ��C++ MessageBox,�ؼ���ɫ���ܵ���/////////////////////////////////
	bool DownLoadFile(const char* pAppName, const bool bShowDlg = true, const HWND hMainWnd = nullptr);
	// ���سɹ�����ȡ�������������������ļ�·��
	void GetFullFilePath(char* pszFilePath, int nbufFilePathLen);
	// ���سɹ���Ŀ����
	void GetProgramName(char* pszProgramName, int nbufProgramNameLen);
	// ��ȡ�ļ��İ汾�š����سɹ��������������ļ��İ汾�ţ�������֮ǰ��ȡ����ʱ����ļ��汾��
	uint32_t GetFileVersion();
private:
	static DWORD WINAPI WorkerThread(LPVOID lpParam);
	SocketClient* m_pSockClient;
};

