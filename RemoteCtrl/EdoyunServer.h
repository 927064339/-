#pragma once
#include <MSWSock.h>
#include "EdoyunThread.h"
#include"CEdoyunQueue.h"
#include <map>
enum  EdoyunOperator{//��Ӧ��������
    ENone,
    EAccept,
    ERecv,
    ESend,
    EError
};
class EdoyunServer;
class EdoyunClient;
typedef std::shared_ptr<EdoyunClient>PCLIENT;
class EdoyunOverlapped {
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;//���� �μ�EdoyunOperator
    std::vector<char>m_buffer;//������
    ThreadWorker m_worker;//������
    EdoyunServer* m_server;//����������
};
template<EdoyunOperator>
class AcceptOverlapped;
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;

class EdoyunClient {
public:
	EdoyunClient();

	~EdoyunClient() {
		closesocket(m_sock);
	}
	void SetOverlapped(PCLIENT& ptr);
	operator SOCKET() {
		return m_sock;
	}
	operator PVOID() {
		return &m_buffer[0];
	}
	operator LPOVERLAPPED();

	operator LPDWORD() {
		return &m_received;
	}
	sockaddr_in* GetLocalAddr() { return &m_laddr; }
	sockaddr_in* GetRemoteAddr() { return &m_raddr; }
private:
	SOCKET m_sock;
	DWORD m_received;
	std::shared_ptr<ACCEPTOVERLAPPED> m_overlapped;
	std::vector<char>m_buffer;
	sockaddr_in m_laddr;
	sockaddr_in m_raddr;
	bool m_isbusy;
};
 template<EdoyunOperator>
class AcceptOverlapped :public EdoyunOverlapped, ThreadFuncBase
{
public:
	AcceptOverlapped();
	int AcceptWorker();
    PCLIENT m_client;
};
template<EdoyunOperator>
class RecvOverlapped :public EdoyunOverlapped, ThreadFuncBase
{
public:
    RecvOverlapped() :m_operator(ERecv), m_worker(this, &RecvOverlapped::RecvWorker) {
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		m_buffer.resize(1024*256);
	}
	int RecvWorker() {
		//TODO:

	}
};
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;
template<EdoyunOperator>
class SendOverlapped :public EdoyunOverlapped, ThreadFuncBase
{
public:
    SendOverlapped() :m_operator(ESend), m_worker(this, &SendOverlapped::SendWorker) {
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		m_buffer.resize(1024*256);
	}
	int SendWorker() {
		//TODO:

	}
};
typedef SendOverlapped<ESend> SENDOVERLAPPED;

template<EdoyunOperator>
class ErrorOverlapped :public EdoyunOverlapped, ThreadFuncBase
{
public:
    ErrorOverlapped() :m_operator(EError), m_worker(this, &ErrorOverlapped::ErrorWorker) {
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		m_buffer.resize(1024);
	}
	int ErrorWorker() {
		//TODO:

	}
};
typedef ErrorOverlapped<EError> EERROROVERLAPPED;



class EdoyunServer :
    public ThreadFuncBase
{
public:
    EdoyunServer(const std::string& ip="0.0.0.0",short port=9527) :m_pool(10) {
        m_hIocp = INVALID_HANDLE_VALUE;
        m_sock = INVALID_SOCKET;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    ~EdoyunServer(){}
    bool StartService() {
		CreateSocket();
		if (bind(m_sock, (sockaddr*)&m_addr, sizeof(m_addr)) == -1) {
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			return false;
		}
		if (listen(m_sock, 3) == -1) {
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			return false;
		}
		m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
		if (m_hIocp == NULL) {
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			m_hIocp = INVALID_HANDLE_VALUE;
			return false;
		}
		CreateIoCompletionPort((HANDLE)m_sock, m_hIocp, (ULONG_PTR)this, 0);
		m_pool.Invoke();
		m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&EdoyunServer::threadIocp));
		if (!NewAccept())return false;
		return true;
    }
	bool NewAccept() {
		PCLIENT pClient(new EdoyunClient());
		pClient->SetOverlapped(pClient);
		m_client.insert(std::pair<SOCKET, PCLIENT>(*pClient, pClient));
		if (!AcceptEx(m_sock, *pClient, *pClient, 0,
			sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			*pClient, *pClient))
		{
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			m_hIocp = INVALID_HANDLE_VALUE;
			return false;
		}
		return true;
	}
private:
	void CreateSocket() {
		m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		int opt = 1;
		setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	}
    int threadIocp() {
        DWORD tranferred = 0;
        ULONG_PTR CompletionKey = 0;
        OVERLAPPED* lpoverlapped = NULL;
        if (GetQueuedCompletionStatus(m_hIocp, &tranferred, &CompletionKey, &lpoverlapped, INFINITE)) {
            if (tranferred > 0 && (CompletionKey != 0)) {
                EdoyunOverlapped* pOverlapped = CONTAINING_RECORD(lpoverlapped, EdoyunOverlapped, m_overlapped);
                switch(pOverlapped->m_operator) {
                case EAccept:
				{
					ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)pOverlapped;
					m_pool.DispatchWorker(pOver->m_worker);
				}
				break;
                    
                case ERecv:
				{
					RECVOVERLAPPED* pOver = (RECVOVERLAPPED*)pOverlapped;
					m_pool.DispatchWorker(pOver->m_worker);
				}
                 break;
                case ESend:
				{
					SENDOVERLAPPED* pOver = (SENDOVERLAPPED*)pOverlapped;
					m_pool.DispatchWorker(pOver->m_worker);
				}
                 break;
                case EError:
				{
					EERROROVERLAPPED* pOver = (EERROROVERLAPPED*)pOverlapped;
					m_pool.DispatchWorker(pOver->m_worker);
				}
                 break;
                } 
            }
            else
            {
                return -1;
            }
           
          
        }
        return 0;
    }
private:
    EdoyunThreadPool m_pool;
    HANDLE m_hIocp;
    SOCKET m_sock;
    sockaddr_in m_addr;
    std::map<SOCKET, std::shared_ptr<EdoyunClient>>m_client;
};
