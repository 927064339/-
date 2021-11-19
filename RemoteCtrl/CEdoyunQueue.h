#pragma once
#include "pch.h"
#include <atomic>
#include <iostream>
#include <list>
template<class T>
class CEdoyunQueue
{	//�̰߳�ȫ�Ķ��У�����iocpʵ�֣�
 public:
	enum {
		EQNone,
		EQpush,
		EQpop,
		EQSize,
		EQClear
	};
	typedef struct IocpParam {
		size_t nOperator;//����
		T Data;//����
		HANDLE hEvent;//pop������Ҫ
		IocpParam(int op, const T& data,HANDLE hEve=NULL) {
			nOperator = op;
			Data = data;
			hEvent = hEve;
		}
		IocpParam() {
			nOperator = EQNone;
		}
	}PPARAM;//Post Parameter ����Ͷ����Ϣ�Ľṹ��
	
public:
	CEdoyunQueue() {
		m_lock = false;
		m_hCompeletionPort= CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);//epoll�������1//����iocp
		m_hTread = INVALID_HANDLE_VALUE;//��Ч���
		if (m_hCompeletionPort != NULL) {
			m_hTread=(HANDLE)_beginthread(CEdoyunQueue<T>::threadEntry,
				0, this);//�Ѵ����õ�iocp�����߳���
		}
	}
	~CEdoyunQueue() {
		if (m_lock)return;
		m_lock = true;
		PostQueuedCompletionStatus(m_hCompeletionPort, 0, NULL, NULL);//����io����
		WaitForSingleObject(m_hTread, INFINITE);
		if (m_hCompeletionPort != NULL) {
			HANDLE hTemp = m_hCompeletionPort;
			m_hCompeletionPort = NULL;
			CloseHandle(hTemp);
		}
	}
	bool PushBack(const T& data) {
		IocpParam* pParam = new IocpParam(EQpush, data);
		if (m_lock) {
			delete pParam;
			return false;
		}
	    bool ret= PostQueuedCompletionStatus(m_hCompeletionPort,sizeof(PPARAM),(ULONG_PTR)pParam,
			NULL);
		if (ret == false) delete pParam;
	//	printf("push back done %d %08p\r\n", ret, (void*)pParam);
		return ret;
	}
	bool PopFront(T& data) { //ȡֵ����
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam pParam(EQpop, data, hEvent);
		if (m_lock) {
			if (hEvent)CloseHandle(hEvent);
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&pParam,
			NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return false;
		}
		ret=WaitForSingleObject(hEvent, INFINITE)==WAIT_OBJECT_0;
		if (ret) {
			data = pParam.Data;
		}
		return ret;
		 
	}
	size_t Size() {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam pParam(EQSize, T(), hEvent);
		if (m_lock) {
			if (hEvent)CloseHandle(hEvent);
			return -1;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&pParam,
			NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return -1;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
		if (ret) {
			return pParam.nOperator;
		}
		return -1;
	}
	bool Clear() {
		if (m_lock)return false;
		IocpParam* pParam = new IocpParam(EQClear, T());
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam,
			NULL);
		if (ret == false) delete pParam;
		//printf("Clear done %d %08p\r\n", ret, (void*)pParam);
		return ret;
	}
private:
	static void threadEntry(void* arg) {
		CEdoyunQueue<T>* thiz = (CEdoyunQueue<T>*)arg;
		thiz->threadMain();
		_endthread();
	}
	void DealParam(PPARAM* pParam) {
		switch (pParam->nOperator) {
		case EQpush:         //������
			m_lstData.push_back(pParam->Data);
			delete pParam;
			//printf("delete %08p\r\n", (void*)pParam);
			break;
		case EQpop:           //������
			if (m_lstData.size() > 0) {
				pParam->Data = m_lstData.front();
				m_lstData.pop_front();
			}
			if (pParam->hEvent != NULL) {
				SetEvent(pParam->hEvent);
			}
			break;
		case EQSize:
			pParam->nOperator = m_lstData.size();
			if (pParam->hEvent != NULL) {
				SetEvent(pParam->hEvent);
			}
			break;
		case EQClear:
			m_lstData.clear();
			delete pParam;
			//printf("delete %08p\r\n" ,(void*)pParam);
			break;
		default:
			OutputDebugStringA("unknown operator\r\n");
			break;
		}
	}
	void threadMain() {
		DWORD dwTransferred = 0;
		PPARAM* pParam = NULL;
		ULONG_PTR CompletionKey = 0;
		OVERLAPPED* pOverlapped = NULL;
		while (GetQueuedCompletionStatus(
			m_hCompeletionPort,
			&dwTransferred,
			&CompletionKey,
			&pOverlapped,
			INFINITE)) {//����PostQueuedCompletionStatus����Ϣ
			if (dwTransferred == 0 || (CompletionKey == NULL)) {
				printf("thread is prepare to exit\r\n");
				break;
			}
			 pParam = (PPARAM*)CompletionKey;
			 DealParam(pParam);
			
		}
		while (GetQueuedCompletionStatus(m_hCompeletionPort,
			&dwTransferred,
			&CompletionKey,
			&pOverlapped,
			0)) {
			if (dwTransferred == 0 || (CompletionKey == NULL)) {
				printf("thread is prepare to exit\r\n");
				continue;
			}
			pParam = (PPARAM*)CompletionKey;
			DealParam(pParam);
		}
		HANDLE hTemp = m_hCompeletionPort;
		m_hCompeletionPort = NULL;
		CloseHandle(hTemp);
	}
private:
	std::list<T> m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hTread;
	std::atomic<bool> m_lock;//������������

};

