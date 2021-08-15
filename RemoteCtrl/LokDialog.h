#pragma once


// CLokDialog 对话框

class CLokDialog : public CDialog
{
	DECLARE_DYNAMIC(CLokDialog)

public:
	CLokDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CLokDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
