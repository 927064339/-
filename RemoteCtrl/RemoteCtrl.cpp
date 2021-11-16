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
CWinApp theApp;
using namespace std;
void WriteRegisterTable(const CString& strPath) {
    CString  strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	char sPath[MAX_PATH] = "";
	char sSys[MAX_PATH] = "";
	std::string strExe = "\\RemoteCtrl.exe ";
	GetCurrentDirectoryA(MAX_PATH, sPath);
	GetSystemDirectoryA(sSys, sizeof(sSys));
	std::string strCmd = "mklink" + std::string(sSys) + strExe + std::string(sPath) + strExe;//创造一个软连接
	int ret = system(strCmd.c_str());
	TRACE("ret =%d\r\n", ret);
	HKEY hKey = NULL;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		MessageBox(NULL, _T("设置自动开机启动失败!，是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		exit(0);
	}
	TCHAR sSysPath[MAX_PATH] = _T("");
	GetSystemDirectoryW(sSysPath, MAX_PATH);
	ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		MessageBox(NULL, _T("设置自动开机启动失败!，是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		exit(0);
	}
	RegCloseKey(hKey);
}
/*
*改bug思路
* 0观察现象
* 1确认范围
* 2分析错误的可能
* 3调试或者打日志,排查错误
* 4处理错误
* 5验证/长时间验证/多次验证/多条件验证
*/
void  WriteStartupDir(const CString& strPath) {
   
   CString strCmd = GetCommandLine();
   strCmd.Replace(_T("\""), _T(""));
   BOOL ret= CopyFile(strCmd,strPath, FALSE);
   if (ret == FALSE) {
       MessageBox(NULL, _T("复制文件失败,是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
       exit(0);
   }
 }
void ChooseAutoInvoke() {
    TCHAR wcsSystem[MAX_PATH] = _T("");
    CString strPath = CString(_T("C:\\Users\\coo boy\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe"));
    if (PathFileExists(strPath)) {
        return;
    }
  
    CString  strInfo = _T("该程序只允许用于合法的用途！\n");
    strInfo += _T("继续运行该程序,将使得这台机器处于被监控！\n");
    strInfo += _T("如果你不希望这样，请按“取消”按钮,退出程序.\n");
    strInfo += _T("按下“是”按钮,该程序将被复制到你的机器上\n");
    strInfo += _T("按下“否”按钮，程序只运行一次，不会在系统内留下任何东西！\n");
   int ret=  MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
   if (ret == IDYES) {
      //WriteRegisterTable(strPath)
       WriteStartupDir(strPath);
   }
   else if (ret == IDCANCEL) {
       exit(0);
   }
}
void ShowError() {//错误信息函数
    LPVOID lpMessageBuf = NULL;
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMessageBuf, 0, NULL);
    OutputDebugString((LPWSTR)lpMessageBuf);
    LocalFree(lpMessageBuf);
}
bool IsAdmin() {// 查看是否管理员权限函数
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        ShowError();
        return false;
    }
    TOKEN_ELEVATION eve;
    DWORD len = 0;
    if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
        ShowError();
        return false;
    }
    CloseHandle(hToken);
    if (len == sizeof(eve)) {
        return eve.TokenIsElevated;
    }
    printf("length of tokeninformation is %d\r\n", len);
    return false;
}
void RunAsAdmin() {//获取管理检测，使用该权限创建进程
    HANDLE hToken = NULL;
    BOOL ret = LogonUser(L"Administrator", NULL, NULL, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken);
    if (!ret) {
        ShowError();
        MessageBox(NULL, _T("登录错误！"), _T("程序错误"), 0);
        exit(0);
    }
    OutputDebugString(L"Logon administrator success!\r\n");
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    TCHAR spath[MAX_PATH] = _T("");
    GetCurrentDirectory(MAX_PATH, spath);
    CString strCmd = spath;
    strCmd += _T("\\RemoteCtrl.exe");
    ret = CreateProcessWithLogonW(_T("Administrator"), NULL,
        NULL, LOGON_WITH_PROFILE, NULL, (LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
    CloseHandle(hToken);
    if (!ret) {
        ShowError();
        MessageBox(NULL,strCmd,_T("创建进程失败"),0);
        exit(0);
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
int main()
{
    int nRetCode = 0;
   
    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
		{
			if (IsAdmin()) {
				OutputDebugString(L"current is run as adinistrator!\r\n");
			}
			else {
				OutputDebugString(L"current is run as normal user!\r\n");
				RunAsAdmin();
				return nRetCode;
			}

            CCommand cmd;
            ChooseAutoInvoke();
           int ret= CServersocket::getInstance()->Run(&CCommand::RunCommand,&cmd);
           switch (ret) {
           case -1:
               MessageBox(NULL, _T(""), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
               exit(0);
               break;
           case -2:
               MessageBox(NULL, _T("多次无法正常接入用户，结束程序"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
               exit(0);
               break;

           }
        }
    }

            
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModu leHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
