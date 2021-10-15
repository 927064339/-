
// RemoteClientDlg.h: 头文件
//

#pragma once
#include"ClientSocket.h"
#include"StatusDlg.h"
#define WM_SEND_PACKET (WM_USER + 1)//数据包的消息

// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIMEv
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
public:
	bool isFull()const {
		return m_isFull;
	}
	CImage& GetImage() {
		return m_image;
	}
	void SetImageStatus(bool isFull= false) {
		m_isFull = isFull;
	}
private:
	CImage m_image; //缓存
	bool m_isFull;//缓存是否有数据  true表示有缓存数据  false 表示没有缓存数据
private:
	void LoadFileCurrent();
	void LoadFileInfo();
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	
protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_nPort;
	afx_msg void OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTree1Dir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTree1Dir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickList1File(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloaoFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRueFile();
	afx_msg void OnTvnSelchangedTree1Dir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLvnItemchangedList1File(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEdit1Port();
};
 