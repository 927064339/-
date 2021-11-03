#pragma once
#include "pch.h"
#include"framework.h"
#include<string>
#include <vector>
#include<map>
#include<list>
#include<mutex>
#define WM_SEDN_PACK (WM_USER+1)//���Ͱ�����
#pragma pack(push)
#pragma pack(1)
class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize,HANDLE hEvent)  //�������
	{
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
		this->hEvent = hEvent;
	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		hEvent = pack.hEvent;
	}
	CPacket(const BYTE* pData, size_t& nSize) :hEvent(INVALID_HANDLE_VALUE) {
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {  //�ҵ���ͷ
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//�����ݿ��ܲ���ȫ,���߰�ͷδ����ȫ�ӽ��յ�
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//��δ��ȫ���յ�,�ͷ��ء�����ʧ��
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			TRACE("%s\r\n", strData.c_str());
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;//head2 length4 
			return;
		}
		nSize = 0;
	}
	~CPacket() {}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
			hEvent = pack.hEvent;
		}
		return *this;
	}
	int Size() {//�����ݵĴ�С
		return nLength + 6;
	}
	const char* Data(std::string& strOut) const {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)(pData) = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();

	}
public:
	WORD sHead;  //��ͷ�̶�FEFF
	DWORD nLength;//������(�ӿ������ʼ������У�����)
	WORD sCmd; //������
	std::string strData;//����
	WORD sSum;//��У��
	//std::string strOut;//������������
	HANDLE hEvent;
};
#pragma pack(pop)
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;

	}
	WORD nAction;//���������� ������ƶ���˫��
	WORD nButton;//������Ҽ����н�
	POINT ptXY;//����
}MOUSEEV, * PMOUSEEV;
typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid; // �Ƿ�Ϊ��Ч����
	BOOL IsDirectory;//�Ƿ� ΪĿ¼ 0�� 1 ��
	BOOL HasNext;//�Ƿ��к���
	char szFileName[256];//�ļ�����


}FILEINFO, * PFILEINFO;


std::string GetErrInfo(int wsaErrCode);
class CClientSocket
{

public:
	static CClientSocket* getInstance()        //����
	{
		if (m_instance == NULL)
		{
			m_instance = new CClientSocket();
		}
		return m_instance;

	}
	bool InitSocket();

#define  BUFFER_SIZE 4096000
	int DealCommand()  //��������
	{
		if (m_sock == -1)return false;
		//char buffer[1024]{0};
		char* buffer = m_buffer.data();
		static size_t index = 0;
		while (true)
		{
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
			if (((int)len <= 0 ) && ((int)index <=0)) {
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, index - len);
				index -= len;
				return m_packet.sCmd;
			}

		}
		return -1;

	}
	bool SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed = true);
	bool GetFiePath(std::string& strPath) {                        
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) {
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& Getpacket() {
		TRACE("packet=%d\r\n", m_packet);
		return m_packet;

	}
	void CloseSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
	void UpdateAddress(int nIP, int nPort) {
		if ((m_nIP != nIP) || (m_nPort != nPort)) {
			m_nIP = nIP;
			m_nPort = nPort;
		}
		

	}
	static void threadEntry(void* arg);
	void threadFunc();
	void threadFunc2();
private:
	typedef void (CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM
		wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC>m_mapFunc;
	HANDLE m_hThread;
	bool m_bAutoClose;
	std::mutex m_lock;
	std::list<CPacket>m_lstSend;
	std::map<HANDLE, std::list<CPacket>& >m_mapAck;
	std::map<HANDLE, bool>m_mapAutoClosed;
	int m_nIP;//��ַ
	int m_nPort;//�˿�
	std::vector<char>m_buffer;
	SOCKET m_sock;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& ss) {} //����������������˽��
	CClientSocket(const CClientSocket& ss)
	{
		m_hThread = INVALID_HANDLE_VALUE;
		m_bAutoClose = ss.m_bAutoClose;
		m_sock = ss.m_sock;
		m_nIP = ss.m_nIP;
		m_nPort = ss.m_nPort;
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
				TRACE("����ʧ��,��Ϣֵ:%d ����ֵ:08X,���:%d\r\n", funcs[i].message, funcs[i].func,i);
			}
		}
		
	}//�Ѹ������캯������Ϊ˽��
	CClientSocket():
		m_nIP(INADDR_ANY), m_nPort(0),m_sock
		(INVALID_SOCKET), m_bAutoClose(true),
		m_hThread(INVALID_HANDLE_VALUE)
	{
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���"), _T("��ʼ������!"), MB_OK | MB_ICONERROR);
			exit(0);

		}
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);
		
	}
	~CClientSocket()
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		WSACleanup();
	}
	BOOL InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
		{
			return FALSE;//TODO:����ֵ����
		}
		return TRUE;

	}
	static void releaseInstance()
	{
		if (m_instance != NULL)
		{
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;

		}
	}
	bool Send(const char* pData, int nSize) {
		if (m_sock == -1)return false;
		return send(m_sock, pData, nSize, 0) > 0;
	}
	bool Send(const CPacket& pack);
	void SendPack(UINT nMsg, WPARAM wParam/*��������ֵ*/, LPARAM lParam/*�������ĳ���*/);
	static CClientSocket* m_instance;
	class CHelper {
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};
