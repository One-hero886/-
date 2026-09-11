#pragma once
#include "pch.h"
#include "framework.h"
#include<list>
#include"Packet.h"


typedef void(*SOCK_CALLBACK)(void* , int ,std::list<CPacket>&,CPacket&);
class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) { 
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool InitSocket(short port) {
	
		if (m_sock == -1) return false;
		sockaddr_in serv_addr;

		memset(&serv_addr,0,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(port);
		if (bind(m_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		{
			return false;
		}
		if (listen(m_sock, 1) == -1)
		{
			return false;
		}

		return true;
	}

	int Run(SOCK_CALLBACK callback, void* arg, short port = 8080) {

		bool ret = InitSocket(port);
		if (ret == false) return -1;
		std::list<CPacket> lstPackets;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
		while (true) {
			if (AcceptClient() == FALSE) {
				if (count >= 3) {
					return -2;
				}
				count++;
			}
			int ret = DealCommand();
			if (ret > 0) {
				m_callback(m_arg, ret, lstPackets,m_packet);
				while (lstPackets.size() > 0) {
					Send(lstPackets.front());
						lstPackets.pop_front();
				}
			}
			CloseClient();
		}
		return 0;
	}
	bool AcceptClient() {

		sockaddr_in client_addr;
		int cli_size = sizeof(client_addr);
		m_client = accept(m_sock,(sockaddr*)&client_addr,&cli_size);
		TRACE("m_client=%d\r\r",m_client);
		if (m_client == -1)return false;

		return true;
	}
#define BUFFER_SIZE 4096
	int  DealCommand() {
		if (m_client == -1)return false;
		//char buffer[1024] = "";
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL) {
			TRACE("内存不足！！\r\r");
		}
		memset(buffer, 0, BUFFER_SIZE);

		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer+index, BUFFER_SIZE -index, 0);
			if (len <= 0)
			{
				delete[]buffer;
				return -1;
			}
			TRACE("recv len %d\r\n", len);
			index += len;
			m_packet=CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer,buffer+len, BUFFER_SIZE-len);
				index -= len;
				delete[]buffer;
				return m_packet.sCmd;
			}
		}
		delete[]buffer;
		return -1;
	}
	bool Send(const char* pData, size_t nSize)
	{
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack){
		if (m_client == -1)return false;
		//Dump((BYTE*)pack.Data(), pack.Size());
		return send(m_client, pack.Data(), pack.Size(), 0);

	}


	void CloseClient() {
		if (m_client != INVALID_SOCKET) {
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		}

	}
private:
	SOCK_CALLBACK m_callback;
	void* m_arg;
	SOCKET m_sock,m_client;
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& ss) {}
	CServerSocket(const CServerSocket& ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerSocket() {
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL,_T("无法初始化套接字环境,请检查网络设置"),_T("初始化失败"),MB_OK|MB_ICONERROR);
			exit(0);
		}

		m_sock= socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServerSocket() {

		closesocket(m_sock);
		WSACleanup();
	}

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(2, 0), &data) != 0)
		{
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}

	}
	static CServerSocket* m_instance;
	

	class CHelper {
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};

	static CHelper m_helper;
};

