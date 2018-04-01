#include "stdafx.h"
#include "MessageDisplay.h"
#include <cstring>
MessageDisplay::MessageDisplay()
{
}


MessageDisplay::~MessageDisplay()
{
}

void MessageDisplay::ShowMessage(const char* cFormat, ...)
{
	memset(m_szBuff, 0, sizeof(m_szBuff));
	va_list args = NULL;
	va_start(args, cFormat);
	vsnprintf(m_szBuff, MESSAGE_BUF_LEN - 1, cFormat, args);
	va_end(args);
	AddMessage(m_szBuff);
}

//void MessageDisplay::AddMessage(const char* message)
//{
//
//}
