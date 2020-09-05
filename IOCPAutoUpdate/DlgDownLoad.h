#pragma once
#include "SocketClient.h"
#include "afxcmn.h"

// CDlgDownLoad 对话框

class CDlgDownLoad : public CDialog
{
	DECLARE_DYNAMIC(CDlgDownLoad)

public:
	CDlgDownLoad(SocketClient* pSocketClient, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgDownLoad();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_DOWNLOAD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	SocketClient* m_pSockClient;
	CProgressCtrl m_progressDownLoad;
	CString m_strFileSize;
	CString m_strDownLoadPercent;
	CString m_strDownLoadSpeed;
	CString m_strErrorInfo;

	LARGE_INTEGER m_nfreq;					// 性能计数器的频率
	LARGE_INTEGER m_nBeforeCount;			// 间隔时间开始的性能计数器
	LARGE_INTEGER m_nLaterCount;			// 最终／间隔时间结束时的性能计数器
	LONGLONG	m_nQuadPart;				// 间隔性能计数值
	DWORD		m_nLastDownLoadLeftSize;	// 上一次剩余的下载数据量
	CBrush		m_brushControl;					// 控件刷子

	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
