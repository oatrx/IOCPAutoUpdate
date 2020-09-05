// DlgTips.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "DlgTips.h"
#include "afxdialogex.h"
#include "resource.h"

// DlgTips �Ի���

IMPLEMENT_DYNAMIC(DlgTips, CDialog)

DlgTips::DlgTips(LPCSTR lpTips, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DLG_TIPS, pParent)
	, m_nYesOrNo(IDNO)
{
	m_textTips.SetString(lpTips);
}

DlgTips::~DlgTips()
{
}

void DlgTips::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BT_TIPS_YES, m_btYes);
	DDX_Control(pDX, IDC_BT_TIPS_NO, m_btNo);
	DDX_Text(pDX, IDC_STATIC_TIPS, m_textTips);
}


BEGIN_MESSAGE_MAP(DlgTips, CDialog)
	ON_BN_CLICKED(IDC_BT_TIPS_YES, &DlgTips::OnClickedBtTipsYes)
	ON_BN_CLICKED(IDC_BT_TIPS_NO, &DlgTips::OnClickedBtTipsNo)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// DlgTips ��Ϣ�������


void DlgTips::OnClickedBtTipsYes()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_nYesOrNo = IDYES;
	PostMessage(WM_CLOSE);
}


void DlgTips::OnClickedBtTipsNo()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_nYesOrNo = IDNO;
	PostMessage(WM_CLOSE);
}


BOOL DlgTips::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	// ��ɫ��ˢ��
	m_brushControl.CreateSolidBrush(RGB(0, 0, 0));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


int DlgTips::GetYesOrNo()
{
	return m_nYesOrNo;
}

void DlgTips::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: �ڴ˴������Ϣ����������
					   // ��Ϊ��ͼ��Ϣ���� CDialog::OnPaint()
					   // ����ˢ�ɺ�ɫ
	CRect rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect, RGB(0, 0, 0));// ��ɫ
}


HBRUSH DlgTips::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	// ���пؼ����óɰ��� ͸����
	if ((nCtlColor == CTLCOLOR_STATIC) &&
		(pWnd->GetDlgCtrlID() == IDC_STATIC_TIPS))
	{
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)m_brushControl;
	}
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

int MessageBoxUpdate(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText)
{
	DlgTips dlg(lpText,nullptr);
	dlg.DoModal();
	return dlg.GetYesOrNo();
}
