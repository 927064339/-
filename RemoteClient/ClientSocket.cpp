#include "pch.h"
#include "ClientSocket.h"
CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pserver = CClientSocket::getInstance();
std::string GetErrInfo(int wsaErrCode)
{
	std::string ret;
	LPVOID lpMsgBuff = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuff, 0, NULL);
	ret = (char*)lpMsgBuff;
	LocalFree(lpMsgBuff);

	return ret;
}
CClientSocket::CClientSocket(const CClientSocket& ss) {
	m_hThread = INVALID_HANDLE_VALUE;
	m_bAutoClose = ss.m_bAutoClose;
	m_sock = ss.m_sock;
	m_nIP = ss.m_nIP;
	m_nPort = ss.m_nPort;
	std::map<UINT, CClientSocket::MSGFUNC>::const_iterator it = ss.m_mapFunc.begin();
	for (; it != ss.m_mapFunc.end(); it++) {
		m_mapFunc.insert(std::pair<UINT, MSGFUNC>(it->first, it->second));
	}
}
CClientSocket::CClientSocket():
	m_nIP(INADDR_ANY), m_nPort(0), m_sock
	(INVALID_SOCKET), m_bAutoClose(true),
	m_hThread(INVALID_HANDLE_VALUE)
{
	if (InitSockEnv() == FALSE)
	{
		MessageBox(NULL, _T("无法初始化套接字环境"), _T("初始化错误!"), MB_OK | MB_ICONERROR);
		exit(0);

	}
	m_eventInvoke = CreateEvent(NULL, TRUE, FALSE,NULL);
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
	if (WaitForSingleObject(m_eventInvoke, 100) == WAIT_TIMEOUT) {
		TRACE("网络消息处理线程启动失败\r\n");
	}
	CloseHandle(m_eventInvoke);
	m_buffer.resize(BUFFER_SIZE);
	memset(m_buffer.data(), 0, BUFFER_SIZE);
	struct {
		UINT message;
		MSGFUNC func;
	}funcs[] = {
		{WM_SEDN_PACK,&CClientSocket::SendPack},
		//{WM_SEDN_PACK,},
		{0,NULL}
	};
	for (int i = 0; funcs[i].message != 0; i++) {
		if (m_mapFunc.insert(std::pair<UINT, MSGFUNC>(funcs[i].message, funcs[i].func)).second == false) {
			TRACE("插入失败,消息值:%d 函数值:08X,序号:%d\r\n", funcs[i].message, funcs[i].func, i);
		}
	}

}
//把副本构造函数设置为私有
bool CClientSocket::InitSocket()
{
	if (m_sock != INVALID_SOCKET)CloseSocket();
	m_sock = socket(PF_INET, SOCK_STREAM, 0);

	if (m_sock == -1)return false;


	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	TRACE("addr %08X nIP%X\r\n", inet_addr("127.0.0.1"), m_nIP);

	serv_adr.sin_addr.s_addr = htonl(m_nIP);
	serv_adr.sin_port = htons(m_nPort);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
		AfxMessageBox("指定的ip地址不存在");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret == -1) {
		AfxMessageBox("连接失败");
		TRACE("连接失败:&d %s\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()).c_str());
		return false;
	}


	return true;
}
bool CClientSocket::SendPacket(HWND hwnd, const CPacket& pack, bool isAutoClosed,WPARAM wParam) {
	UINT nMode = isAutoClosed ? CSM_AUTOCLSE : 0;
	std::string strOut;
	pack.Data(strOut);
	PACKET_DATA* pData = new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, wParam);
	bool ret = PostThreadMessage(m_nThreadID, WM_SEDN_PACK, (WPARAM)new PACKET_DATA(strOut.c_str(),strOut.size() , nMode, wParam), (LPARAM)hwnd);
	if (ret == false) {
		delete pData;
	}
	return ret;
}

//bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed)
//{
//	if (m_sock == INVALID_SOCKET && m_hThread==INVALID_HANDLE_VALUE) {
//		/*if (InitSocket() == false)return false;*/
//		m_hThread=(HANDLE)_beginthread
//		(&CClientSocket::threadEntry, 0, this);
//		TRACE("start thread\r\n");
//	}
//	m_lock.lock();
//	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPacks));
//	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
//	TRACE("cmd:%d event %08X thread id%d\r\n", pack.sCmd, pack.hEvent, GetCurrentThreadId());
//	m_lstSend.push_back(pack);
//	m_lock.unlock();
//	WaitForSingleObject(pack.hEvent, INFINITE);
//	std::map < HANDLE, std::list<CPacket>& >::iterator it;
//	it = m_mapAck.find(pack.hEvent);
//	if (it != m_mapAck.end()) {
//		m_lock.lock();
//		m_mapAck.erase(it);
//		m_lock.unlock();
//		return true;
//	}
//	return false;
//}
unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc2();
	_endthreadex(0);
	return 0;
}


void CClientSocket::threadFunc2()
{
	SetEvent(m_eventInvoke);
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
		if (it != m_mapFunc.end()) {
		  	(this->*it->second)(msg.message, msg.wParam, msg.lParam);
		}
	}

}

bool CClientSocket::Send(const CPacket& pack)
{
	TRACE("m_sock=%d\r\n", m_sock);
	if (m_sock == -1)return false;
	std::string strOut;
	pack.Data(strOut);
	return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
}

void CClientSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
 {//TODO:定义一个消息的数据结构(数据和数据长度，模式)   回调消息的数据结构(HWND MESSAGE)
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	HWND hWnd = (HWND)lParam;
	delete(PACKET_DATA*)wParam;
	if (InitSocket()!=false) {
		int ret = send(m_sock, (char*)data.strData.c_str(), data.strData.size(), 0);
		if (ret > 0) {
			size_t index = 0;
			std::string strBuffer;
			strBuffer.resize(BUFFER_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();
			while(m_sock != INVALID_SOCKET) {
				int length = recv(m_sock,pBuffer+index,BUFFER_SIZE-index,0);
				if (length > 0) {
					index += (size_t)length;
					size_t nLen = index;
					CPacket pack((BYTE*)pBuffer, nLen);
					if (nLen > 0 || (index>0)) {
						::SendMessage(hWnd, WM_SEDN_PACK_ACK, (WPARAM)new CPacket(pack), data.wParam);
						if (data.nMode & CSM_AUTOCLSE) {
							CloseSocket();
							return;
						}
					}
					index -= nLen;
					memmove(pBuffer, pBuffer + index, nLen);
				}
				else {//TODO:对方关闭了套接字，或者网络设备异常
					CloseSocket();
					::SendMessage(hWnd, WM_SEDN_PACK_ACK, NULL, 1);
				}
				
			}
			
		}
		else {
			CloseSocket();
			::SendMessage(hWnd, WM_SEDN_PACK_ACK, NULL, -1);
		}
	}
	else {
		::SendMessage(hWnd, WM_SEDN_PACK_ACK, NULL, -2);
		//TODO:错误处理
	}
}
