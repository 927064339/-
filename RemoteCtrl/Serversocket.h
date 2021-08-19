#pragma once
#include "pch.h"
#include"framework.h"
#pragma pack(push)
#pragma pack(1)
class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)  //打包数据
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
	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {  //找到包头
				sHead = *(WORD*)(pData + i);
				i += 2;            
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//包数据可能不完全,或者包头未能完全接接收到
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//包未完全接收到,就返回。解析失败
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
		}
		return *this;
	}
	int Size() {//包数据的大小
		return nLength + 6;
	}
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;pData += 2;
		*(DWORD*)(pData) = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();

	}
public:
	WORD sHead;  //包头固定FEFF
	DWORD nLength;//包长度(从控制命令开始，到和校验结束)
	WORD sCmd; //控制命
	std::string strData;//数据
	WORD sSum;//和校验
	std::string strOut;//整个包的数据
};
#pragma pack(pop)
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;

	}
	WORD nAction;//描述动作。 点击，移动，双击
	WORD nButton;//左键、右键、中建
	POINT ptXY;//坐标
}MOUSEEV,*PMOUSEEV;
typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid; // 是否为有效数据
	BOOL IsDirectory;//是否 为目录 0否 1 是
	BOOL HasNext;//是否还有后续
	char szFileName[256];//文件名字


}FILEINFO, * PFILEINFO;
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
	bool InitSocket()
	{

		if (m_sock == -1)return false;


		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
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
		if((m_packet.sCmd >=2) && (m_packet.sCmd <= 4)){
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
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
private:
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
