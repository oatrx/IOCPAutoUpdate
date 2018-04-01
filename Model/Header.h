#pragma once
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� IOCPAUTOUPDATE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// IOCPAUTOUPDATE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#if	defined IOCPAUTOUPDATE_EXPORTS || defined IOCPUPDATE_EXPORTS
#define IOCPAUTOUPDATE_API __declspec(dllexport)
#else
#define IOCPAUTOUPDATE_API __declspec(dllimport)
#endif

#include <cstdint>
typedef struct _tagUpdateHeader {
	uint32_t dwVer;
	uint32_t dwCmd;
	uint32_t dwLen;
}sHEADER, *LPsHEADER;// ����������Ϣ�Ľṹ��


#define UPDATE_NET_CMD_LOGIN 0x00000001
typedef struct _tagUpdateLogin {
	CHAR szUserName[128];
	CHAR szPassword[128];
}sLOGIN, *LPsLOGIN;


#define UPDATE_NET_CMD_LOGIN_RET 0x80000001
typedef struct _tagUpdateLoginRet {
	uint32_t dwSuccess;
}sLOGINRET, *LPsLOGINRET;



#define  UPDATE_NET_CMD_PROGRAM_VER 0x00000002
typedef struct _tagUpdateProgramVer{
	CHAR szProgramName[128];	//ԭʼ��������
	CHAR szVer[128];		//ԭʼ����汾��Ϣ
}sPROGRAMVER, *LPsPROGRAMVER;


#define  UPDATE_NET_CMD_PROGRAM_VER_RET 0x80000002
typedef struct _tag_tagUpdateProgramVerRet {
	CHAR szFileName[128];	//�����ļ�����
	uint32_t dwFileSize;	//�ļ�����
	uint32_t dwNewVer; // �Ƿ����°汾
}sPROGRAMVERRET, *LPsPROGRAMVERRET;


#define  UPDATE_NET_CMD_DOWNLOAD 0x00000003
typedef struct _tag_tagUpdateDownLoad {
	uint32_t dwDownLoad; // ����
}sDOWNLOAD, *LPsDOWNLOAD;


#define  UPDATE_NET_CMD_DOWNLOAD_RET 0x80000003
typedef struct _tag_tagUpdateDownLoadRet {
	uint32_t dwDownLoadFinish;
}sDOWNLOADRET, *LPsDOWNLOADRET;
