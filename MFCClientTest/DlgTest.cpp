// DlgTest.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MFCClientTest.h"
#include "DlgTest.h"
#include "afxdialogex.h"


// CDlgTest �Ի���

IMPLEMENT_DYNAMIC(CDlgTest, CDialog)

CDlgTest::CDlgTest(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG1, pParent)
{

}

CDlgTest::~CDlgTest()
{
}

void CDlgTest::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgTest, CDialog)
END_MESSAGE_MAP()


// CDlgTest ��Ϣ�������
