// IOCPAutoUpdate.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "Header.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ψһ��Ӧ�ó������

CWinApp theApp;

using namespace std;

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(NULL);

    if (hModule != NULL)
    {
        // ��ʼ�� MFC ����ʧ��ʱ��ʾ����
        if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
        {
            // TODO: ���Ĵ�������Է���������Ҫ
            wprintf(L"����: MFC ��ʼ��ʧ��\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
        }
    }
    else
    {
        // TODO: ���Ĵ�������Է���������Ҫ
        wprintf(L"����: GetModuleHandle ʧ��\n");
        nRetCode = 1;
    }

    return nRetCode;
}
