#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include<mutex>
#define WM_SEND_PACK (WM_USER+1)//发送包数据
#define WM_SEND_PACK_ACK (WM_USER+2)//发送包数据应答
#pragma pack(push, 1)
typedef struct file_info {
	file_info() {
		IsInvalid = false;
		IsDirectory = -1;
		HasNext = true;
		memset(szFileName, 0, sizeof(szFileName));
	}
	bool IsInvalid;//是否有效
	bool IsDirectory;//是否为目录
	bool HasNext;//是否还有后续
	char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;
#pragma pack(pop)
enum {
	CSM_AUTOCLOSE=1,//CSM=Client socket Mode 自动关闭模式

};
typedef struct PacketData{
	std::string strData;
	UINT nMode;
	WPARAM wParam;
	PacketData(const char* pData, size_t nLen, UINT mode, WPARAM nParam=0) {
		strData.resize(nLen);
		memcpy((char*)strData.c_str(),pData,nLen);
		nMode = mode;
		wParam = nParam;
	}
	PacketData(const PacketData& data) {
		strData=data.strData;
		nMode = data.nMode;
		wParam = data.wParam;
	}
	PacketData& operator=(const PacketData& data) {
		if (this != &data) {
			strData = data.strData;
			nMode = data.nMode;
			wParam = data.wParam;
		}
		return *this;
	}
}PACKET_DATA;
class CPacket {
public:

	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize){
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}

		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sSum += BYTE(strData[j]) & 0xFF;
		}
		
	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	
	}
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		for (; i +1< nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 8 > nSize) {//包数据可能不全，或者包头未能全部接收到
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		if (nLength + i > nSize) {//包未完全接受到
			nSize = 0;
			return;
		}
		i += 4;
		sCmd = *(DWORD*)(pData + i);
		i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;

		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;

	}
	~CPacket() {}
	CPacket& operator=(const CPacket& pack) {
		if (this == &pack)return *this;

		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	
	}
	int Size() {
		return nLength + 6;
	}
	const char* Data(std::string& strOut) const {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)(pData) = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}
public:
	WORD sHead;//包头
	DWORD nLength;//包长度
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
	//std::string strOut;//整个包数据
};

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}

	WORD nAction;//点击，移动，双击
	WORD nButton;//左键，中键，右键
	POINT ptXY;//坐标
}MOUSEEV, * PMOUSEEV;
#include<vector>
#include <list>
#include <map>
std::string GetErrInfo(int wsaErrCode);

class ClientSocket
{
public:
	static ClientSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new ClientSocket();
		}
		return m_instance;
	}
	bool InitSocket();


#define BUFFER_SIZE 4096000
	int  DealCommand() {
		if (m_sock == -1)return false;
		//char buffer[1024] = "";
		char* buffer = m_buffer.data();
		static size_t index = 0;
		while (true)
		{
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
			if (((int)len <= 0)&&((int)index<=0))
			{
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len); 
			if (len > 0) {
				memmove(buffer, buffer + len, index - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	//bool SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed = true);
	bool SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed=true, WPARAM wParam=0);

	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) {
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
	void UpdateAddress(int nIP, int nPort) {
		if((m_nIP!=nIP)||(m_nPort!=nPort)){
			m_nIP = nIP;
			m_nPort = nPort;
		}
	}
private:
	HANDLE m_eventInvoke;//启动事件
	typedef void(ClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC> m_mapFunc;
	HANDLE m_hThread;
	UINT m_hThreadID;
	bool m_bAutoClose;
	std::mutex m_lock;
	std::list<CPacket> m_lstSend;
	std::map<HANDLE, std::list<CPacket>> m_mapAck;
	std::map<HANDLE, bool>m_mapAutoClosed;
	int m_nIP;
	int m_nPort;
	std::vector<char>m_buffer;
	SOCKET m_sock;
	CPacket m_packet;
	ClientSocket& operator=(const ClientSocket& ss) {}
	ClientSocket(const ClientSocket& ss);
	ClientSocket();
	~ClientSocket() {

		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		WSACleanup();
	}
	bool Send(const char* pData, size_t nSize)
	{
		if (m_sock == -1)return false;
		return send(m_sock, pData, nSize, 0) > 0;
	}
	bool Send(const CPacket& pack);
	void SendPack(UINT nMsg,WPARAM wParam/*缓冲区的值*/, LPARAM lParam/*缓冲区的长度*/);
	static unsigned _stdcall threadEntry(void* arg);
	//void threadFunc();
	void threadFunc2();
	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
		{
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance() {
		if (m_instance != NULL) {
			ClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}

	}
	static ClientSocket* m_instance;


	class CHelper {
	public:
		CHelper() {
			ClientSocket::getInstance();
		}
		~CHelper() {
			ClientSocket::releaseInstance();
		}
	};

	static CHelper m_helper;
};

