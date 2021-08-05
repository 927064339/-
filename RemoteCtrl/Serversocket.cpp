#include "pch.h"
#include "Serversocket.h"
CServersocket* CServersocket::m_instance = NULL;
CServersocket::CHelper CServersocket::m_helper;

CServersocket* pserver = CServersocket::getInstance();