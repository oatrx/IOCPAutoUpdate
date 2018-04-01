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

// ���������� (1024*8)
// ֮����Ϊʲô����8K��Ҳ��һ�������ϵľ���ֵ
// ���ȷʵ�ͻ��˷�����ÿ�����ݶ��Ƚ��٣���ô�����õ�СһЩ��ʡ�ڴ�
#define MAX_BUFFER_LEN        8192
// �û���
typedef struct _USER {
	char szUserName[INFO_LEN];				// �û���
	char szPassword[INFO_LEN];				// �û�����
}USER;

// �ͻ���Ӧ�ó���
typedef struct _tagClientProgramInfo
{
	char szProgramName[INFO_LEN];		// �����������
	char szProgramVersion[INFO_LEN];	// ����汾��
	char szFilePath[INFO_LEN];			// �����ļ�������·��
	char szFileName[INFO_LEN];			// �����ļ�����(������׺)
	uint32_t dwFileSize;				// �����ļ��Ĵ�С
	uint32_t dwLeftLen;					// ����ʱ��ʣ��Ĵ�С
	bool bStopDownLoad;					// ֹͣ����
	bool bSocketError;					// ���ӳ���
	bool bDownLoadFinish;				// �յ����������Ϣ
}CLIENTPROGRAMINFO;

class /*IOCPAUTOUPDATE_API*/ SocketClient
{
public:
	SocketClient();
	virtual ~SocketClient();

	// ����socket��
	bool LoadSockLib();

	// ж��socket��
	void UnloadSockLib();

	// ��ȡ������Ϣ
	bool ReadConfig();

	//�ͻ��˳�ʼ�������ӷ�����
	bool start();

	// ֹͣ�ͻ��ˣ��ر�socket
	void stop();

	//1 ��¼
	bool Login();

	//2 ���汾��Ϣ.
	// ����ֵ��-1 ������������ʧ�ܣ�û�и��£�-2 �����ļ������أ�0 ���°汾�ɹ����ء�
	int32_t ProgramVer();

	//3 �Ƿ���Ҫ����,���true���أ�false�����ء�
	bool DownLoad(bool bDownLoad);

	// ���������Ƿ���������ع������ж��������
	bool IsSocketError();

	// �����Ƿ����
	bool IsDownLoadFinish();

	// ��ȡ�����ļ�������·��
	std::string GetFullFilePath();

	// ��ȡ����ʣ��İٷֱ�
	int32_t GetDownLoadLeftPercent();
	
	// ��ȡ�ļ��ߴ�M Ϊ��λ
	double GetFileSize();

	// ��ȡʣ��������������byteΪ��λ
	int32_t GetDownLoadLeftSize();

	// ֹͣ����
	void StopDownLoad(bool bStopDownLoad = true);

private:
	// �������ȷ����Ϣ
	bool DownLoadFinish();

	// �����ļ�����
	bool GetFile();

	//ԭʼ�����ݴ��亯��
	bool SendData(uint32_t dwCmdID, void* pData, uint32_t nLen);
	bool RecvData(void*  pData, uint32_t nLen);
	bool SendRaw(void*  pData, uint32_t nLen);
private:
	std::string						m_strIP;						// �������˵�IP��ַ
	unsigned short					m_nPort;						// �������˵ļ����˿�
	bool							m_bLibLoaded;					// socket���Ƿ񱻼���
	SOCKET							m_hClientSock;					// socket�׽���
	char							m_szSendBuf[SEND_BUFF_LEN];		// ����ƴ���ַ���
	USER							m_User;							// �û���Ϣ
	CLIENTPROGRAMINFO				m_ClientProgramInfo;			// �����Ϣ
};

