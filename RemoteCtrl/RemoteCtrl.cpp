// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "Serversocket.h"
#include"Command.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//C:\Users\coo boy\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
// 唯一的应用程序对象
//开机启动的时候,程序权限是跟随用户的
#define  INVOKE_PATH _T("C:\\Users\\coo boy\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")
CWinApp theApp;
using namespace std;

/*
*改bug思路
* 0观察现象
* 1确认范围
* 2分析错误的可能
* 3调试或者打日志,排查错误
* 4处理错误
* 5验证/长时间验证/多次验证/多条件验证
*/
//业务和通用
bool ChooseAutoInvoke(const CString& strPath) {
    TCHAR wcsSystem[MAX_PATH] = _T("");
	PathFileExists(strPath);
    CString  strInfo = _T("该程序只允许用于合法的用途！\n");
    strInfo += _T("继续运行该程序,将使得这台机器处于被监控！\n");
    strInfo += _T("如果你不希望这样，请按“取消”按钮,退出程序.\n");
    strInfo += _T("按下“是”按钮,该程序将被复制到你的机器上\n");
    strInfo += _T("按下“否”按钮，程序只运行一次，不会在系统内留下任何东西！\n");
   int ret=  MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
   if (ret == IDYES) {
      //WriteRegisterTable(strPath)
	   if (!CEdoyunTool::WriteStartupDir(strPath)) {
		   MessageBox(NULL, _T("复制文件失败,是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		   return false;
	   }
   }
   else if (ret == IDCANCEL) {
	   return false;
   }
   return true;
}
int main()
{
	/*if ( CEdoyunTool::IsAdmin()) {
		OutputDebugString(L"current is run as adinistrator!\r\n");
	}
	else {
		OutputDebugString(L"current is run as normal user!\r\n");
		CEdoyunTool::RunAsAdmin();
		return 0;
	}*/
    if (!CEdoyunTool::Init)return 1;
	CCommand cmd;
	if (ChooseAutoInvoke(INVOKE_PATH)) {
		int ret = CServersocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
		switch (ret) {
		case -1:
			MessageBox(NULL, _T(""), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
			break;
		case -2:
			MessageBox(NULL, _T("多次无法正常接入用户，结束程序"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
			break;
		}
	}
	
	return 0;
}
