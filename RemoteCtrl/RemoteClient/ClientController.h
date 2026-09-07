#pragma once
#include"Clientsocket.h"
#include"WatchDialog.h"
#include"RemoteClientDlg.h"
#include"StatusDlg.h"
#include<map>
#include"resource.h"
#include "EdoyunTool.h"

//#define WM_SEND_DATA (WM_USER+2)//发送数据
#define WM_SHOW_STATUS (WM_USER+3)//展示状态
#define WM_SHOW_WATCH (WM_USER+4)//远程监控
#define WM_SEND_MESSAGE (WM_USER+0X1000)//自定义消息处理
class CClientController
{

public:

	static CClientController* getInstance();//获取单例
		int Invoke(CWnd*& pMainWnd);//启动
		int InitController();
		//发送消息
		LRESULT SendMessage(MSG msg);
		//更新网络服务器地址
		void UpdateAddress(int nIP, int nPort) {
			ClientSocket::getInstance()->UpdateAddress(nIP, nPort);
		}
		int DealCommand() {
			return ClientSocket::getInstance()->DealCommand();
		}
		void CloseSocket() {
			ClientSocket::getInstance()->CloseSocket();
		}

		//1.查看磁盘分区 
		// 2.查看指定目录下文件
		// 3.打开文件 
		// 4.下载文件
		// 5.鼠标操作
		// 6.发送屏幕内容
		// 7.锁机
		// 8.解锁机
		// 9.删除文件
		bool SendCommandPacket(
			HWND hWnd,//数据包收到后，需要应答的窗口
			int nCmd,
			bool bAutoClose = true,
			BYTE* pData = NULL,
			size_t nLength = 0
			
		);


		int GetImage(CImage& image) {
			ClientSocket* pClient = ClientSocket::getInstance();
			return CEdoyunTool::Bytes2Image(image, pClient->GetPacket().strData);

		}
		int DownFile(CString strPath);
		void StartWatchScreen();
protected:
	void threadWatchScreen();
	static void threadWatchScreen(void* arg);
	void threadDownloadFile();
	static void _stdcall threadDownloadEntry(void* arg);
	CClientController():
		m_statusDlg(&m_remoteDlg),
		m_watchDlg(&m_remoteDlg)
	{
		m_isWatchClosed = true;
		HANDLE m_hThreadWatch = INVALID_HANDLE_VALUE;
		m_hThreadDownload= INVALID_HANDLE_VALUE;
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController() {
		
		WaitForSingleObject(m_hThread, 100);
	}

	void threadFunc();
	static unsigned _stdcall threadEntry(void* arg);

	static void releaseInstance() {
		if (m_instance != NULL) {
			delete m_instance;
			m_instance = NULL;
		}
	}

	//LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:

	typedef struct MsgInfo{
		MSG msg;
		LRESULT result;
		HANDLE hEvent;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg,&m,sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
			hEvent = m.hEvent;
		}
		MsgInfo& operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
				hEvent = m.hEvent;
			}
			return *this;
		}
	}MSGINFO;
	typedef LRESULT(CClientController::*MSGFUNC)(UINT nMsg, WPARAM wParam,LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;

	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;

	HANDLE m_hThread;
	HANDLE m_hThreadDownload;
	HANDLE m_hThreadWatch;
	bool m_isWatchClosed;//监视是否关闭
	CString m_strRemote;
	CString m_strLocal;

	unsigned m_nThreadID;
	static CClientController* m_instance;

	class CHelper {
	public:
		CHelper() {
			//CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}
	};

	static CHelper m_helper;

};

