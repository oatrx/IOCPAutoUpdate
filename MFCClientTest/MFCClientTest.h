
// MFCClientTest.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMFCClientTestApp: 
// �йش����ʵ�֣������ MFCClientTest.cpp
//

class CMFCClientTestApp : public CWinApp
{
public:
	CMFCClientTestApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMFCClientTestApp theApp;