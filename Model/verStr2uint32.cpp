#include "stdafx.h"
#include "verStr2uint32.h"
#include <cstdint>
#include <cstring>
#define INFO_LEN 128

#define BYTE0(dwTemp)       (*(char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
#define BYTE4(dwTemp)       (*((char *)(&dwTemp) + 4))
#define BYTE5(dwTemp)       (*((char *)(&dwTemp) + 5))
#define BYTE6(dwTemp)       (*((char *)(&dwTemp) + 6))
#define BYTE7(dwTemp)       (*((char *)(&dwTemp) + 7))

void VersionStr2Uint32(const char* szVersion, uint32_t& nVersion)
{
	int arrDot[3] = { 0 };
	char szVer[INFO_LEN];
	memset(szVer, 0, INFO_LEN);
	memcpy(szVer, szVersion, strlen(szVersion));
	int nu = strlen(szVersion);
	for (int i = 0, j = 0; i < nu; i++)
	{
		if (szVer[i] == '.')
		{
			szVer[i] = '\0';
			arrDot[++j] = i + 1;
		}
	}
	uint8_t nBig = atoi(szVer + arrDot[0]);
	uint8_t nMid= atoi(szVer + arrDot[1]);
	uint8_t nLit = atoi(szVer + arrDot[2]);
	nVersion = (nBig << 16) | (nMid << 8) | nLit;
}

void VersionUint32toStr(char* szVersion, uint32_t nVersion)
{
	char szVer[INFO_LEN];
	memset(szVer, 0, INFO_LEN);
	uint8_t nBig = BYTE2(nVersion);
	uint8_t nMid = BYTE1(nVersion);
	uint8_t nLit = BYTE0(nVersion);
	sprintf(szVer, "%d.%d.%d", nBig, nMid, nLit);
	memcpy(szVersion, szVer, strlen(szVer) + 1);
}