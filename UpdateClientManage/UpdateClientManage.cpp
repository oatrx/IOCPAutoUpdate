// 这是主 DLL 文件。

#include "stdafx.h"

#include "UpdateClientManage.h"

namespace UpdateClientManage
{
	UpdateClient::UpdateClient()
	{
		m_pClient = new Client;
	}

	UpdateClient::~UpdateClient()
	{
		if (m_pClient != nullptr)
			delete m_pClient;
		m_pClient = nullptr;
	}

	int UpdateClient::Authenticate(String^ pAppName)
	{
		if (nullptr != m_pClient)
		{
			pin_ptr<const wchar_t> pwAppName = PtrToStringChars(pAppName);
			int len = WideCharToMultiByte(CP_ACP, 0, pwAppName, -1, nullptr, 0, nullptr, nullptr);
			if (len > 0)
			{
				char* pcAppName = new char[len + 1];
				memset(pcAppName, 0, len + 1);
				WideCharToMultiByte(CP_ACP, 0, pwAppName, -1, pcAppName, len, nullptr, nullptr);
				int32_t res = m_pClient->Authenticate(pcAppName);
				delete pcAppName;
				return res;
			}
		}
		return -1;
	}

	bool UpdateClient::DownLoad(String^ pAppName, const bool bDownLoad, const bool bShowDlg)
	{
		if (nullptr != m_pClient)
		{
			pin_ptr<const wchar_t> pwAppName = PtrToStringChars(pAppName);
			int len = WideCharToMultiByte(CP_ACP, 0, pwAppName, -1, nullptr, 0, nullptr, nullptr);
			if (len > 0)
			{
				char* pcAppName = new char[len + 1];
				memset(pcAppName, 0, len + 1);
				WideCharToMultiByte(CP_ACP, 0, pwAppName, -1, pcAppName, len, nullptr, nullptr);
				bool res = m_pClient->DownLoad(pcAppName, bDownLoad, bShowDlg);
				delete pcAppName;
				return res;
			}
		}
		return false;
	}

	double UpdateClient::GetFileSize()
	{
		if (nullptr != m_pClient)
		{
			return m_pClient->GetFileSize();
		}
		return 0;
	}

	int32_t UpdateClient::GetDownLoadPercent()
	{
		if (nullptr != m_pClient)
		{
			return 100 - m_pClient->GetDownLoadLeftPercent();
		}
		return 100;
	}

	bool UpdateClient::IsSocketError()
	{
		if (nullptr != m_pClient)
		{
			return m_pClient->IsSocketError();
		}
		return true;
	}

	void UpdateClient::StopDownLoad()
	{
		if (nullptr != m_pClient)
		{
			m_pClient->StopDownLoad();
		}
	}

	void UpdateClient::GetFullFilePath(array<wchar_t>^ pszFilePath, int nbufFilePathLen)
	{
		if (nullptr != m_pClient)
		{
			char* pcFilePath = new char[nbufFilePathLen];
			memset(pcFilePath, 0, nbufFilePathLen);
			m_pClient->GetFullFilePath(pcFilePath, nbufFilePathLen);
			int len = MultiByteToWideChar(CP_ACP, 0, pcFilePath, -1, nullptr, 0);
			if (len > 0)
			{
				wchar_t* pwFilePath = new wchar_t[len + 1];
				memset(pwFilePath, 0, 2 * (len + 1));
				MultiByteToWideChar(CP_ACP, 0, pcFilePath, -1, pwFilePath, len);
				int i = 0;
				for (; i < len; i++)
				{
					pszFilePath[i] = pwFilePath[i];
				}
				delete pwFilePath;
			}

			delete pcFilePath;
		}
			
	}

	void UpdateClient::GetProgramName(array<wchar_t>^ pszProgramName, int nbufProgramNameLen)
	{
		if (nullptr != m_pClient)
		{
			char* pcProgramName = new char[nbufProgramNameLen];
			memset(pcProgramName, 0, nbufProgramNameLen);
			m_pClient->GetProgramName(pcProgramName, nbufProgramNameLen);
			int len = MultiByteToWideChar(CP_ACP, 0, pcProgramName, -1, nullptr, 0);
			if (len > 0)
			{
				wchar_t* pwProgramName = new wchar_t[len + 1];
				memset(pwProgramName, 0, 2 * (len + 1));
				MultiByteToWideChar(CP_ACP, 0, pcProgramName, -1, pwProgramName, len);
				int i = 0;
				for (; i < len; i++)
				{
					pszProgramName[i] = pwProgramName[i];
				}
				delete pwProgramName;
			}
			delete pcProgramName;
		}

	}

	uint32_t UpdateClient::GetFileVersion()
	{
		if (nullptr != m_pClient)
			return m_pClient->GetFileVersion();
		return 0;
	}

}
