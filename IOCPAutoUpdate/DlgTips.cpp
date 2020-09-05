// DlgTips.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgTips.h"
#include "afxdialogex.h"
#include "resource.h"

// DlgTips 对话框

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


// DlgTips 消息处理程序


void DlgTips::OnClickedBtTipsYes()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nYesOrNo = IDYES;
	PostMessage(WM_CLOSE);
}


void DlgTips::OnClickedBtTipsNo()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nYesOrNo = IDNO;
	PostMessage(WM_CLOSE);
}


BOOL DlgTips::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// 黑色的刷子
	m_brushControl.CreateSolidBrush(RGB(0, 0, 0));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


int DlgTips::GetYesOrNo()
{
	return m_nYesOrNo;
}

void DlgTips::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialog::OnPaint()
					   // 背景刷成黑色
	CRect rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect, RGB(0, 0, 0));// 黑色
}


HBRUSH DlgTips::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	// 所有控件设置成白字 透明底
	if ((nCtlColor == CTLCOLOR_STATIC) &&
		(pWnd->GetDlgCtrlID() == IDC_STATIC_TIPS))
	{
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)m_brushControl;
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

int MessageBoxUpdate(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText)
{
	DlgTips dlg(lpText,nullptr);
	dlg.DoModal();
	return dlg.GetYesOrNo();
}
