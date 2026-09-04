#include "pch.h"
#include "ClientSocket.h"



ClientSocket* ClientSocket::m_instance = NULL;
ClientSocket::CHelper ClientSocket::m_helper;
ClientSocket* pserver = ClientSocket::getInstance();
std::string GetErrInfo(int wsaErrCode) {
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}

void ClientSocket::threadEntry(void* arg) {
	ClientSocket* thiz = (ClientSocket*)arg;
	thiz->threadFunc();

}
void ClientSocket::threadFunc() {

	if (InitSocket() == false) {
		return;
	}
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (CHAR*)strBuffer.c_str();

	int index = 0;
	while (m_sock != INVALID_SOCKET) {
		if (m_lstSend.size() > 0) {
			CPacket& head = m_lstSend.front();
			if (Send(head) == false) {
				TRACE("·¢ËÍÊ§°Ü£¡\r\n");
				continue;
			}
			auto pr=m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()))
				;
			std::list<CPacket> lstRecv;
			int length = recv(m_sock, pBuffer, BUFFER_SIZE - index, 0);
			if (length > 0 || index > 0) {
				index += length;
				size_t size = (size_t)index;
				CPacket pack((BYTE*)pBuffer, size);
				 
				if (size > 0) {
					pack.hEvent = head.hEvent;
					pr.first->second.push_back(pack);
					SetEvent(head.hEvent);
				}
				continue;
			}
			else if(length <=0&&index <=0){
				CloseSocket(); 
			}
			m_lstSend.pop_front();
		}

	}
}