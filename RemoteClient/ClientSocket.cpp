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
		AfxMessageBox("ָ����ip��ַ������");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret == -1) {
		AfxMessageBox("����ʧ��");
		TRACE("����ʧ��:&d %s\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()).c_str());
		return false;
	}


	return true;

}

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed)
{
	if (m_sock == INVALID_SOCKET && m_hThread==INVALID_HANDLE_VALUE) {
		/*if (InitSocket() == false)return false;*/
		m_hThread=(HANDLE)_beginthread
		(&CClientSocket::threadEntry, 0, this);
		TRACE("start thread\r\n");
	}
	m_lock.lock();
	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPacks));
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
	TRACE("cmd:%d event %08X thread id%d\r\n", pack.sCmd, pack.hEvent, GetCurrentThreadId());
	m_lstSend.push_back(pack);
	m_lock.unlock();
	WaitForSingleObject(pack.hEvent, INFINITE);
	std::map < HANDLE, std::list<CPacket>& >::iterator it;
	it = m_mapAck.find(pack.hEvent);
	if (it != m_mapAck.end()) {
		m_lock.lock();
		m_mapAck.erase(it);
		m_lock.unlock();
		return true;
	}
	return false;
}
void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
}

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	 int index = 0;
	 InitSocket();
	while (m_sock != INVALID_SOCKET) {
		if (m_lstSend.size() > 0) {
			TRACE("lstSend siize= %d\r\n", m_lstSend.size());
			m_lock.lock();
			CPacket& head = m_lstSend.front();
			m_lock.unlock();
			if (Send(head) == false) {
				TRACE("����ʧ�ܣ�\r\n");
				continue;
			}
			std::map <HANDLE, std::list<CPacket>& >::iterator it;
			it = m_mapAck.find(head.hEvent);
			if (it != m_mapAck.end()) {
				std::map<HANDLE, bool>::iterator it0 =
					m_mapAutoClosed.find(head.hEvent);
				do {
					int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
					if (length > 0 || index > 0) {
						index += length;
						size_t size = (size_t)index;
						CPacket pack((BYTE*)pBuffer, size);
						if (size > 0) {
							//TODO:
							pack.hEvent = head.hEvent;
							it->second.push_back(pack);
							memmove(pBuffer, pBuffer + size, index - size);
							index -= size;
							if (it0->second) {
								SetEvent(head.hEvent);
								break;
							}
						}

					}
					else if (length <= 0 && index <= 0) {
						CloseSocket();
						SetEvent(head.hEvent);//�ȵ��������ر�������֪ͨ�������
						if (it0 != m_mapAutoClosed.end()) {
						}
						else {
							TRACE("�쳣�����,û�ж�Ӧ��pair\r\n");
						}
						break;
					}
				} while (it0->second == false);
				
			}
			m_lock.lock();
			m_lstSend.pop_front();
			m_mapAutoClosed.erase(head.hEvent);
			m_lock.unlock();
			if (InitSocket() == false) {
				InitSocket();
			}
		}
		Sleep(1);
	}
	CloseSocket();
}

void CClientSocket::threadFunc2()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (m_mapFunc.find(msg.message) != m_mapFunc.end()) {
		  	(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
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
{//TODO:����һ����Ϣ�����ݽṹ(���ݺ����ݳ��ȣ�ģʽ)   �ص���Ϣ�����ݽṹ(HWND MESSAGE)
	if (InitSocket() == true) {
		int ret = send(m_sock, (char*)wParam, int(lParam), 0);
		if (ret>0)
		{
		}
		else {
			CloseSocket();
		}
	}
	else {
		//TODO:������
	}
	
	//TODO:
}
