
#include <stdio.h>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#include "Init_Winsock_Server.h"

#pragma comment(lib, "Ws2_32.lib")


#define DEFAULT_PORT "3000"

//--------------------------------------------------------------------
// ������ �����͸� �����Ѵ�. 
// 
// ���ο��� SOCKET�� �� ��ȯ�Ͽ� ����Ѵ�.
//--------------------------------------------------------------------
bool init_Listen_Socket(void* Listen_Socket, void* wsa_Data)
{
	struct addrinfo hints;
	struct addrinfo* result = NULL;
	SOCKET Temp_Listen_Socket = *(SOCKET*)Listen_Socket;
	u_long on;
	linger Linger_Opt;
	
	//---------------------------------------------------
	// return �� ���� ���� 
	int Ret_WSAStartup;
	int Ret_getaddrinfo;
	int Ret_bind;
	int Ret_listen;
	int Ret_ioctlsocket;
	int Ret_setsockopt;

	//---------------------------------------------------
	// Initialize Winsock
	Ret_WSAStartup = WSAStartup(MAKEWORD(2, 2), (WSADATA*)wsa_Data);
	if (Ret_WSAStartup != 0)
	{
		printf_s("WSAStartup failed with error: %d \n", Ret_WSAStartup);
		return false;
	}

	//---------------------------------------------------
	// SetUp hints 
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//---------------------------------------------------
	// Resolve the server address and port
	// ���� �ּ� �� ��Ʈ Ȯ��
	Ret_getaddrinfo = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (Ret_getaddrinfo != 0)
	{
		printf_s("getaddrinfo failed with error: %d \n", Ret_getaddrinfo);
		WSACleanup();
		return false;
	}

	//---------------------------------------------------
	// Create a SOCKET for the server to listen for client connections.
	// ������ Ŭ���̾�Ʈ ������ ������ SOCKET�� �����.
	Temp_Listen_Socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (Temp_Listen_Socket == INVALID_SOCKET)
	{
		printf_s("socket failed with error : %ld \n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	//---------------------------------------------------
	// setsockopt_Linger
	Linger_Opt.l_onoff = 1;
	Linger_Opt.l_linger = 0;
	Ret_setsockopt = setsockopt(Temp_Listen_Socket, SOL_SOCKET, SO_LINGER, (char*)&Linger_Opt, sizeof(Linger_Opt));
	if (Ret_setsockopt == SOCKET_ERROR)
	{
		printf_s("setsockopt failed with error: %ld \n", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		return false;
	}



	//---------------------------------------------------
	// Setup the TCP listening socket
	// TCP listening socket �� �غ��Ѵ�.
	Ret_bind = bind(Temp_Listen_Socket, result->ai_addr, (int)result->ai_addrlen);
	if (Ret_bind == SOCKET_ERROR)
	{
		printf_s("bind failed with error: %d \n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		return false;
	}

	freeaddrinfo(result);

	//---------------------------------------------------
	// Listen a client socket
	Ret_listen = listen(Temp_Listen_Socket, SOMAXCONN);
	//i_Result = listen(Temp_Listen_Socket, SOMAXCONN_HINT(65535));	// ��� ~200 -> �⺻ 200��, 200~ ���� ����,    
	if (Ret_listen == SOCKET_ERROR)
	{
		printf_s("listen failed with error: %d \n", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		return false;
	}

	//---------------------------------------------------
	// �ͺ��ŷ �������� ��ȯ
	on = 1;
	Ret_ioctlsocket = ioctlsocket(Temp_Listen_Socket, FIONBIO, &on);
	if (Ret_ioctlsocket == SOCKET_ERROR)
	{
		printf_s("ioctlsocket failed with error: %ld \n", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		return false;
	}

	*(SOCKET*)Listen_Socket = Temp_Listen_Socket;

	return true;
}