// UpdateClientManage.h

#pragma once
#include <windows.h>
#include "Client.h"
#include <vcclr.h>
using namespace System;

namespace UpdateClientManage {

	public ref class UpdateClient
	{
		// TODO:  �ڴ˴���Ӵ���ķ�����
	public:
		UpdateClient();
		~UpdateClient();
		/**
		* ������õ� appName
		* ����ֵ��0 ���°汾��������, 1 ���°��ļ������Ǳ��������°汾�ļ�����ͬ�ı����ļ����ڡ�2 û���°汾
		*		  3 socket������ʧ��, 4 ���ӷ�����ʧ�ܣ� 5 ��¼��֤ʧ��, 6 �����쳣
		*/
		int Authenticate(String^  pAppName);
		// ����,�ɹ�����true��ʧ�ܷ���false
		bool DownLoad(String^  pAppName, const bool bDownLoad, const bool bShowDlg);

		// ��ȡ�ļ���С����MΪ��λ����Ҫ��Authenticate���óɹ������
		double GetFileSize();

		// ��Ҫ�����̵߳��ã������ȫ��û�������ݷ���0��������ɷ���100;
		int32_t GetDownLoadPercent();

		// ���ع����У���������������true
		bool IsSocketError();

		// ���������DownLoad���ع����У���ȡ�����أ�����Ե��ô˺�����
		void StopDownLoad();

		// ���سɹ�����ȡ�������������������ļ�·��
		void GetFullFilePath(array<wchar_t>^  pszFilePath, int nbufFilePathLen);
		// ���سɹ���Ŀ����
		void GetProgramName(array<wchar_t>^ pszProgramName, int nbufProgramNameLen);
		// ��ȡ�ļ��İ汾�š����سɹ��������������ļ��İ汾�ţ�������֮ǰ��ȡ����ʱ����ļ��汾��
		uint32_t GetFileVersion();
	private:
		Client* m_pClient;
	};
}
