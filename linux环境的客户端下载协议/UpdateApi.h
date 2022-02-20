#ifndef UPDATEAPI_H
#define UPDATEAPI_H
#include <time.h>
#include "ProtocolHeader.h"
typedef int SOCKET;

/* 功能： 创建socket连接
 * 返回值： 0 成功， -1 失败。
 * */

SOCKET CreateTcpConnect(const char* ip, const unsigned short port);

/* 功能：设置socket发送和接收超时(默认是5)，需要修改就调用这个函数
 * 返回值： 0 成功， -1 失败。
 * */
int SetTimeOut(SOCKET sockfd, const int sendTimeOut, const int recvTimeOut);

/* 功能：登录
 * 返回值： 0 成功， -1 失败。
 * */
int Login(SOCKET sockfd,const char* userName, const char* passwd);

/*
 * 功能：获取是否有新版本，
 * 入参：
 *  socktfd:    网络套接字
 *  programName: 和服务器上约定的该程序的唯一标示
 *  programVer: 该程序的当前版本好。
 * 出参：
 *  programInfo: 包括程序的文件名， 程序文件的长度，程序文件的版本号
 * 返回值： 0 成功， -1 失败。
 * */
int ProgramVerify(SOCKET sockfd, const char* programName, const uint32_t programVer,sPROGRAMVERRET* programInfo);

/* 功能： 向服务器发送下载或在不下载指令
 * 入参：
 *  bDownLoad： 0 不下载， 1 下载
 * 返回值： 0 成功， -1 失败。
 * */
int DownLoad(SOCKET sockfd, const int bDownLoad);

/* 功能： 接收程序数据
 * 入参：
 *  bDownLoad： 0 不下载， 1 下载
 * 返回值： 0 成功， -1 出错。
 * */
int RecvData(SOCKET sockfd, void*  pData, const uint32_t nLen);

/* 功能： 接收文件下载完成的标志
 * 返回值： 0 成功， -1 出错。
 * */
int DownLoadFinish(SOCKET sockfd);

/* 功能： 关闭套接字
 * */
void CloseSocket(SOCKET sockfd);
#endif // UPDATEAPI_H
