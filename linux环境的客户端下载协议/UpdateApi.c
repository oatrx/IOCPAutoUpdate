#include "UpdateApi.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

static int SendData(SOCKET sockfd, uint32_t dwCmdID, void* pData, uint32_t nLen);
static int SendRaw(SOCKET sockfd, void*  pData, uint32_t nLen);
#define SEND_BUFF_LEN	1400
SOCKET CreateTcpConnect(const char* ip, const unsigned short port)
{
    int sockfd= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        return -1;
    }

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    //已注册端口号：1024~49151
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    // 设置默认操时3s
    if(SetTimeOut(sockfd, 5, 5) < 0)
    {
        CloseSocket(sockfd);
        return -1;
    }

    if (0 != connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        CloseSocket(sockfd);
        return -1;
    }
    return sockfd;
}

int SetTimeOut(SOCKET sockfd, const int sendTimeOut_s, const int recvTimeOut_s)
{
    // 发送数据或者接收数据失败。网络可能就出现错误了。
    struct timeval sendTimeOut = {sendTimeOut_s,0};
    struct timeval recvTimeOut = {recvTimeOut_s,0};
    int nRet = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&sendTimeOut, sizeof(sendTimeOut));
    if (nRet < 0)
    {
        return -1;
    }

    nRet = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeOut, sizeof(recvTimeOut));
    if (nRet < 0)
    {
        return -1;
    }
    return 0;
}

//登录
int Login(SOCKET sockfd, const char* userName, const char* passwd)
{
    sLOGIN login;
    memset(&login, 0, sizeof(sLOGIN));
    strcpy(login.szUserName, userName);
    strcpy(login.szPassword, passwd);
    if (SendData(sockfd, UPDATE_NET_CMD_LOGIN, &login, sizeof(login)) == 0)
    {
        sHEADER header = {0, 0, 0};
        int resHeader = RecvData(sockfd, &header, sizeof(header));
        sLOGINRET loginret = { 0 };
        int resLogin = RecvData(sockfd, &loginret, sizeof(loginret));
        if((0 == resHeader)
                && (UPDATE_NET_CMD_LOGIN_RET == header.dwCmd)
                &&(0 == resLogin))
        {
            return loginret.dwSuccess == 1 ? 0 : -1;
        }
    }
    return -1;
}

int ProgramVerify(SOCKET sockfd, const char* programName, const uint32_t programVer,sPROGRAMVERRET* programInfo)
{
    sPROGRAMVER programVerision;
    memset(&programVerision, 0, sizeof(sPROGRAMVER));
    strcpy(programVerision.szProgramName, programName);
    programVerision.dwVer = programVer;
    if (SendData(sockfd, UPDATE_NET_CMD_PROGRAM_VER, &programVerision, sizeof(programVerision)) == 0)
    {
        sHEADER header = {0, 0, 0};
        int resHeader = RecvData(sockfd, &header, sizeof(header));
        sPROGRAMVERRET programVerRet;
        memset(&programVerRet, 0, sizeof(sPROGRAMVERRET));
        int resVer = RecvData(sockfd, &programVerRet, sizeof(programVerRet));
        if ((0 == resHeader)
                && (UPDATE_NET_CMD_PROGRAM_VER_RET == header.dwCmd)
                && (0 == resVer)
                && (programVerRet.dwNewVer > programVer))
        {
            // 新版号随参数输出
            memcpy(programInfo, &programVerRet, sizeof(sPROGRAMVERRET));
            return 0;
        }
    }
    return -1;
}

int DownLoad(SOCKET sockfd, const int bDownLoad)
{
    sDOWNLOAD downLoad = { 0 };
    downLoad.dwDownLoad = bDownLoad ? 1 : 0;
    if (SendData(sockfd, UPDATE_NET_CMD_DOWNLOAD, &downLoad, sizeof(downLoad)) == 0)
    {
        return 0;
    }
    return -1;
}

int RecvData(SOCKET sockfd, void* pData, const uint32_t nLen)
{
    int32_t nLeft = nLen;
    char* pTemp = (char*)pData;
    while (nLeft > 0) {
        int nRev = recv(sockfd, pTemp, nLeft, 0);
        if ((nRev <= 0))
        {
            // 出错
            return -1;
        }
        pTemp = pTemp + nRev;
        nLeft = nLeft - nRev;
    }
    return 0;
}

int DownLoadFinish(SOCKET sockfd)
{
    sHEADER header = { 0,0,0 };
    int resHeader = RecvData(sockfd, &header, sizeof(header));
    if ((0 == resHeader) && UPDATE_NET_CMD_DOWNLOAD_RET == header.dwCmd)
    {
        sDOWNLOADRET downLoadRet = { 0 };
        int resFinish = RecvData(sockfd, &downLoadRet, sizeof(downLoadRet));
        if(0 == resFinish)
        {
            return downLoadRet.dwDownLoadFinish == 1 ? 0 : -1;
        }
    }
    return -1;
}

int SendData(SOCKET sockfd, uint32_t dwCmdID, void* pData, uint32_t nLen)
{
    sHEADER header = { 0 };
    header.dwCmd = dwCmdID;
    header.dwLen = nLen;
    uint32_t nSendLen = sizeof(header);
    char sendBuf[SEND_BUFF_LEN];
    // 数据头
    memmove(sendBuf, &header, nSendLen);
    // 数据
    memmove(sendBuf + nSendLen, pData, nLen);
    nSendLen += nLen;
    //发送数据包的包头 和 数据，服务器为完成端口，需要一次性发送数据
    if (SendRaw(sockfd, sendBuf, nSendLen) < 0)
    {
        return -1;
    }
    return 0;
}

int SendRaw(SOCKET sockfd, void*  pData, uint32_t nLen)
{
    int32_t nLeft = nLen;
    char* pTemp = (char*)pData;
    while (nLeft > 0){
        int nSend = send(sockfd, pTemp, nLeft, 0);
        if (nSend < 0)
        {
            // 出错
            // 网络出错标志
            return -1;
        }
        pTemp = pTemp + nSend;
        nLeft = nLeft - nSend;
    }
    return 0;
}

void CloseSocket(SOCKET sockfd)
{
    close(sockfd);
}
