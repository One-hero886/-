// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"

// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_WATCH, pParent)
{
	m_nObjHeight = -1;
	m_nObjWidth = -1;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch1)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序

CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen)
{
	CRect clientRect;
	if(isScreen)ScreenToClient(&point);//全局坐标转为客户区坐标

	CRect rect;
	m_picture.GetWindowRect(rect);
	ScreenToClient(rect);

	int relX = point.x - rect.left;
	int relY = point.y - rect.top;

	int ctrlWidth = rect.Width();
	int ctrlHeight = rect.Height();
	if (ctrlWidth <= 0 || ctrlHeight <= 0 || m_nObjWidth <= 0 || m_nObjHeight <= 0) {
		return CPoint(0, 0);
	}

	// 限制在控件范围内
	relX = max(0, min(relX, ctrlWidth - 1));
	relY = max(0, min(relY, ctrlHeight - 1));


	// 4. 比例换算到远程屏幕坐标
	int remoteX = (int)((double)relX * m_nObjWidth / ctrlWidth);
	int remoteY = (int)((double)relY * m_nObjHeight / ctrlHeight);

	//本地坐标到远程屏幕坐标
	//float width0=clientRect.Width();
	//float height0= clientRect.Height();
	//int width = 2560, height = 1600;
	return CPoint(remoteX, remoteY);
		//CPoint(point.x * m_nObjWidth / width0,point.y *m_nObjHeight / height0);
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();


	//m_picture.MoveWindow(0, 0, 2560, 1600);


	SetTimer(0, 300,NULL);



	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	 //TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull()) {
			CRect rect;
			m_picture.GetWindowRect(rect);
			if (m_nObjWidth == -1) {
				m_nObjWidth = pParent->GetImage().GetWidth();
			}
			if (m_nObjHeight == -1) {
				m_nObjHeight = pParent->GetImage().GetHeight();
			}
			pParent->GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(),
				0, 0,rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);


			pParent->GetImage().Destroy();
			pParent->SetImageStatus();

		}
	}
	CDialog::OnTimer(nIDEvent);

	//if (nIDEvent == 0) {
	//	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	//	if (pParent && pParent->isFull()) {
	//		if (!pParent->GetImage().IsNull()) {

	//			// 1. 获取监控对话框当前的整个显示区域（无论窗口多大）
	//			CRect rect;
	//			GetClientRect(rect);

	//			// 2. 直接获取对话框窗口的 DC 绘制
	//			CDC* pDC = GetDC();
	//			if (pDC != NULL) {
	//				// 设置平滑缩放模式，防止画面失真
	//				SetStretchBltMode(pDC->GetSafeHdc(), HALFTONE);

	//				// 3. 将整张大图自动拉伸铺满当前的窗口区域 rect
	//				pParent->GetImage().Draw(pDC->GetSafeHdc(), rect);

	//				ReleaseDC(pDC); // 释放 DC
	//			}

	//			pParent->GetImage().Destroy();
	//		}
	//		pParent->SetImageStatus();
	//	}
	//}
	//CDialog::OnTimer(nIDEvent);

}

void CWatchDialog::OnStnClickedWatch1()
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		
		CPoint point;
		GetCursorPos(&point);

		CPoint remotePoint = UserPoint2RemoteScreenPoint(point, true);

		MOUSEEV event;
		event.ptXY = remotePoint;
		event.nAction = 0;//左键
		event.nButton = 0;//单击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);
	}
}

void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{

	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {


		// TODO: 在此添加消息处理程序代码和/或调用默认值
		CPoint remotePoint = UserPoint2RemoteScreenPoint(point);

		MOUSEEV event;
		event.ptXY = remotePoint;
		event.nAction = 1;//左键
		event.nButton = 0;//双击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		TRACE("x=%d y=%d\r\n", point.x, point.y);
		CPoint remotePoint = UserPoint2RemoteScreenPoint(point);
		TRACE("x=%d y=%d\r\n", point.x, point.y);
		MOUSEEV event;
		event.ptXY = remotePoint;
		event.nAction = 2;//左键
		event.nButton = 0;//按下
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		CPoint remotePoint = UserPoint2RemoteScreenPoint(point);

		MOUSEEV event;
		event.ptXY = remotePoint;
		event.nAction = 3;//左键
		event.nButton = 0;//双击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);

	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		CPoint remotePoint = UserPoint2RemoteScreenPoint(point);

		MOUSEEV event;
		event.ptXY = remotePoint;
		event.nAction = 1;//右键
		event.nButton = 1;//双击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}

void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		CPoint remotePoint = UserPoint2RemoteScreenPoint(point);

		MOUSEEV event;
		event.ptXY = remotePoint;
		event.nAction = 2;//右键
		event.nButton = 1;//按下
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);
	}
	CDialog::OnRButtonDown(nFlags, point);
}

void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		CPoint remotePoint = UserPoint2RemoteScreenPoint(point);

		MOUSEEV event;
		event.ptXY = remotePoint;
		event.nAction = 3;//左键
		event.nButton = 1;//放开
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);

	}

	CDialog::OnRButtonUp(nFlags, point);
}

void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		CPoint remotePoint = UserPoint2RemoteScreenPoint(point);

		MOUSEEV event;
		event.ptXY = remotePoint;
		event.nAction = 8;//无按键
		event.nButton = 0;//移动
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);


	}
	CDialog::OnMouseMove(nFlags, point);
}

void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}

void CWatchDialog::OnBnClickedBtnLock()
{
	// TODO: 在此添加控件通知处理程序代码
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);
}

void CWatchDialog::OnBnClickedBtnUnlock()
{
	// TODO: 在此添加控件通知处理程序代码
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1);
}
