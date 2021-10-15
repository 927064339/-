#include "pch.h"
#include "ClientController.h"


std::map<UINT, CClientController::MSGFUNC>
CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;
CClientController* CClientController::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new CClientController();
		struct {
			UINT nMsg;
			MSGFUNC func;
		}MsgFuncs[] = {
			{WM_SEDN_PACK,&CClientController::onSedPack},
			{WM_SEND_DATA,&CClientController::onSedData},
			{WM_SHOW_STATUS,&CClientController::onShowStatus},
			{WM_SHOW_WATCH,&CClientController::onshowWatcher},
			{(UINT)-1,NULL}
		};
		for (int i = 0; MsgFuncs[i].nMsg != NULL; i++) {
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>
				(MsgFuncs[i].nMsg, MsgFuncs[i].func));
		}
	}
	return nullptr;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0,
		&CClientController::threadEntry, this, 0,
		&m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS, &m_remoteDlg);
	return 0;
}

int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}
LRESULT CClientController::SendMessage(MSG msg)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)return-2;
	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE,
		(WPARAM)&info, (LPARAM)&hEvent);
	WaitForSingleObject(hEvent, -1);
	return info.result;
}
void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	CWatchDialog dlg(&m_remoteDlg);
	m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadWatchScreen, 0, this);
	dlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);
}
void CClientController::threadWatchScreen()
{
	Sleep(50);
	while (!m_isClosed) {
		if (m_remoteDlg.isFull() == false) {
			int ret = SendCommandPacket(6);
			if (ret == 6) {
				if (GetImage(m_remoteDlg.GetImage()) == 0) {

					m_remoteDlg.SetImageStatus(true);
				}
				else {
					TRACE("获取图片失败！ret=%d\r\n", ret);
				}

			}
		}
		Sleep(1);
	}

}
void CClientController::threadWatchScreen(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatchScreen();
	_endthread();
}
void CClientController::threadDownloadFile()
{
	FILE* pFile = fopen(m_strLocal, "wb+");
	if (pFile == NULL) {
		AfxMessageBox("本地没有权限保存文件，或者无法创建！！！");
		m_statusDlg.ShowWindow(SW_HIDE);
		m_remoteDlg.EndWaitCursor();
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	do {
		int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)m_strRemote,
			m_strRemote.GetLength());
		long long nLength = *(long long*)pClient->Getpacket().strData.c_str();
		TRACE("c_str=%d\r\n", pClient->Getpacket().strData.c_str());
		if (nLength == 0) {
			AfxMessageBox("文件长度为零或者无法读取文件!!!");
			break;
		}
		long long nCount = 0;

		while (nCount < nLength) {
			ret = pClient->DealCommand();
			if (ret < 0) {
				AfxMessageBox("传输失败！！！");
				TRACE("传输失败：ret=%d\r\n", ret);
				break;
			}
			fwrite(pClient->Getpacket().strData.c_str(), 1, pClient->Getpacket().strData.size(), pFile);
			nCount += pClient->Getpacket().strData.size();
		}
	} while (false);
	fclose(pFile);
	pClient->CloseSocket();
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成!!"), _T("完成"));

}
void CClientController::threadDownloadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadDownloadFile();
	_endthread();
}
void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE) {
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find
			(msg.message);
			if (it != m_mapFunc.end()) {
				pmsg->result = (this->*it->second)(pmsg->msg.message,
					pmsg->msg.wParam, pmsg->msg.lParam);
			}
			else {
				pmsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else {
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find
			(msg.message);
			if (it != m_mapFunc.end()) {
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}
		
	}
}

unsigned _stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}

LRESULT CClientController::onSedPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket* pPacket = (CPacket*)wParam;
	return pClient->Send(*pPacket);
}

LRESULT CClientController::onSedData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CClientSocket* pClient = CClientSocket::getInstance();
	char* pBacket = (char*)wParam;
	return pClient->Send(pBacket,(int)lParam);
}

LRESULT CClientController::onShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::onshowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}
