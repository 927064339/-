#pragma once

#include "Resource.h"
#include<map>
#include "Serversocket.h"
#include <direct.h>
#include <atlimage.h>
#include <stdio.h>
#include <io.h>
#include <list>
#include"CEdoyunTool.h"
#include "LokDialog.h"
class CCommand
{
public:
	CCommand();
	~CCommand() {};
	 int ExcuteCommand(int nCmd);
protected:
	typedef int (CCommand::* CMDFUNC)();//成员函数指针
	std::map<int, CMDFUNC>m_mapFunction;//从命令号到功能的映射
    CLokDialog dlg;
    unsigned threadid;

protected:
  static  unsigned  __stdcall threadLockDlg(void* arg)
    {
      CCommand* thiz = (CCommand*)arg;
      thiz->threadLockDlgMain();
        _endthreadex(0);
        return 0;
    }
  void threadLockDlgMain()
  {
      TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
      dlg.Create(IDD_DIALOG_INFO, NULL);
      dlg.ShowWindow(SW_SHOW);
      //遮蔽后台窗口
      CRect rect;
      rect.left = 0;
      rect.top = 0;
      rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//w1
      rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
      rect.bottom = LONG(rect.bottom * 1.10); //把y的像素点扩大
      dlg.MoveWindow(rect);
      CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
      if (pText) {
          CRect rtText;
          pText->GetWindowRect(rtText);
          int nWidth = rtText.Width();//w0
          int x = (rect.right - nWidth) / 2;
          int nHeight = rtText.Height();
          int y = (rect.bottom - nHeight) / 2;
          pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
      }
      //窗口置顶
      dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOSIZE);
      //限制鼠标
      ShowCursor(false);
      //隐藏任务栏
      ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);

      ClipCursor(rect);//限制鼠标活动范围
      rect.left = 0;
      rect.top = 0;
      rect.right = 1;
      rect.bottom = 1;
      MSG msg;
      while (GetMessage(&msg, NULL, 0, 0)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
          if (msg.message == WM_KEYDOWN) {
              TRACE("msg:%08X wparam:%08X  lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
              if (msg.wParam == 0x41) {  //按下a键推出循环
                  break;
              }

          }
      }
      //恢复鼠标
      ShowCursor(true);
      //恢复任务栏
      ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
      dlg.DestroyWindow();
  }
    int MakeDriverInfo()
    {
        std::string result;
        for (int i = 1; i <= 26; i++) {
            if (_chdrive(i) == 0) {
                if (result.size() > 0)
                    result += ',';
                result += 'A' + i - 1;

            }
        }
        result += ',';
        CPacket pack(1, (BYTE*)result.c_str(), result.size());
        CEdoyunTool::Dump((BYTE*)pack.Data(), pack.Size());
        CServersocket::getInstance()->Send(pack);
        return 0;
    }


    int MakeDirectoryInfo() {
        std::string strPath;
        //std::list<FILEINFO>lstFileInfos;
        if (CServersocket::getInstance()->GetFiePath(strPath) == false) {
            OutputDebugString(_T("当前的命令,不是获取文件的列表,命令解析错误！！"));
            return -1;

        }
        if (_chdir(strPath.c_str()) != 0) {
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServersocket::getInstance()->Send(pack);
            OutputDebugString(_T("没有权限,访问目录！！"));
            return -2;
        }
        _finddata_t fdata;
        int hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            OutputDebugString(_T("没有找到任何文件！！"));
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServersocket::getInstance()->Send(pack);
            return -3;
        }
        int count = 0;
        do {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
            TRACE("%s\r\n", finfo.szFileName);
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServersocket::getInstance()->Send(pack);
            //lstFileInfos.push_back(finfo);
            count++;
        } while (!_findnext(hfind, &fdata));
        TRACE("server:count = %d\r\n", count);
        //发送信息到控制端
        FILEINFO  finfo;
        finfo.HasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServersocket::getInstance()->Send(pack);
        return 0;
    }
    int RunFile()   //打开文件
    {
        std::string strPath;
        CServersocket::getInstance()->GetFiePath(strPath);
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        CPacket pack(3, NULL, 0);
        CServersocket::getInstance()->Send(pack);
        return 0;
    }
    int  DownloadFile()  //下载文件
    {
        std::string strPath;
        CServersocket::getInstance()->GetFiePath(strPath);
        long long data = 0;
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
        if (err != 0) {
            CPacket pack(4, (BYTE*)&data, 8);
            CServersocket::getInstance()->Send(pack);
            return -1;
        }
        if (pFile != NULL) {
            fseek(pFile, 0, SEEK_END);
            data = _ftelli64(pFile);
            CPacket head(4, (BYTE*)&data, 8);
            CServersocket::getInstance()->Send(head);
            fseek(pFile, 0, SEEK_SET);
            char buffer[1024] = "";
            size_t rlen = 0;
            do {
                rlen = fread(buffer, 1, 1024, pFile);
                CPacket pack(4, (BYTE*)buffer, rlen);
                CServersocket::getInstance()->Send(pack);
            } while (rlen >= 1024);

            fclose(pFile);

        }
        CPacket pack(4, NULL, 0);
        CServersocket::getInstance()->Send(pack);
        return 0;
    }
    int MouseEvent() {
        MOUSEEV mouse;
        if (CServersocket::getInstance()->GetMouseEvent(mouse)) {
            DWORD nFlags = 0;
            switch (mouse.nButton) {
            case 0://鼠标左键
                nFlags = 1;
                break;
            case 1://鼠标右键
                nFlags = 2;
                break;
            case 2://鼠标中键
                nFlags = 4;
                break;
            case 4:
                nFlags = 8;
                break;
            }
            if (nFlags != 8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
            switch (mouse.nAction) {
            case 0:  //单击动作
                nFlags |= 0x10;
                break;
            case 1://双击动作
                nFlags |= 0x20;
                break;
            case 2://按下动作
                nFlags |= 0x40;
                break;
            case 3://放开
                nFlags |= 80;
                break;
            default:
                break;
            }
            switch (nFlags)
            {
            case 0x21://左键双击动作
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x11:  //左键单击动作
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x41://左键按下动作
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x81://左键放开
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x22://右键双击动
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x12:  //右键单击动作
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x42://右键按下动作
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x82://右键放开
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x24://中键双击动作
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x14:  //中键单击动作
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x44://中键按下动作
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x84://中键放开
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x08://鼠标移动
                mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
                break;
            }
            CPacket pack(5, NULL, 0);
            CServersocket::getInstance()->Send(pack);
        }
        else {
            OutputDebugString(_T("获取鼠标操作参数失败！！！"));
            return -1;
        }
        return 0;
    }
    int SendScreen()
    {
        CImage screen;//GDI
        HDC hScreen = ::GetDC(NULL);
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL); //获取像素点
        int nWidth = GetDeviceCaps(hScreen, HORZRES);   //宽
        int nHeight = GetDeviceCaps(hScreen, VERTRES);  //高
        screen.Create(nWidth, nHeight, nBitPerPixel);
        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);//复制原截图
        ReleaseDC(NULL, hScreen);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
        if (hMem == NULL)return -1;
        IStream* pStream = NULL;
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatJPEG);
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);
            PBYTE pData = (PBYTE)GlobalLock(hMem);
            SIZE_T nSize = GlobalSize(hMem);
            CPacket pack(6, pData, nSize);
            CServersocket::getInstance()->Send(pack);
            GlobalUnlock(hMem);
        }
        pStream->Release();
        GlobalFree(hMem);
        screen.ReleaseDC();
        return 0;
    }
	int  LockMachine() {
		if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
			//_beginthread(threadLockDlg, 0, NULL);
			_beginthreadex(NULL, 0, &CCommand::threadLockDlg, this, 0, &threadid);
			TRACE("threadid=%d\r\n", threadid);

			return 0;
		}

		CPacket pack(7, NULL, 0);
		CServersocket::getInstance()->Send(pack);


		return 0;
    }
int UnlockMachine()
{

    PostThreadMessage(threadid, WM_KEYDOWN, 0X41, 0);

    CPacket pack(8, NULL, 0);
    CServersocket::getInstance()->Send(pack);
    return 0;
}
int TestConnect()
{
    CPacket pack(1981, NULL, 0);
    bool ret = CServersocket::getInstance()->Send(pack);
    TRACE("Send ret=%d\r\n", ret);
    return 0;
}
int  DeleteLocalFile()
{
    std::string strPath;
    CServersocket::getInstance()->GetFiePath(strPath);
    TCHAR sPath[MAX_PATH] = _T("");
    // mbstowcs(sPath, strPath.c_str(), strPath.size());
    MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(),
        sPath, sizeof(sPath) / sizeof(TCHAR));
    DeleteFileA(strPath.c_str());
    CPacket pack(9, NULL, 0);
    bool ret = CServersocket::getInstance()->Send(pack);
    TRACE("Send ret=%d\r\n", ret);
    return 0;
}
};

