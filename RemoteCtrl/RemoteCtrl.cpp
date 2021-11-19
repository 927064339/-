// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "Serversocket.h"
#include"Command.h"
#include<conio.h>
#include "CEdoyunQueue.h"
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
	if (PathFileExists(strPath)) {
		return true;
	}
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
#define  IOCP_LTST_EMPTY 0
#define  IOCP_LTST_PUSH 1
#define  IOCP_LTST_POP  2

enum {
	IocpListEmpty,
	IocpListpush,
	IocpListpop
};
typedef struct IocpParam {
	int nOperator;//操作
	std::string strData;//数据
	_beginthread_proc_type cbFunc;//回调
	IocpParam(int op, const char* sData, _beginthread_proc_type cb = NULL) {
		nOperator = op;
		strData = sData;
		cbFunc = cb;
	}
	IocpParam() {
		nOperator = -1;
	}
}IOCP_PAPAM;

void threadmain(HANDLE hIOCP) {
	std::list<std::string>lstString;
	DWORD dwTransferred = 0;
	ULONG_PTR Completionkey = 0;
	OVERLAPPED* pOverlapped = NULL;
	while (GetQueuedCompletionStatus(hIOCP, &dwTransferred, &Completionkey, &pOverlapped, INFINITE)) {//接收PostQueuedCompletionStatus的消息
		if (dwTransferred == 0 && (Completionkey == NULL)) {
			printf("thread is prepare to exit\r\n");
			break;
		}
		IOCP_PAPAM* pParam = (IOCP_PAPAM*)Completionkey;
		if (pParam->nOperator == IocpListpush) {
			lstString.push_back(pParam->strData);
		}
		else if (pParam->nOperator == IocpListpop) {
			std::string str;
			if (lstString.size() > 0) {
				str = lstString.front();
				lstString.pop_front();
			}
			if (pParam->cbFunc) {
				pParam->cbFunc(&str);
			}

		}
		else if (pParam->nOperator == IocpListEmpty) {
			lstString.clear();
		}
		delete pParam;
	}
}
void threadQueueEntry(HANDLE hIOCP) {
	threadmain(hIOCP);
	_endthread();
}
void func(void* arg) {
	std::string* pstr = (std::string*)arg;
	if (pstr != NULL) {
		printf("pop from list:%s\r\n", pstr->c_str());
		delete pstr;
	}
	else {
		printf("list is empty,no data\r\n");
	}
	
}
void test(){
	printf("press any key to exit .....\r\n");
	CEdoyunQueue<std::string> lstStrings;
	ULONGLONG tick0 = GetTickCount64(), tick = GetTickCount64(), total = GetTickCount64();
	while (GetTickCount64() -total<=1000) {//完成端口，把请求与实现分离了
		if (GetTickCount64() - total >13) {
			lstStrings.PushBack("hello world");
			tick0 = GetTickCount64();
		}
		if (GetTickCount64() - tick > 20) {
			std::string str;
			lstStrings.PopFront(str);
			tick = GetTickCount64();
			printf("pop from queue:%s\r\n", str.c_str());
		}
		Sleep(1);
	}
	printf("exit done!size %d\r\n", lstStrings.Size());
	lstStrings.Clear();
	printf("exit done!size %d\r\n", lstStrings.Size());

}/*
 1 BUG测试/功能测试
 2关键因素的测试（内存泄漏，运行的稳定，条件性）
 3 压力测试（可靠性测试）
 4 性能测试
 */
int main()
{
	if (!CEdoyunTool::Init)return 1;
	for (int i = 0; i < 10; i++) {
		test();
	}
	//CEdoyunQueue<std::string> lstStrings;
	//ULONGLONG tick0 = GetTickCount64(),tick=GetTickCount64();
	//while (_kbhit() == 0) {//完成端口，把请求与实现分离了
	//	if (GetTickCount64() - tick0> 1300) {
	//		lstStrings.PushBack("hello world");
	//		tick0 = GetTickCount64();
	//	}
	//	if (GetTickCount64() - tick >2000) {
	//		std::string str;
	//		lstStrings.PopFront(str);
	//		tick = GetTickCount64();
	//		printf("pop from queue:%s\r\n", str.c_str());
	//	}
	//	Sleep(1);
	//}
	//printf("exit done!size %d\r\n", lstStrings.Size());
	//lstStrings.Clear();
	//printf("exit done!size %d\r\n", lstStrings.Size());

	/*if ( CEdoyunTool::IsAdmin()) {
		OutputDebugString(L"current is run as adinistrator!\r\n");
	}
	else {
		OutputDebugString(L"current is run as normal user!\r\n");
		CEdoyunTool::RunAsAdmin();
		return 0;
	}*/
	/*CCommand cmd;
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
	}*/
	
	return 0;
}
