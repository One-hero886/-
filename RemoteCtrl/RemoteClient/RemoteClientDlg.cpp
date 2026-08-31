
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include <stdio.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "WatchDialog.h"


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_BTN_FILEINFO, m_Ttree);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

int CRemoteClientDlg::SendCommandPacket(int nCmd, bool bAutoClose , BYTE* pData, size_t nLength)
{
	UpdateData();
	ClientSocket* pClient = ClientSocket::getInstance();
	bool ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_nPort));
	if (!ret) {
		AfxMessageBox("网络初始化失败！！");
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	ret = pClient->Send(pack);
	TRACE("Send ret %d\r\n", ret);
	int cmd = pClient->DealCommand();
	TRACE("ack:%d\r\n", cmd);
	if (bAutoClose) {
		pClient->CloseSocket();
	}
	return cmd;
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DEL_FILE, &CRemoteClientDlg::OnDelFile)
	ON_COMMAND(ID_OPEN_FILE, &CRemoteClientDlg::OnOpenFile)
	ON_MESSAGE(WM_SEND_PACKET, &CRemoteClientDlg::OnSendPacket)
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	//m_server_address = 0x7f000001;
	m_server_address = 0xC0A8F383;//192.168.243.131
	m_nPort = _T("8080");
	UpdateData(FALSE);

	m_dlgStatus.Create(IDD_DLG_STATUS,this);
	m_dlgStatus.ShowWindow(SW_HIDE);
	m_isFull = false;


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void CRemoteClientDlg::OnBnClickedBtnTest()
{
	SendCommandPacket(1981);
}

void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	// TODO: 在此添加控件通知处理程序代码

	int ret=SendCommandPacket(1);
	if (ret == -1) {
		AfxMessageBox(_T("命令处理失败！！"));
	}
	ClientSocket* pClient = ClientSocket::getInstance();
	if (pClient == nullptr) {
		AfxMessageBox(_T("无法获取 ClientSocket 实例"));
		return;
	}
	std::string drivers=pClient->GetPacket().strData;
	std::string dr;
	m_Tree.DeleteAllItems();
	
	for (size_t i = 0; i < drivers.size(); i++) {
		if (drivers[i] == ',') {
			dr += ":";
			HTREEITEM hTemp=m_Tree.InsertItem(dr.c_str(),TVI_ROOT,TVI_LAST);
			m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
	if (!dr.empty()) {
		dr += ":";
		m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
		dr.clear();
	}
}
void CRemoteClientDlg::threadEntryForWatchData(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadWatchData();
	_endthread();
}
void CRemoteClientDlg::threadWatchData()
{
	Sleep(50);
	ClientSocket* pClient = NULL;
	do {
		pClient = ClientSocket::getInstance();

	} while (pClient == NULL);
	ULONGLONG tick = GetTickCount64();
	while(!m_isWatchClosed) {
		if (m_isFull == false) {
			//CPacket pack(6, NULL, 0);
			//bool ret = pClient->Send(pack);
			int ret = SendMessage(WM_SEND_PACKET, 6 << 1 | 1);
			if (ret == 6) {
				BYTE* pData = (BYTE*)pClient->GetPacket().strData.c_str();

				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
				if (hMem == NULL) {
					TRACE("内存不足！！！");
					Sleep(1);
					continue;
				}
				IStream* pStream = NULL;
				HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
				if (hRet == S_OK) {
					ULONG length = 0;
					pStream->Write(pData, pClient->GetPacket().strData.size(), &length);
					LARGE_INTEGER bg = { 0 };
					pStream->Seek(bg, STREAM_SEEK_SET, NULL);
					if((HBITMAP)m_image!=NULL)
						m_image.Destroy();

					m_image.Load(pStream);
					m_isFull = true;
				}
			}
			else {
				Sleep(1);
			}
		}
		else Sleep(1);
	}
}
void CRemoteClientDlg::threadEntryForDownFile(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadDownFile();
	_endthread();

}
void CRemoteClientDlg::threadDownFile()
{
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);

	CFileDialog dlg(FALSE, "*", m_List.GetItemText(nListSelected, 0), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
		, NULL, this);

	if (dlg.DoModal() == IDOK) {
		FILE* pFile = fopen(dlg.GetPathName(), "wb+");
		if (pFile == NULL) {
			AfxMessageBox("文件无法创建！！");
			m_dlgStatus.ShowWindow(SW_HIDE);
			EndWaitCursor();
			return;
		}
		HTREEITEM hSelected = m_Tree.GetSelectedItem();

		strFile = GetPath(hSelected) + _T("\\") + strFile;
		ClientSocket* pClient = ClientSocket::getInstance();
		TRACE(_T("获取的文件路径%s\r\n"), strFile);
		do {
			//int ret = SendCommandPacket(4, false, (BYTE*)(LPCTSTR)strFile, strFile.GetLength());
			int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCSTR)strFile);
			if (ret < 0) {
				AfxMessageBox("执行下载命令失败！！");
				TRACE(_T("执行下载失败:ret=%d\r\n"), ret);
				break;
			}
			long long nlength = *(long long*)pClient->GetPacket().strData.c_str();
			if (nlength == 0) {
				AfxMessageBox("文件长度为零或者无法读取文件！！");
				break;
			}
			long long nCount = 0;
			while (nCount < nlength) {
				ret = pClient->DealCommand();
				if (ret < 0) {
					AfxMessageBox("传输失败！！");
					TRACE(_T("传输失败:ret=%d\r\n"), ret);
					break;
				}
				fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
				nCount += pClient->GetPacket().strData.size();
			}
		} while (0);
		fclose(pFile);
		pClient->CloseSocket();
	}
	m_dlgStatus.ShowWindow(SW_HIDE);
	EndWaitCursor();
	MessageBox(_T("下载完成！！"), _T("完成"));
}
void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL) {
		TRACE(_T("[调试] HitTest 未命中任何节点！\n"));
		return;
	}

	DeleteTreeChildrenItem(hTreeSelected);

	CString strPath = GetPath(hTreeSelected);

	//DeleteTreeChildrenItem(hTreeSelected);
	m_List.DeleteAllItems();
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)ClientSocket::getInstance()->GetPacket().strData.c_str();

	ClientSocket* pClient = ClientSocket::getInstance();
	while (pInfo->HasNext ) {
		TRACE("[%s] is a dir &d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (pInfo->IsDirectory) {
			if (CString(pInfo->szFileName) == "." || (CString(pInfo->szFileName) == "..")) {
				int cmd = pClient->DealCommand();
				TRACE("ack:%d\r\n", cmd);
				if (cmd < 0) break;
				pInfo = (PFILEINFO)ClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			m_Tree.InsertItem("", hTemp, TVI_LAST);
		}
		else {
			m_List.InsertItem(0,pInfo->szFileName);
		}

		int cmd = pClient->DealCommand();
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0) break;
		pInfo = (PFILEINFO)ClientSocket::getInstance()->GetPacket().strData.c_str();

	}
	pClient->CloseSocket();
}
//void ccc() {
//
//	CString strTestPath = _T("C:\\Windows");
//	std::string sTestPath = CT2A(strTestPath);
//	ClientSocket* pClient = ClientSocket::getInstance();
//
//	int nCmd = SendCommandPacket(2, false, (BYTE*)sTestPath.c_str(), (int)sTestPath.length());
//	TRACE(_T(">>> [测试发送] 路径: %s, 字节数: %d\n"), (LPCTSTR)strTestPath, (int)sTestPath.length());
//
//	if (nCmd < 0) {
//		TRACE(_T(">>> [测试] 发送失败！\n"));
//		return;
//	}
//	const std::string& data = pClient->GetPacket().strData;
//	TRACE(_T(">>> [测试接收] 实际收到数据字节大小: %d 字节 (FILEINFO大小应为 %d 字节)\n"),
//		(int)data.size(), (int)sizeof(FILEINFO));
//
//	CString strHex = _T("");
//	int nPrintLen = min((int)data.size(), 64);
//	for (int i = 0; i < nPrintLen; ++i) {
//		CString strByte;
//		strByte.Format(_T("%02X "), (BYTE)data[i]);
//		strHex += strByte;
//	}
//	TRACE(_T(">>> [测试原始十六进制数据]: %s\n"), (LPCTSTR)strHex);
//
//}
void CRemoteClientDlg::LoadFileInfo1()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL) {
		TRACE(_T("[调试] HitTest 未命中任何节点！\n"));
		return;
	}

	DeleteTreeChildrenItem(hTreeSelected);

	CString strPath = GetPath(hTreeSelected);

	//DeleteTreeChildrenItem(hTreeSelected);
	m_List.DeleteAllItems();
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)ClientSocket::getInstance()->GetPacket().strData.c_str();

	ClientSocket* pClient = ClientSocket::getInstance();
	while (pInfo->HasNext) {
		CString strName = CA2T(pInfo->szFileName);
		TRACE("[%s] is a dir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
			if (CString(pInfo->szFileName) == "." || (CString(pInfo->szFileName) == "..")) {
				int cmd = pClient->DealCommand();
				TRACE("ack:%d\r\n", cmd);
				if (cmd < 0) break;
				pInfo = (PFILEINFO)ClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
		if (pInfo->IsDirectory) {

			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			if (hTemp == NULL) {
				TRACE(_T(">>> [错误] 目录节点插入失败: [%s]\n"), (LPCTSTR)strName);
			}
			else {
				TRACE(_T(">>> [成功] 插入目录节点: [%s], 句柄: %p\n"), (LPCTSTR)strName, hTemp);
				// 2. 插入占位符
				m_Tree.InsertItem(_T(""), hTemp, TVI_LAST);
			}
			//m_Tree.InsertItem("", hTemp, TVI_LAST);
		}
		else {
			m_List.InsertItem(m_List.GetItemCount(), pInfo->szFileName);
		}

		int cmd = pClient->DealCommand();
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0) break;
		pInfo = (PFILEINFO)ClientSocket::getInstance()->GetPacket().strData.c_str();

	}
	pClient->CloseSocket();

}
void CRemoteClientDlg::LoadFileCurrent()
{
	HTREEITEM hTree= m_Tree.GetSelectedItem();
	CString strPath = GetPath(hTree);
	m_List.DeleteAllItems();

	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)ClientSocket::getInstance()->GetPacket().strData.c_str();

	ClientSocket* pClient = ClientSocket::getInstance();
	while (pInfo->HasNext ) {
		TRACE("[%s] is a dir &d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (!pInfo->IsDirectory) {
			m_List.InsertItem(0,pInfo->szFileName);
		}

		int cmd = pClient->DealCommand();
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0) break;
		pInfo = (PFILEINFO)ClientSocket::getInstance()->GetPacket().strData.c_str();

	}
	pClient->CloseSocket();

}
//void ccc() {
//	CString strRet, strTmp;
//	do {
//		strTmp = m_Tree.GetItemText(hTree);
//		strRet += strTmp+'\\' + strRet;
//		hTree = m_Tree.GetParentItem(hTree);
//	} while (hTree != NULL);
//	return strRet;
//}
CString CRemoteClientDlg::GetPath(HTREEITEM hTree) {
	CString strPath = _T("");
	while (hTree != NULL)
	{
		CString strItem = m_Tree.GetItemText(hTree);
		strItem.Trim();
		if (!strItem.IsEmpty())
		{
			if (strPath.IsEmpty())
				strPath = strItem;
			else
				strPath = strItem + _T("\\") + strPath;
		}
		hTree = m_Tree.GetParentItem(hTree);
	}
	if (strPath.GetLength() == 2 && strPath.GetAt(1) == _T(':'))
	{
		strPath += _T("\\");
	}
	return strPath; // 拼接结果示例: "D:\KEIL5"

}

void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{

	HTREEITEM hChild = m_Tree.GetChildItem(hTree);
	while (hChild != NULL) {
		DeleteTreeChildrenItem(hChild);
		m_Tree.DeleteItem(hChild);
		hChild = m_Tree.GetChildItem(hTree); 
	}
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	//LoadFileInfo();
	LoadFileInfo1();
}

void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	//LoadFileInfo();
	LoadFileInfo1();
}

void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse,ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);
	int ListSelected = m_List.HitTest(ptList, 0);
	if (ListSelected <0) {
		TRACE(_T("[调试] HitTest 未命中任何节点！\n"));
		return;
	}

	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPupup = menu.GetSubMenu(0);
	if (pPupup != NULL) {
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
	}
}

void CRemoteClientDlg::OnDownloadFile()
{
	// TODO: 在此添加命令处理程序代码

	//添加线程函数
	_beginthread(CRemoteClientDlg::threadEntryForDownFile, 0, this);
	BeginWaitCursor();

	m_dlgStatus.m_info.SetWindowText(_T("命令正在执行中！！"));
	m_dlgStatus.ShowWindow(SW_SHOW);
	m_dlgStatus.SetActiveWindow();
	m_dlgStatus.CenterWindow(this);
	Sleep(50);
}

void CRemoteClientDlg::OnDelFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();

	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	strFile = GetPath(hSelected) + _T("\\") + strFile;
	TRACE(_T("删除文件名称：%s\r\n"),strFile);
	int ret = SendCommandPacket(9, true, (BYTE*)(LPCTSTR)strFile, strFile.GetLength());
	if (ret < 0) {
		AfxMessageBox("删除文件命令失败！！");
		TRACE(_T("删除文件失败:ret=%d\r\n"), ret);
		return;
	}

	LoadFileCurrent();
}

void CRemoteClientDlg::OnOpenFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	strFile = GetPath(hSelected) + _T("\\") + strFile;

	int ret=SendCommandPacket(3, true, (BYTE*)(LPCTSTR)strFile, strFile.GetLength());
	if (ret < 0) {
		AfxMessageBox("执行打开文件命令失败！！");
		TRACE(_T("打开文件失败:ret=%d\r\n"), ret);
		return;
	}

}

LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)
{
	int ret = 0;
	int cmd = wParam >> 1;
	switch (cmd)
	{
	case 4: 
	{
		CString strFile = (LPCSTR)lParam;
		ret = SendCommandPacket(cmd,wParam & 1, (BYTE*)(LPCTSTR)strFile, strFile.GetLength());
	}
	break;
	case 5:
	{
		ret = SendCommandPacket(cmd, wParam & 1,(BYTE*)lParam,sizeof(MOUSEEV));
	}
	break;
	case 6:
	case 7:
	case 8:
	{
		ret = SendCommandPacket(cmd, wParam & 1);
	}
	break;
	default:
		ret = -1;
	}
	return ret;
}

void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	m_isWatchClosed = false;
	CWatchDialog dlg(this);
	// TODO: 在此添加控件通知处理程序代码
	HANDLE hThread=(HANDLE)_beginthread(CRemoteClientDlg::threadEntryForWatchData, 0, this);
	//GetDlgItem(IDC_BTN_START_WATCH)->EnableWindow(FALSE);
	dlg.DoModal();
	m_isWatchClosed = true;
	WaitForSingleObject(hThread, 500);
}
