#pragma once
class CEdoyunTool
{
public:
   static void Dump(BYTE* pData, size_t nSize)
    {
        std::string strOut;
        for (size_t i = 0; i < nSize; i++)
        {
            char buf[8] = "";
            if (i > 0 && i % 16 == 0)strOut += "/n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;
        }
        strOut += "/n";
        OutputDebugStringA(strOut.c_str());
    }
   static bool RunAsAdmin() {//获取管理检测，使用该权限创建进程
	  STARTUPINFO si = { 0 };
	  PROCESS_INFORMATION pi = { 0 };
	  TCHAR spath[MAX_PATH] = _T("");
	  GetModuleFileName(NULL, spath, MAX_PATH);
	  CString strCmd = spath;
	  BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL,
		   NULL, LOGON_WITH_PROFILE, NULL, (LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
	   if (!ret) {
		   ShowError();
		   MessageBox(NULL, strCmd, _T("创建进程失败"), 0);//TODO:去除调试哦信息
		   return false;
	   }
	   WaitForSingleObject(pi.hProcess, INFINITE);
	   CloseHandle(pi.hProcess);
	   CloseHandle(pi.hThread);
	   return true;
   }
   static bool IsAdmin() {// 查看是否管理员权限函数
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
   static void ShowError() {//错误信息函数
	   LPVOID lpMessageBuf = NULL;
	   FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		   NULL, GetLastError(),
		   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMessageBuf, 0, NULL);
	   OutputDebugString((LPWSTR)lpMessageBuf);
	   LocalFree(lpMessageBuf);
   }
   static bool  WriteStartupDir(const CString& strPath) {//通过修改开机启动文件夹来实现开机启动
	   TCHAR sPath[MAX_PATH] = _T("");
	   GetModuleFileName(NULL, sPath, MAX_PATH);
	   return  CopyFile(sPath, strPath, FALSE);
	   
   }
   static bool WriteRegisterTable(const CString& strPath) {//通过修改注册表来实现开机启动
	   CString  strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	   TCHAR sPath[MAX_PATH] = _T("");
	   GetModuleFileName(NULL, sPath, MAX_PATH);
	   BOOL ret = CopyFile(sPath, strPath, FALSE);
	   if (ret == FALSE) {
		   MessageBox(NULL, _T("复制文件失败,是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		   return false;
	   }
	   HKEY hKey = NULL;
	   ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
	   if (ret != ERROR_SUCCESS) {
		   RegCloseKey(hKey);
		   MessageBox(NULL, _T("设置自动开机启动失败!，是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		   return false;
	   }
	   TCHAR sSysPath[MAX_PATH] = _T("");
	   GetSystemDirectoryW(sSysPath, MAX_PATH);
	   ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
	   if (ret != ERROR_SUCCESS) {
		   RegCloseKey(hKey);
		   MessageBox(NULL, _T("设置自动开机启动失败!，是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		   return false;
	   }
	   RegCloseKey(hKey);
	   return true;
   }
   static bool Init() {//用户mfc命令行项目初始化(通用)
	   HMODULE hModule = ::GetModuleHandle(nullptr);
	   if (hModule == nullptr) {
		   wprintf(L"错误: GetModu leHandle 失败\n");
		   return false;
	   }
	   if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
	   {
		   // TODO: 在此处为应用程序的行为编写代码。
		   wprintf(L"错误: MFC 初始化失败\n");
		   return false;
	   }
	   return true;
   }
};

