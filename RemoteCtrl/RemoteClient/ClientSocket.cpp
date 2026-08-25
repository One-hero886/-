#include "pch.h"
#include "ClientSocket.h"



ClientSocket* ClientSocket::m_instance = NULL;
ClientSocket::CHelper ClientSocket::m_helper;
ClientSocket* pserver = ClientSocket::getInstance();
