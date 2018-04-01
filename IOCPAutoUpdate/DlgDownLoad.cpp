// DlgDownLoad.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgDownLoad.h"
#include "afxdialogex.h"
#include "resource.h"

// CDlgDownLoad 对话框

IMPLEMENT_DYNAMIC(CDlgDownLoad, CDialog)

CDlgDownLoad::CDlgDownLoad(SocketClient* pSocketClient, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DLG_DOWNLOAD, pParent),
	m_pSockClient(pSocketClient)
	, m_strFileSize(_T(""))
	, m_strDownLoadPercent(_T(""))
	, m_strErrorInfo(_T(""))
	, m_strDownLoadSpeed(_T(""))
{

}

CDlgDownLoad::~CDlgDownLoad()
{
}

void CDlgDownLoad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_DOWNLOADED, m_progressDownLoad);
	DDX_Text(pDX, IDC_FILE_SIZE, m_strFileSize);
	DDX_Text(pDX, IDC_DOWDLOAD_PERCENT, m_strDownLoadPercent);
	DDX_Text(pDX, IDC_ERROR_INFO, m_strErrorInfo);
	DDX_Text(pDX, IDC_DOWNLOAD_SPEED, m_strDownLoadSpeed);
}


BEGIN_MESSAGE_MAP(CDlgDownLoad, CDialog)
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDlgDownLoad 消息处理程序


void CDlgDownLoad::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pSockClient->StopDownLoad();
	KillTimer(1);
	KillTimer(2);
	CDialog::OnClose();
}


void CDlgDownLoad::OnTimer(UINT_PTR nIDEvent)
{
	if (1 == nIDEvent)
	{
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		int nDownLoadPercent = 100 - m_pSockClient->GetDownLoadLeftPercent();
		m_progressDownLoad.SetPos(nDownLoadPercent);
		m_strDownLoadPercent.Format("%d", nDownLoadPercent);
		if (m_pSockClient->IsSocketError())
			m_strErrorInfo.Format("网络连接错误！");
		if (m_pSockClient->IsDownLoadFinish())
		{
			m_strErrorInfo.Format("下载完成！");
			PostMessage(WM_CLOSE, 0, 0);
		}
	}
	else if (2 == nIDEvent)
	{
		QueryPerformanceCounter(&m_nLaterCount);
		m_nQuadPart = m_nLaterCount.QuadPart - m_nBeforeCount.QuadPart;
		double time = (double)m_nQuadPart / m_nfreq.QuadPart;
		m_nBeforeCount = m_nLaterCount;
		DWORD _nCurrentLeftSize = m_pSockClient->GetDownLoadLeftSize();
		// 实时下载速度
		double v = (double)(m_nLastDownLoadLeftSize - _nCurrentLeftSize) / time / 1024; // 换算成kb/s
		m_nLastDownLoadLeftSize = _nCurrentLeftSize;
		if ((int)v > 1024)
		{
			v /= 1024; //  M/s
			m_strDownLoadSpeed.Format("%0.2lf M/s", v);
		}
		else
		{
			m_strDownLoadSpeed.Format("%0.2lf kb/s", v);
		}
	}
	UpdateData(FALSE);
	CDialog::OnTimer(nIDEvent);
}


BOOL CDlgDownLoad::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(1, 300, NULL);
	SetTimer(2, 1000, NULL);
	QueryPerformanceFrequency(&m_nfreq);
	QueryPerformanceCounter(&m_nBeforeCount);
	QueryPerformanceCounter(&m_nLaterCount);
	// 下载的文件大小
	m_strFileSize.Format("%0.2lf", m_pSockClient->GetFileSize());
	m_nLastDownLoadLeftSize = m_pSockClient->GetDownLoadLeftSize();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
