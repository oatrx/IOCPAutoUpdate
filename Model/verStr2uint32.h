#pragma once
/*
* szVersion�ĸ�ʽ��: "***.***.***", ***�ķ�Χ��0~255
* 
*/
void VersionStr2Uint32(const char* szVersion, uint32_t& nVersion);

void VersionUint32toStr(char* szVersion, uint32_t nVersion);