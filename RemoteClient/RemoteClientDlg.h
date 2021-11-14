
// RemoteClientDlg.h: 头文件
//

#pragma once
#include"ClientSocket.h"
#include"StatusDlg.h"
#ifndef WM_SEDN_PACK_AC
#define WM_SEDN_PACK_ACK (WM_USER+2)//发送包数据应答
#endif
// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数
	void LoadFileInfo();

// 对话框数据
#ifdef AFX_DESIGN_TIMEv
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
private://TODO：代码即文档
	void DealCommand(WORD nCmd,const std::string& strData,LPARAM lParam);//命令处理函数
	void InitUIData();//客户端ui
	void LoadFileCurrent();//加载文件当前信息
	CString GetPath(HTREEITEM hTree);//获取当前路径
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	void Str2Tree(const std::string& drivers, CTreeCtrl& tree);//显示系统磁盘
	void UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent);//查找文件
	void UpdateDownloadFile(const std::string& strData, FILE* pFile);//下载文件
	
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
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLvnItemchangedList1File(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEdit1Port();
	afx_msg LRESULT OnSendPackAck(WPARAM wParam, LPARAM lParam);
};
 