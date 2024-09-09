
#include <stdio.h>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#include "c_Save_Log.h"

#include "CSessionManager.h"

#pragma comment (lib, "Ws2_32.lib")

C_SessionManager C_SessionManager::_C_SessionManager;

C_SessionManager::C_SessionManager()
{
	int i_Result;
	wchar_t Log[256];

	//---------------------------------------------------
	// initialize Winsock
	i_Result = WSAStartup(MAKEWORD(2, 2), &wsa_Data);
	if(i_Result != 0)
	{
		printf_s("WSAStartup failed with error: %d \n", i_Result);
		swprintf(Log, L"WSAStartup failed with error: %d \n", i_Result);
		c_Save_Log.saveLog(Log);
		__debugbreak();
	}
}

C_SessionManager::~C_SessionManager()
{
}

C_SessionManager* C_SessionManager::GetInstance(void)
{
	return nullptr;
}

void C_SessionManager::SessionUpdate(void)
{
}
