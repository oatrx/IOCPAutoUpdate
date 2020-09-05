// UpdateClientManage.h

#pragma once
#include <windows.h>
#include "Client.h"
#include <vcclr.h>
using namespace System;

namespace UpdateClientManage {

	public ref class UpdateClient
	{
		// TODO:  在此处添加此类的方法。
	public:
		UpdateClient();
		~UpdateClient();
		/**
		* 入参配置的 appName
		* 返回值：0 有新版本可以下载, 1 有新版文件，但是本地有与新版本文件名相同的本地文件存在。2 没有新版本
		*		  3 socket库启动失败, 4 连接服务器失败， 5 登录验证失败, 6 网络异常
		*/
		int Authenticate(String^  pAppName);
		// 下载,成功返回true，失败返回false
		bool DownLoad(String^  pAppName, const bool bDownLoad, const bool bShowDlg);

		// 获取文件大小，以M为单位，需要在Authenticate调用成功后调用
		double GetFileSize();

		// 需要另起线程调用，如果完全还没下载数据返回0，下载完成返回100;
		int32_t GetDownLoadPercent();

		// 下载过程中，如果网络出错，返回true
		bool IsSocketError();

		// 如果阻塞在DownLoad下载过程中，想取消下载，则可以调用此函数。
		void StopDownLoad();

		// 下载成功，获取到的是新下载下来的文件路径
		void GetFullFilePath(array<wchar_t>^  pszFilePath, int nbufFilePathLen);
		// 下载成功项目名称
		void GetProgramName(array<wchar_t>^ pszProgramName, int nbufProgramNameLen);
		// 获取文件的版本号。下载成功后则是新下载文件的版本号，否则是之前读取配置时候的文件版本号
		uint32_t GetFileVersion();
	private:
		Client* m_pClient;
	};
}
