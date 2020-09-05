#pragma once
#include "SocketClient.h"
#include "afxcmn.h"

// CDlgDownLoad �Ի���

class CDlgDownLoad : public CDialog
{
	DECLARE_DYNAMIC(CDlgDownLoad)

public:
	CDlgDownLoad(SocketClient* pSocketClient, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgDownLoad();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_DOWNLOAD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
	SocketClient* m_pSockClient;
	CProgressCtrl m_progressDownLoad;
	CString m_strFileSize;
	CString m_strDownLoadPercent;
	CString m_strDownLoadSpeed;
	CString m_strErrorInfo;

	LARGE_INTEGER m_nfreq;					// ���ܼ�������Ƶ��
	LARGE_INTEGER m_nBeforeCount;			// ���ʱ�俪ʼ�����ܼ�����
	LARGE_INTEGER m_nLaterCount;			// ���գ����ʱ�����ʱ�����ܼ�����
	LONGLONG	m_nQuadPart;				// ������ܼ���ֵ
	DWORD		m_nLastDownLoadLeftSize;	// ��һ��ʣ�������������
	CBrush		m_brushControl;					// �ؼ�ˢ��

	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
