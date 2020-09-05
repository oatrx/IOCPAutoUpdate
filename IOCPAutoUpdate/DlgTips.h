#pragma once
#include "afxwin.h"


// DlgTips �Ի���

class DlgTips : public CDialog
{
	DECLARE_DYNAMIC(DlgTips)

public:
	DlgTips(LPCSTR lpTips, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~DlgTips();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_TIPS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	int GetYesOrNo();
	afx_msg void OnClickedBtTipsYes();
	afx_msg void OnClickedBtTipsNo();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
	int			m_nYesOrNo;
	CBrush		m_brushControl;					// �ؼ�ˢ��
	CButton m_btYes;
	CButton m_btNo;
	CString m_textTips;
};

int MessageBoxUpdate(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText);
