#include "stdafx.h"
#include "SocketClient.h"
#include "Header.h"
#include <iostream>
#include "fstream"
#include "Log.h"
#include <cassert>
#include <cstring>
#include "shlwapi.h"
#include "verStr2uint32.h"
using namespace std;

// Ĭ�϶˿�
#define DEFAULT_PORT          "12345"
// Ĭ��IP��ַ
#define DEFAULT_IP            _T("127.0.0.1")
#pragma comment(lib, "Shlwapi.lib")
SocketClient::SocketClient()
	:m_bLibLoaded(false),
	m_hClientSock(INVALID_SOCKET)
{
	Log::getInstance().init(LEVEL_INFOR, "updateClient");
	ZeroMemory(m_szSendBuf, sizeof(m_szSendBuf));
	ZeroMemory(&m_User, sizeof(m_User));
	ZeroMemory(&m_ClientProgramInfo, sizeof(m_ClientProgramInfo));
	m_ClientProgramInfo.bStopDownLoad = false;
	m_ClientProgramInfo.bSocketError = false;
	m_ClientProgramInfo.bDownLoadFinish = false;
}

SocketClient::~SocketClient()
{
	stop();
	UnloadSockLib();
}

bool SocketClient::LoadSockLib()
{
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (NO_ERROR != nResult)
	{
		Loger(LEVEL_ERROR, perro("WSAStartup", WSAGetLastError()).c_str());
		return  m_bLibLoaded = false;
	}
	return m_bLibLoaded = true;
}

bool SocketClient::IsLibLoaded()
{
	return m_bLibLoaded;
}

void SocketClient::UnloadSockLib()
{
	if (m_bLibLoaded)
	{
		m_bLibLoaded = false;
		WSACleanup();
	}
}

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//Project properties->Configuration Properties->C / C++->General->SDL checks->No
//����socket�����ӷ�����
bool SocketClient::start(const char* pAppName)
{
	// �����ļ�������
	if (!ReadConfig(pAppName))
		return false;

	// �����û�йرյ����ӣ��ȹر�����
	stop();

	m_hClientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_hClientSock == INVALID_SOCKET)
	{
		Loger(LEVEL_ERROR, perro("����socketʧ��.", WSAGetLastError()).c_str());
		return false;
	}

	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	//��ע��˿ںţ�1024~49151
	addr.sin_port = htons(m_nPort);
	addr.sin_addr.S_un.S_addr = inet_addr(m_strIP.c_str());
	if (0 != connect(m_hClientSock, (sockaddr *)&addr, sizeof(addr)))
	{
		Loger(LEVEL_ERROR, perro("���ӷ�����ʧ��.", WSAGetLastError()).c_str());
		return false;
	}

	int nNetTimeout = 7000; // 7s �������ݻ��߽�������ʧ�ܡ�������ܾͳ��ִ����ˡ�
	//bool bKeepAlive = true;
	int nRet = setsockopt(m_hClientSock, SOL_SOCKET, SO_SNDTIMEO, (char*)&nNetTimeout, sizeof(nNetTimeout));
	if (SOCKET_ERROR == nRet)
	{
		Loger(LEVEL_ERROR, perro("���÷��ͳ�ʱʱ��ʧ��", WSAGetLastError()).c_str());
		return false;
	}

	//bool bKeepAlive = true;
	nRet = setsockopt(m_hClientSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nNetTimeout, sizeof(nNetTimeout));
	if (SOCKET_ERROR == nRet)
	{
		Loger(LEVEL_ERROR, perro("���ý��ճ�ʱʱ��ʧ��", WSAGetLastError()).c_str());
		return false;
	}
	return true;
}

void SocketClient::stop()
{
	if (INVALID_SOCKET != m_hClientSock)
	{
		closesocket(m_hClientSock);
		m_hClientSock = INVALID_SOCKET;
	}
}

//��¼
bool SocketClient::Login(){
	sLOGIN login = { 0 };
	strcpy(login.szUserName, m_User.szUserName);
	strcpy(login.szPassword, m_User.szPassword);
	if (SendData(UPDATE_NET_CMD_LOGIN, &login, sizeof(login)))
	{
		sHEADER header = { 0 };
		RecvData(&header, sizeof(header));
		sLOGINRET loginret = { 0 };
		RecvData(&loginret, sizeof(loginret));
		return loginret.dwSuccess == 1 ? true : false;
	}
	return false;
}

//��ȡ�汾�ţ��ļ���Ϣ
int32_t SocketClient::ProgramVer()
{
	sPROGRAMVER programVer = { 0 };
	strcpy(programVer.szProgramName, m_ClientProgramInfo.szProgramName);
	programVer.dwVer = m_ClientProgramInfo.dwOldVersion;
	if (SendData(UPDATE_NET_CMD_PROGRAM_VER, &programVer, sizeof(programVer)))
	{
		sHEADER header = { 0 };
		RecvData(&header, sizeof(header));
		sPROGRAMVERRET programVerRet = { 0 };
		RecvData(&programVerRet, sizeof(programVerRet));
		if (programVerRet.dwNewVer > m_ClientProgramInfo.dwOldVersion)
		{
			// �°����������
			m_ClientProgramInfo.dwNewVersion = programVerRet.dwNewVer;

			strcpy(m_ClientProgramInfo.szFileName, programVerRet.szFileName);
			m_ClientProgramInfo.dwFileSize = programVerRet.dwFileSize;
			m_ClientProgramInfo.dwLeftLen = programVerRet.dwFileSize;
			// �����ļ��Ƿ����
			string strFullPath(m_ClientProgramInfo.szFilePath);
			strFullPath += m_ClientProgramInfo.szFileName;
			if (PathFileExists(strFullPath.c_str()))
				return -2;
			else
				return 0;
		}
	}
	return -1;
}

bool SocketClient::DownLoad(bool bDownLoad)
{
	sDOWNLOAD downLoad = { 0 };
	downLoad.dwDownLoad = bDownLoad ? 1 : 0;
	if (SendData(UPDATE_NET_CMD_DOWNLOAD, &downLoad, sizeof(downLoad)))
	{
		// �����أ��Ͳ�ȥ��������
		if (!bDownLoad)
			return false;
		// �ɹ�����
		if (GetFile())
			return m_ClientProgramInfo.bDownLoadFinish = DownLoadFinish();
	}
	return false;
}

bool SocketClient::IsSocketError()
{
	return m_ClientProgramInfo.bSocketError;
}

bool SocketClient::IsDownLoadFinish()
{
	return m_ClientProgramInfo.bDownLoadFinish;
}

std::string SocketClient::GetFullFilePath()
{
	string strFullPath(m_ClientProgramInfo.szFilePath);
	strFullPath += m_ClientProgramInfo.szFileName;
	return strFullPath;
}

std::string SocketClient::GetProgramName()
{
	return string(m_ClientProgramInfo.szProgramName);
}

uint32_t SocketClient::GetFileVersion()
{
	return m_ClientProgramInfo.dwOldVersion;
}

// ��ȡ����ʣ��İٷֱ�
int32_t SocketClient::GetDownLoadLeftPercent()
{
	return 100 * (double)m_ClientProgramInfo.dwLeftLen / ((m_ClientProgramInfo.dwFileSize > 0) ? m_ClientProgramInfo.dwFileSize : 1);
}

double SocketClient::GetFileSize()
{
	return (double)m_ClientProgramInfo.dwFileSize / 1024 / 1024;
}

int32_t SocketClient::GetDownLoadLeftSize()
{
	return m_ClientProgramInfo.dwLeftLen;
}

// ֹͣ����
void SocketClient::StopDownLoad(bool bStopDownLoad /*= true*/)
{
	m_ClientProgramInfo.bStopDownLoad = bStopDownLoad;
}

bool SocketClient::DownLoadFinish()
{
	sHEADER header = { 0 };
	RecvData(&header, sizeof(header));
	if (UPDATE_NET_CMD_DOWNLOAD_RET == header.dwCmd)
	{
		sDOWNLOADRET downLoadRet = { 0 };
		RecvData(&downLoadRet, sizeof(downLoadRet));
		return downLoadRet.dwDownLoadFinish == 1 ? true : false;
	}
	return false;
}

//�����ļ�
bool SocketClient::GetFile()
{
	string strFullPath(m_ClientProgramInfo.szFilePath);
	strFullPath += m_ClientProgramInfo.szFileName;
	HANDLE m_hProFile = CreateFile(strFullPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if (INVALID_HANDLE_VALUE == m_hProFile)
	{
		Loger(LEVEL_ERROR, perro("���������ļ�ʧ��.", GetLastError()).c_str());
		return false;
	}
	CHAR szBuff[MAX_BUFFER_LEN] = { 0 };
	DWORD dwBytesToWrite = 0;
	DWORD dwBytesWrited = 0;
	CHAR* pszBuf;
	m_ClientProgramInfo.dwLeftLen = m_ClientProgramInfo.dwFileSize;
	// �������� ��û��ֹͣ���أ��ͼ������ء�
	while (m_ClientProgramInfo.dwLeftLen > 0 && !m_ClientProgramInfo.bStopDownLoad){
		int nRecv = MAX_BUFFER_LEN;
		if (m_ClientProgramInfo.dwLeftLen < nRecv)
		{
			nRecv = m_ClientProgramInfo.dwLeftLen;
		}
		if (!RecvData(szBuff, nRecv))
		{
			Loger(LEVEL_ERROR, perro("�ļ����ع�����ʧ��.", GetLastError()).c_str());
			CloseHandle(m_hProFile);
			return false;
		}
		m_ClientProgramInfo.dwLeftLen = m_ClientProgramInfo.dwLeftLen - nRecv;
		// д�ļ�
		dwBytesToWrite = nRecv;
		dwBytesWrited = 0;
		pszBuf = szBuff;
		do { //ѭ��д�ļ���ȷ���������ļ���д��
			WriteFile(m_hProFile, pszBuf, dwBytesToWrite, &dwBytesWrited, NULL);
			dwBytesToWrite -= dwBytesWrited;
			pszBuf += dwBytesWrited;
		} while (dwBytesToWrite > 0);
	}
	CloseHandle(m_hProFile);
	return true;
}

bool SocketClient::RecvData(void*  pData, uint32_t nLen){
	INT nLeft = nLen;
	LPSTR pTemp = (LPSTR)pData;
	while (nLeft > 0) {
		int nRev = recv(m_hClientSock, pTemp, nLeft, 0);
		if ((nRev == SOCKET_ERROR) || (nRev == 0))
		{
			m_ClientProgramInfo.bSocketError = true;// ��������־
			Loger(LEVEL_ERROR, perro("���ݽ���ʧ��.", WSAGetLastError()).c_str());
			return false;
		}
		pTemp = pTemp + nRev;
		nLeft = nLeft - nRev;
	}
	return true;
}

bool SocketClient::SendData(uint32_t dwCmdID, void* pData, uint32_t nLen)
{
	sHEADER header = { 0 };
	header.dwCmd = dwCmdID;
	header.dwLen = nLen;
	uint32_t nSendLen = sizeof(header);
	// ����ͷ
	memmove(m_szSendBuf, &header, nSendLen);
	// ����
	memmove(m_szSendBuf + nSendLen, pData, nLen);
	nSendLen += nLen;
	//�������ݰ��İ�ͷ �� ���ݣ�������Ϊ��ɶ˿ڣ���Ҫһ���Է�������
	if (SendRaw(m_szSendBuf, nSendLen) != true)
	{
		return false;
	}
	return true;
}
bool SocketClient::SendRaw(void*  pData, uint32_t nLen){
	INT nLeft = nLen;
	LPSTR pTemp = (LPSTR)pData;
	while (nLeft > 0){
		int nSend = send(m_hClientSock, pTemp, nLeft, 0);
		if (nSend == SOCKET_ERROR)
		{
			m_ClientProgramInfo.bSocketError = true;// ��������־
			Loger(LEVEL_ERROR, perro("���ݷ���ʧ��.", WSAGetLastError()).c_str());
			return false;
		}
		pTemp = pTemp + nSend;
		nLeft = nLeft - nSend;
	}
	return true;
}

// ����Ĭ�ϵ������ļ�
void SocketClient::CreateDefaultConfig(const char* pAppName, const char* szConfigFilePath)
{
	WritePrivateProfileString("TCP\\IP", "SERVER_IP", "180.168.141.242", szConfigFilePath);
	WritePrivateProfileString("TCP\\IP", "SERVER_PORT", "8888", szConfigFilePath);

	WritePrivateProfileString("USER", "NAME", "dta",  szConfigFilePath);
	WritePrivateProfileString("USER", "PWD",  "dta",  szConfigFilePath);

	char szName[128];
	memset(szName, 0, sizeof(szName));
	strncpy(szName, pAppName, strlen(pAppName) >= sizeof(szName) ? sizeof(szName) - 1 : strlen(pAppName));
	_strlwr(szName);
	WritePrivateProfileString(pAppName, "NAME", szName, szConfigFilePath);
	WritePrivateProfileString(pAppName, "PATH", ".\\Update\\", szConfigFilePath);
	WritePrivateProfileString(pAppName, "FILE_NAME", "", szConfigFilePath);
	WritePrivateProfileString(pAppName, "VERSION", "1.0.0", szConfigFilePath);
}

// ��ȡ������Ϣ
bool SocketClient::ReadConfig(const char*  pAppName)
{
	const uint32_t nLen = 128;
	char szBuf[nLen] = { 0 };
	char szConfigPath[nLen] = { 0 };
	char* pszDir = ".\\config\\";
	char* pszConfigFile = "UpdateConfigClient.ini";
	// ��������ļ��Ƿ����
	int _res = SearchPath(pszDir, pszConfigFile, NULL, nLen, szConfigPath, NULL);
	if (_res == 0)
	{
		std::string strInfo("û���ҵ������ļ���");
		strInfo += pszConfigFile;
		Loger(LEVEL_ERROR, perro(strInfo.c_str(), WSAGetLastError()).c_str());
		strInfo = pszDir;
		strInfo += pszConfigFile;
		CreateDefaultConfig(pAppName, strInfo.c_str());
		return false;
	}

	// ��ȡip��ַ�Ͷ˿ں�
	GetPrivateProfileString("TCP\\IP", "SERVER_IP", DEFAULT_IP, szBuf, nLen, szConfigPath);
	m_strIP = szBuf;
	GetPrivateProfileString("TCP\\IP", "SERVER_PORT", DEFAULT_PORT, szBuf, nLen, szConfigPath);
	m_nPort = atoi(szBuf);

	// ��ȡ�û�����
	if (0 == GetPrivateProfileString("USER", "NAME", "", m_User.szUserName, INFO_LEN, szConfigPath))
	{
		// û�и�����
		if (0x02 == GetLastError())
		{
			return false;
		}
	}
	// û���������룬��Ϊ��
	GetPrivateProfileString("USER", "PWD", "", m_User.szPassword, INFO_LEN, szConfigPath);

	// �ͻ��˳�������
	// ��������
	if (0 == GetPrivateProfileString(pAppName, "NAME", "", m_ClientProgramInfo.szProgramName, INFO_LEN, szConfigPath))
	{
		// û�и�����
		if (0x02 == GetLastError())
		{
			return false;
		}
	}
	// �ļ�·��
	GetPrivateProfileString(pAppName, "PATH", ".\\", m_ClientProgramInfo.szFilePath, INFO_LEN, szConfigPath);

	char szProgramVersion[INFO_LEN];
	// �汾��
	GetPrivateProfileString(pAppName, "VERSION", "", szProgramVersion /*m_ClientProgramInfo.szProgramVersion */, INFO_LEN, szConfigPath);
	
	// ��str���͵İ汾��ת�������֡����ڴ���ͶԱ�
	VersionStr2Uint32(szProgramVersion, m_ClientProgramInfo.dwOldVersion);

	// ÿ�����¶�ȡ�������ݣ�����Ҫ��Щ��־λ�óɳ�ʼ״̬��Ȼ�����������µ��ļ�
	m_ClientProgramInfo.bDownLoadFinish = false;
	m_ClientProgramInfo.bSocketError = false;
	m_ClientProgramInfo.bStopDownLoad = false;

	return true;
}

void SocketClient::UpdateProgramInfo(const char*  pAppName)
{
	// ����汾��Ϊ0�������û���°汾�ţ����ڴ�����á�
	if (m_ClientProgramInfo.dwNewVersion == 0)
		return;

	const uint32_t nLen = 128;
	char szBuf[nLen] = { 0 };
	char szConfigPath[nLen] = { 0 };
	int _res = SearchPath(".\\config", "UpdateConfigClient.ini", NULL, nLen, szConfigPath, NULL);
	if (_res == 0)
	{
		Loger(LEVEL_ERROR, perro("û���ҵ������ļ���UpdateConfigClient.ini", WSAGetLastError()).c_str());
		return;
	}

	// ���°汾��֮ǰ���µİ汾�Ÿ�ֵ����Ա���󣬱������سɹ����ȡ��ǰ�ļ��İ汾��
	m_ClientProgramInfo.dwOldVersion = m_ClientProgramInfo.dwNewVersion;
	// ���³����ļ��� �汾��
	char szProgramVer[nLen];
	VersionUint32toStr(szProgramVer, m_ClientProgramInfo.dwNewVersion);
	if (0 == WritePrivateProfileString(pAppName, "VERSION", szProgramVer, szConfigPath))
	{
		// û�и�����
		if (0x02 == GetLastError())
			return;
	}
	// �ļ���
	if (0 == WritePrivateProfileString(pAppName, "FILE_NAME", m_ClientProgramInfo.szFileName, szConfigPath))
	{
		// û�и�����
		if (0x02 == GetLastError())
			return;
	}
}
