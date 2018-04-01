#include "stdafx.h"
#include "SocketClient.h"
#include "Header.h"
#include <iostream>
#include "fstream"
#include "Log.h"
#include <cassert>
#include <cstring>
#include "shlwapi.h"
using namespace std;

// 默认端口
#define DEFAULT_PORT          "12345"
// 默认IP地址
#define DEFAULT_IP            _T("127.0.0.1")
#pragma comment(lib, "Shlwapi.lib")
SocketClient::SocketClient()
	:m_bLibLoaded(false),
	m_hClientSock(INVALID_SOCKET)
{
	Log::getInstance().setLogLevel(LEVEL_INFOR);
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
	Log::getInstance().removeInstance();
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
//创建socket，连接服务器
bool SocketClient::start()
{
	// 配置文件不存在
	if (!ReadConfig())
		return false;

	m_hClientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_hClientSock == INVALID_SOCKET)
	{
		Loger(LEVEL_ERROR, perro("创建socket失败.", WSAGetLastError()).c_str());
		return false;
	}

	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	//已注册端口号：1024~49151
	addr.sin_port = htons(m_nPort);
	addr.sin_addr.S_un.S_addr = inet_addr(m_strIP.c_str());
	if (0 != connect(m_hClientSock, (sockaddr *)&addr, sizeof(addr)))
	{
		Loger(LEVEL_ERROR, perro("连接服务器失败.", WSAGetLastError()).c_str());
		return false;
	}

	int nNetTimeout = 8000; // 8s 发送数据或者接收数据失败。网络可能就出现错误了。
	//bool bKeepAlive = true;
	int nRet = setsockopt(m_hClientSock, SOL_SOCKET, SO_SNDTIMEO, (char*)&nNetTimeout, sizeof(nNetTimeout));
	if (SOCKET_ERROR == nRet)
	{
		Loger(LEVEL_ERROR, perro("设置发送超时时间失败", WSAGetLastError()).c_str());
		return false;
	}

	//bool bKeepAlive = true;
	nRet = setsockopt(m_hClientSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nNetTimeout, sizeof(nNetTimeout));
	if (SOCKET_ERROR == nRet)
	{
		Loger(LEVEL_ERROR, perro("设置接收超时时间失败", WSAGetLastError()).c_str());
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

//登录
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

//获取所有类型
int32_t SocketClient::ProgramVer()
{
	sPROGRAMVER programVer = { 0 };
	strcpy(programVer.szProgramName, m_ClientProgramInfo.szProgramName);
	strcpy(programVer.szVer, m_ClientProgramInfo.szProgramVersion);
	if (SendData(UPDATE_NET_CMD_PROGRAM_VER, &programVer, sizeof(programVer)))
	{
		sHEADER header = { 0 };
		RecvData(&header, sizeof(header));
		sPROGRAMVERRET programVerRet = { 0 };
		RecvData(&programVerRet, sizeof(programVerRet));
		if (1 == programVerRet.dwNewVer)
		{
			strcpy(m_ClientProgramInfo.szFileName, programVerRet.szFileName);
			m_ClientProgramInfo.dwFileSize = programVerRet.dwFileSize;
			m_ClientProgramInfo.dwLeftLen = programVerRet.dwFileSize;
			// 下载文件是否存在
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
		// 不下载，就不去接收数据
		if (!bDownLoad)
			return false;
		// 成功下载
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

// 获取下载剩余的百分比
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

// 停止下载
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

//下载文件
bool SocketClient::GetFile()
{
	string strFullPath(m_ClientProgramInfo.szFilePath);
	strFullPath += m_ClientProgramInfo.szFileName;
	HANDLE m_hProFile = CreateFile(strFullPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if (INVALID_HANDLE_VALUE == m_hProFile)
	{
		Loger(LEVEL_ERROR, perro("创建下载文件失败.", GetLastError()).c_str());
		return false;
	}
	CHAR szBuff[MAX_BUFFER_LEN] = { 0 };
	DWORD dwBytesToWrite = 0;
	DWORD dwBytesWrited = 0;
	CHAR* pszBuf;
	m_ClientProgramInfo.dwLeftLen = m_ClientProgramInfo.dwFileSize;
	// 还有数据 和没有停止下载，就继续下载。
	while (m_ClientProgramInfo.dwLeftLen > 0 && !m_ClientProgramInfo.bStopDownLoad){
		int nRecv = MAX_BUFFER_LEN;
		if (m_ClientProgramInfo.dwLeftLen < nRecv)
		{
			nRecv = m_ClientProgramInfo.dwLeftLen;
		}
		if (!RecvData(szBuff, nRecv))
		{
			Loger(LEVEL_ERROR, perro("文件下载过程中失败.", GetLastError()).c_str());
			CloseHandle(m_hProFile);
			return false;
		}
		m_ClientProgramInfo.dwLeftLen = m_ClientProgramInfo.dwLeftLen - nRecv;
		// 写文件
		dwBytesToWrite = nRecv;
		dwBytesWrited = 0;
		pszBuf = szBuff;
		do { //循环写文件，确保完整的文件被写入
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
			m_ClientProgramInfo.bSocketError = true;// 网络出错标志
			Loger(LEVEL_ERROR, perro("数据接收失败.", WSAGetLastError()).c_str());
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
	// 数据头
	memmove(m_szSendBuf, &header, nSendLen);
	// 数据
	memmove(m_szSendBuf + nSendLen, pData, nLen);
	nSendLen += nLen;
	//发送数据包的包头 和 数据，服务器为完成端口，需要一次性发送数据
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
			m_ClientProgramInfo.bSocketError = true;// 网络出错标志
			Loger(LEVEL_ERROR, perro("数据发送失败.", WSAGetLastError()).c_str());
			return false;
		}
		pTemp = pTemp + nSend;
		nLeft = nLeft - nSend;
	}
	return true;
}

// 读取配置信息
bool SocketClient::ReadConfig()
{
	const uint32_t nLen = 128;
	char szBuf[nLen] = { 0 };
	char szConfigPath[nLen] = { 0 };
	// 检查配置文件是否存在
	int _res = SearchPath(".\\", "UpdateConfigClient.ini", NULL, nLen, szConfigPath, NULL);
	if (_res == 0)
	{
		Loger(LEVEL_ERROR, perro("没有找到配置文件：UpdateConfigClient.ini", WSAGetLastError()).c_str());
		return false;
	}
	// 读取ip地址和端口号
	GetPrivateProfileString("TCP\\IP", "SERVER_IP", DEFAULT_IP, szBuf, nLen, szConfigPath);
	m_strIP = szBuf;
	GetPrivateProfileString("TCP\\IP", "SERVER_PORT", DEFAULT_PORT, szBuf, nLen, szConfigPath);
	m_nPort = atoi(szBuf);

	// 读取用户配置
	if (0 == GetPrivateProfileString("USER", "NAME", "", m_User.szUserName, INFO_LEN, szConfigPath))
	{
		// 没有该配置
		if (0x02 == GetLastError())
			return false;
	}
	// 没有配置密码，就为空
	GetPrivateProfileString("USER", "PWD", "", m_User.szPassword, INFO_LEN, szConfigPath);

	// 客户端程序配置
	// 程序名称
	if (0 == GetPrivateProfileString("PROGRAM", "NAME", "", m_ClientProgramInfo.szProgramName, INFO_LEN, szConfigPath))
	{
		// 没有该配置
		if (0x02 == GetLastError())
			return false;
	}
	// 文件路径
	GetPrivateProfileString("PROGRAM", "PATH", ".\\", m_ClientProgramInfo.szFilePath, INFO_LEN, szConfigPath);

	// 版本号
	GetPrivateProfileString("PROGRAM", "VERSION", "", m_ClientProgramInfo.szProgramVersion, INFO_LEN, szConfigPath);

	return true;
}