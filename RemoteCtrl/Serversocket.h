#pragma once
#include "pch.h"
#include"framework.h"
class CPacket
{
public:
	size_t i;
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize) {
		for (i = 0; i < nSize; i++) {
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
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[i]) & 0xFF;
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
		}
		return *this;
	}
public:
	WORD sHead;  //��ͷ�̶�FEFF
	DWORD nLength;//������(�ӿ������ʼ������У�����)
	WORD sCmd; //������������
	std::string strData;//����
	WORD sSum;//��У��
};
class CServersocket
{

public:
	static CServersocket* getInstance()
	{
		if (m_instance == NULL)
		{
			m_instance = new CServersocket();
		}
		return m_instance;

	}
	bool InitSocket()
	{

		if (m_sock == -1)return false;


		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		//���׽��ֵ�����
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
		sockaddr_in client_adr;
		memset(&client_adr, 0, sizeof(client_adr));

		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz); //�ֻ�һ��ȥ����
		if (m_client == -1)return false;
		return true;

	}
#define  BUFFER_SIZE 4096
	int DealCommand()  //��������
	{
		if (m_client == -1)return false;
		//char buffer[1024]{0};
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				return -1;

			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}

		}
		return -1;

	}
	bool Send(const char* pData, int nSize) {
		if (m_client == -1)return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
private:
	SOCKET m_client;
	SOCKET m_sock;
	CPacket m_packet;
	CServersocket& operator=(const CServersocket& ss) {} //����������������˽��
	CServersocket(const CServersocket& ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;

	}//�Ѹ������캯������Ϊ˽��
	CServersocket()
	{
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���"), _T("��ʼ������!"), MB_OK | MB_ICONERROR);
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
			return FALSE;//TODO:����ֵ����
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
