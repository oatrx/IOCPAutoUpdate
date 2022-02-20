#ifndef PROTOCOLHEADER_H
#define PROTOCOLHEADER_H
#include <stdint.h>

typedef struct _tagUpdateHeader {
    uint32_t dwVer;
    uint32_t dwCmd;
    uint32_t dwLen;
}sHEADER, *LPsHEADER;// 用来接收消息的结构体


#define UPDATE_NET_CMD_LOGIN 0x00000001
typedef struct _tagUpdateLogin {
    char szUserName[128];
    char szPassword[128];
}sLOGIN, *LPsLOGIN;


#define UPDATE_NET_CMD_LOGIN_RET 0x80000001
typedef struct _tagUpdateLoginRet {
    uint32_t dwSuccess;
}sLOGINRET, *LPsLOGINRET;



#define  UPDATE_NET_CMD_PROGRAM_VER 0x00000002
typedef struct _tagUpdateProgramVer{
    char szProgramName[128];	//原始程序名称
    uint32_t dwVer;				//原始程序版本信息
}sPROGRAMVER, *LPsPROGRAMVER;


#define  UPDATE_NET_CMD_PROGRAM_VER_RET 0x80000002
typedef struct _tag_tagUpdateProgramVerRet {
    char szFileName[128];	//程序文件名称
    uint32_t dwFileSize;	//文件长度
    uint32_t dwNewVer;		//服务器上文件的版本号。如果比客户端的更高，则是服务器上的版本号。如果比客户端的版本低（或者打开文件失败）则为0.
}sPROGRAMVERRET, *LPsPROGRAMVERRET;


#define  UPDATE_NET_CMD_DOWNLOAD 0x00000003
typedef struct _tag_tagUpdateDownLoad {
    uint32_t dwDownLoad; // 下载
}sDOWNLOAD, *LPsDOWNLOAD;


#define  UPDATE_NET_CMD_DOWNLOAD_RET 0x80000003
typedef struct _tag_tagUpdateDownLoadRet {
    uint32_t dwDownLoadFinish;
}sDOWNLOADRET, *LPsDOWNLOADRET;

#endif // PROTOCOLHEADER_H
