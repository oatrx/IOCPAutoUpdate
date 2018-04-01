#include "stdafx.h"
#include "Server.h"
#include "IOCPServer.h"
#include "MessageDisplay.h"
//#include "vld.h"
Server::Server()
	: m_pIOCPServer(NULL)
{
}


Server::~Server()
{
	if (NULL != m_pIOCPServer)
	{ 
		delete m_pIOCPServer;
		m_pIOCPServer = NULL;
	}
}

bool Server::start(MessageDisplay* pMessageDlg)
{
	if (NULL == m_pIOCPServer)
		m_pIOCPServer = new(std::nothrow) IOCPServer(pMessageDlg);
	if (NULL == m_pIOCPServer)
		return false;
	if (!m_pIOCPServer->LoadSockLib())
		return false;
	if (!m_pIOCPServer->Start())
		return false;
}

void Server::stop()
{
	assert(m_pIOCPServer != NULL);
	m_pIOCPServer->Stop();
	m_pIOCPServer->UnloadSockLib();
}
