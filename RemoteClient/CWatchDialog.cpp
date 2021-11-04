// CWatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "CWatchDialog.h"
#include "afxdialogex.h"
#include"ClientController.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{   
	m_isFull = false;
	m_objWidth = -1;
	m_nobjHeight = -1;
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
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序

CPoint CWatchDialog::UserpOint2RemoteScreenPoint(CPoint& point,bool isScreen)
{//客户端 800 450
	CRect clientRect;
	if(!isScreen)ClientToScreen(&point);//转换为相对屏幕左上角的坐标（屏幕内的绝对的坐标）
    m_picture.ScreenToClient(&point);//转换为客户区域坐标（相对picture控件左上角坐标）
	TRACE("x=%d y=%d\r\n", point.x, point.y);
	//本地坐标，到远程坐标
	m_picture.GetWindowRect(&clientRect);
	return CPoint(point.x * m_objWidth/ clientRect.Width(), point.y * m_nobjHeight / clientRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_isFull = false;
	SetTimer(0, 45, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		CClientController* pParent = CClientController::getInstance();
		if (m_isFull) {
			CRect rect;
			m_picture.GetWindowRect(rect);
			m_objWidth = m_image.GetWidth();
			m_nobjHeight = m_image.GetHeight();
			m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0,
				rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);
			TRACE("更新图片完成%d %d\r\n", m_objWidth, m_nobjHeight, (HBITMAP)m_image);
			m_image.Destroy();
			m_isFull = false;
			
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point) //左键双击
{
	//坐标转换
	if ((m_nobjHeight != -1) && (m_objWidth != -1)) {


		CPoint remot = UserpOint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remot;
		event.nButton = 0;//左键
		event.nAction = 2;//双击
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),5, true,(BYTE*)&event,sizeof(event));
	
	}
	CDialog::OnLButtonDblClk(nFlags, point);
	
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nobjHeight != -1) && (m_objWidth != -1)) {
		CPoint remot = UserpOint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remot;
		event.nButton = 0;//左键
		event.nAction = 2;//按下
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nobjHeight != -1) && (m_objWidth != -1)) {
		CPoint remot = UserpOint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remot;
		event.nButton = 0;//左键
		event.nAction = 3;//弹起
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nobjHeight != -1) && (m_objWidth != -1)) {
		CPoint remot = UserpOint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remot;
		event.nButton = 1;//右键
		event.nAction = 1;//双击
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nobjHeight != -1) && (m_objWidth != -1)) {
		CPoint remot = UserpOint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remot;
		event.nButton = 1;//右键
		event.nAction = 2;//按下
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nobjHeight != -1) && (m_objWidth != -1)) {
		CPoint remot = UserpOint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remot;
		event.nButton = 1;//左键
		event.nAction = 3;//弹起
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nobjHeight != -1) && (m_objWidth != -1)) {
		CPoint remot = UserpOint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remot;
		event.nButton = 8;//没有按键
		event.nAction = 0;//移动
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	if ((m_nobjHeight != -1) && (m_objWidth != -1)) {
		CPoint point;
		GetCursorPos(&point);
		CPoint remot = UserpOint2RemoteScreenPoint(point, true);
		//封装
		MOUSEEV event;
		event.ptXY = remot;
		event.nButton = 0;//左键
		event.nAction = 0;//单击
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}

}


void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),7);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	// TODO: 在此添加控件通知处理程序代码
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),8);
}
