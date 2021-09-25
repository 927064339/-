#pragma once
#include "pch.h"
#include"framework.h"
#include<list>
#include"Packet.h"

typedef void(*SOCKT_CALLBACK)(void* , int ,std::list<CPacket>&,CPacket&);
class CServersocket
{

public:
	static CServersocket* getInstance()        //单例
	{
		if (m_instance == NULL)
		{
			m_instance = new CServersocket();
		}
		return m_instance;

	}

	int Run(SOCKT_CALLBACK callback, void* arg,short port=9527) {
		bool ret = InitSocket(port);
		if (ret == false)return -1;
		std::list<CPacket>lstPackets;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
		while (true) {
			if (AcceptClient() == false) {
				if (count >= 3) {
					return -2;
				}
				count++;
			}
			int ret = DealCommand();
			while(ret > 0) {
				m_callback(m_arg, ret, lstPackets,m_packet);
				if (lstPackets.size() > 0) {
					Send(lstPackets.front());
					lstPackets.pop_front();
				}
			}
			CloseClient();
		}
		return 0;
	}
protected:
	bool InitSocket(short port)
	{

		if (m_sock == -1)return false;


		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(port);
		//绑定套接字到本地
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		{
			return false;
		}//TODO:
		//TODO:
		if (listen(m_sock, 1) == -1) {
			return false;
		}

		return true;

	}
	bool AcceptClient()
	{
		TRACE("enter AcceptClient\r\n");
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz); //分机一个去服务
		TRACE("m_client=%d\r\n", m_client);
		if (m_client == -1)return false;
		return true;

	}
#define  BUFFER_SIZE 4096
	int DealCommand()  //接受数据
	{
		if (m_client == -1)return false;
		//char buffer[1024]{0};
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL) {
			TRACE("内存不足!\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				delete[]buffer;
				return -1;
			}
			TRACE("recv %d\r\n", len);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[]buffer;
				return m_packet.sCmd;
			}

		}
		delete[]buffer;
		return -1;

	}
	bool Send(const char* pData, int nSize) {
		if (m_client == -1)return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1)return false;
		return send(m_client,pack.Data(), pack.Size(), 0) > 0;
	}
	bool GetFiePath(std::string& strPath) {                        
		if((m_packet.sCmd >=2) && (m_packet.sCmd <= 4)||
			(m_packet.sCmd==9)){
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent( MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& Getpacket() {
		return m_packet;
	}
		
	void CloseClient()
	{
		if (m_client != INVALID_SOCKET) {
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		}
	}
		
private:
	SOCKT_CALLBACK m_callback;
	void* m_arg;
	SOCKET m_client;
	SOCKET m_sock;
	CPacket m_packet;  //接受的数据包
	CServersocket& operator=(const CServersocket& ss) {} //把重载运算符构造成私有
	CServersocket(const CServersocket& ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;

	}//把副本构造函数设置为私有
	CServersocket()
	{
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("无法初始化套接字环境"), _T("初始化错误!"), MB_OK | MB_ICONERROR);
			exit(0);

		} 
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServersocket()
	{
		closesocket(m_sock);
		WSACleanup();
	}
	BOOL InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
		{
			return FALSE;//TODO:返回值处理
		}
		return TRUE;

	}
	static void releaseInstance()
	{
		if (m_instance != NULL)
		{
			CServersocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;

		}
	}
	static CServersocket* m_instance;
	class CHelper {
	public: 
		CHelper() {
			CServersocket::getInstance();
		}
		~CHelper() {
			CServersocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};
