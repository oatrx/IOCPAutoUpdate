#pragma once
// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 IOCPAUTOUPDATE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// IOCPAUTOUPDATE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
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
}sHEADER, *LPsHEADER;// 用来接收消息的结构体


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
	CHAR szProgramName[128];	//原始程序名称
	CHAR szVer[128];		//原始程序版本信息
}sPROGRAMVER, *LPsPROGRAMVER;


#define  UPDATE_NET_CMD_PROGRAM_VER_RET 0x80000002
typedef struct _tag_tagUpdateProgramVerRet {
	CHAR szFileName[128];	//程序文件名称
	uint32_t dwFileSize;	//文件长度
	uint32_t dwNewVer; // 是否有新版本
}sPROGRAMVERRET, *LPsPROGRAMVERRET;


#define  UPDATE_NET_CMD_DOWNLOAD 0x00000003
typedef struct _tag_tagUpdateDownLoad {
	uint32_t dwDownLoad; // 下载
}sDOWNLOAD, *LPsDOWNLOAD;


#define  UPDATE_NET_CMD_DOWNLOAD_RET 0x80000003
typedef struct _tag_tagUpdateDownLoadRet {
	uint32_t dwDownLoadFinish;
}sDOWNLOADRET, *LPsDOWNLOADRET;
