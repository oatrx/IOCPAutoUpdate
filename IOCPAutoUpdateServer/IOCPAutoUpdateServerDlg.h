
// IOCPAutoUpdateServerDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "MessageDisplay.h"
#include "Server.h"
// CIOCPAutoUpdateServerDlg �Ի���
class CIOCPAutoUpdateServerDlg : public CDialogEx,public MessageDisplay
{
// ����
public:
	CIOCPAutoUpdateServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IOCPAUTOUPDATESERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	virtual void AddMessage(const char* message);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	Server	m_Server;
	CListCtrl m_listMessage;
	afx_msg void OnClose();
};
