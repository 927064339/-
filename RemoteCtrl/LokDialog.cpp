// LokDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "LokDialog.h"
#include "afxdialogex.h"


// CLokDialog 对话框

IMPLEMENT_DYNAMIC(CLokDialog, CDialog)

CLokDialog::CLokDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_INFO, pParent)
{

}

CLokDialog::~CLokDialog()
{
}

void CLokDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLokDialog, CDialog)
END_MESSAGE_MAP()


// CLokDialog 消息处理程序
