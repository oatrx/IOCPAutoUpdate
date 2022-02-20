#include <stdio.h>
#include "UpdateApi.h"
#define BUF_LEN 1400
int main()
{
    int res;
    // 1. 创建失败
    SOCKET sockfd = CreateTcpConnect("47.99.141.237",8888);
    if(sockfd < 0)
    {
        printf("create connect error!!\n");
        return -1;
    }
    // 2. 设置接收和发送操时（默认5s, 需要修改则调用）
    res = SetTimeOut(sockfd, 7, 7);
    if (res < 0)
    {
        CloseSocket(sockfd);
        return -1;
    }
    // 3. 登录（验证用户合法性）
    res = Login(sockfd, "dta", "123456");
    if (res < 0)
    {
        CloseSocket(sockfd);
        return -1;
    }

    // 4. 获取程序的最新版本信息
    uint32_t oldVersion=0;
    // 老版本号：1.2.14
    oldVersion = (1<<16) + (2 << 8) + (14 << 0);
    // 新的程序文件信息
    sPROGRAMVERRET newProgramInfo;
    res = ProgramVerify(sockfd, "longyouBoard", oldVersion, &newProgramInfo);
    if(res < 0)
    {
        CloseSocket(sockfd);
        return -1;
    }

    // 5. 下载（有新版本文件）
    res = DownLoad(sockfd, 1);
    if(res < 0)
    {
        CloseSocket(sockfd);
        return -1;
    }

    // 6. 下载程序文件的原始数据
    printf("program file name: %s\n", newProgramInfo.szFileName);
    int fileSize = newProgramInfo.dwFileSize;
    uint32_t newVersion = newProgramInfo.dwNewVer;
    char recvBuf[BUF_LEN];
    int remainingSize = fileSize;
    while(remainingSize>0)
    {
        int recvSize = 0;
        if (remainingSize/BUF_LEN > 0)
            recvSize = BUF_LEN;
        else
            recvSize = remainingSize;
        // 接收程序文件的原始数据
        res = RecvData(sockfd, recvBuf, recvSize);
        if(res < 0)
        {
            CloseSocket(sockfd);
            return -1;
        }
        // 还未接收的程序文件原始数据长度
        remainingSize -= recvSize;
    }

    // 7. 接收服务器发送的完成标志
    res = DownLoadFinish(sockfd);
    if(res < 0)
    {
        // 没有接收到完成标志
        CloseSocket(sockfd);
        return -1;
    }
    CloseSocket(sockfd);
    // 下载流程走完，数据成功接收
    // 处理程序下载的数据
    printf("download success!!!!!!!!!!!!!!!!!!!!!!!\n");
    return 0;
}
