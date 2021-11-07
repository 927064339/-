#pragma once
#ifndef WM_SEDN_PACK_AC
#define WM_SEDN_PACK_ACK (WM_USER+2)//发送包数据应答
#endif
// CWatchDialog 对话框

class CWatchDialog : public CDialog
{
	DECLARE_DYNAMIC(CWatchDialog)

public:
	CWatchDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWatchDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_WATCH };
#endif
public:
	int m_objWidth;
	int m_nobjHeight;
	CImage m_image;
protected:
	bool m_isFull;//缓存是否有数据  true表示有缓存数据  false 表示没有缓存数据
	DECLARE_MESSAGE_MAP()
public:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	void SetImageStatus(bool isFull = false) {
		m_isFull = isFull;
	}
	bool isFull()const {
		return m_isFull;
	}
	CImage& GetImage() {
		return m_image;
	}
	CPoint UserpOint2RemoteScreenPoint(CPoint& point, bool isScreen=false);
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_picture;
	afx_msg LRESULT OnSendPackAck(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedWatch();
	virtual void OnOK();
	afx_msg void OnBnClickedBtnLock();
	afx_msg void OnBnClickedBtnUnlock();
};
