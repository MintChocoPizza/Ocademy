
#include <map>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

#include "Set_SerializeBuffer.h"
#include "Set_Log.h"
#include "CList.h"
#include "c_Save_Log.h"
#include "C_Ring_Buffer.h"
#include "Logic.h"
#include "SerializeBuffer.h"
#include "NetWork.h"
#include "PacketDefine.h"
#include "main.h"




/////////////////////////////////////////////////////////////////////
// 
// WSAAccept, accept
//-���� ��û ���� ����
//
//closesocket
//- SO_LINGER�ɼǿ� Ÿ�Ӿƿ�
//
//WSAConnect, connect
//- ���� �õ�
//
//recv�Լ���
//- ������ ���� ������ : �ٽ� ��û �ؾ���
//
//send�Լ���
//- ������ ���� ���� ����.���߿� �ٽ� �õ�.
// 
/////////////////////////////////////////////////////////////////////

const char* DEFAULT_PORT = "5000";

std::map<DWORD, st_SESSION*> g_Session_List;
CList<st_SESSION*> g_Disconnect_List;

DWORD g_Session_ID;


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
		c_Save_Log.printfLog(L"WSAStartup failed with error: %d \n", Ret_WSAStartup);
		return false;
	}
	printf_s("WSAStartup # \n");

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
		c_Save_Log.printfLog(L"getaddrinfo failed with error: %d \n", Ret_getaddrinfo);
		WSACleanup();
		return false;
	}

	//---------------------------------------------------
	// Create a SOCKET for the server to listen for client connections.
	// ������ Ŭ���̾�Ʈ ������ ������ SOCKET�� �����.
	Temp_Listen_Socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (Temp_Listen_Socket == INVALID_SOCKET)
	{
		c_Save_Log.printfLog(L"socket failed with error : %ld \n", WSAGetLastError());
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
		c_Save_Log.printfLog(L"setsockopt failed with error: %ld \n", WSAGetLastError());
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
		c_Save_Log.printfLog(L"bind failed with error: %d \n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		return false;
	}
	printf_s("Bind OK # Port: %s \n", DEFAULT_PORT);

	freeaddrinfo(result);

	//---------------------------------------------------
	// Listen a client socket
	Ret_listen = listen(Temp_Listen_Socket, SOMAXCONN);
	//i_Result = listen(Temp_Listen_Socket, SOMAXCONN_HINT(65535));	// ��� ~200 -> �⺻ 200��, 200~ ���� ����,    
	if (Ret_listen == SOCKET_ERROR)
	{
		c_Save_Log.printfLog(L"listen failed with error: %d \n", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		return false;
	}
	printf_s("Listen OK # \n");

	//---------------------------------------------------
	// �ͺ��ŷ �������� ��ȯ
	on = 1;
	Ret_ioctlsocket = ioctlsocket(Temp_Listen_Socket, FIONBIO, &on);
	if (Ret_ioctlsocket == SOCKET_ERROR)
	{
		c_Save_Log.printfLog(L"ioctlsocket failed with error: %ld \n", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		return false;
	}

	*(SOCKET*)Listen_Socket = Temp_Listen_Socket;

	return true;
}

void netIOProcess(void)
{
	int i_Result;

	std::map<DWORD, st_SESSION*>::iterator iter;

	st_SESSION* pSession;

	FD_SET ReadSet;
	FD_SET WriteSet;

	timeval Time;


	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);

	//------------------------------------------
	// �������� �ֱ�
	//------------------------------------------
	FD_SET(g_Listen_Socket, &ReadSet);

	//------------------------------------------
	// �������� �� �������� ��� Ŭ���̾�Ʈ�� ���� SOCKET �� üũ�Ѵ�. 
	//------------------------------------------
	for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
	{
		pSession = (*iter).second;

		if (pSession->Disconnect) continue;

		//------------------------------------------
		// �ش� Ŭ���̾�Ʈ Read Set ��� / SendQ �� �����Ͱ� �ִٸ� Write Set ���
		//------------------------------------------
		FD_SET(pSession->Socket, &ReadSet);
		if (pSession->SendQ.GetUseSize() > 0)
			FD_SET(pSession->Socket, &WriteSet);
	}

	Time.tv_sec = 0;
	Time.tv_usec = 0;

	i_Result = select(0, &ReadSet, &WriteSet, 0, &Time);
	if (i_Result == SOCKET_ERROR)
	{
		c_Save_Log.printfLog(L"select failed with error: %ld \n", WSAGetLastError());
		__debugbreak();
	}
	//------------------------------------------
	// ���ϰ��� 0 �̻��̶�� �������� �����Ͱ� �Դ�.
	//------------------------------------------
	while (i_Result > 0)
	{
		if (FD_ISSET(g_Listen_Socket, &ReadSet))
		{
			netProc_Accept();
			--i_Result;
		}

		//------------------------------------------
		// ��ü ������ � ������ ������ �������� �ٽ� Ȯ���Ѵ�. 
		//------------------------------------------
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{
			pSession = (*iter).second;
			if (FD_ISSET(pSession->Socket, &ReadSet))
			{
				netProc_Recv(pSession);
				--i_Result;
			}

			if (FD_ISSET(pSession->Socket, &WriteSet))
			{
				netProc_Send(pSession);
				--i_Result;
			}
		}
	}

	Disconnect();
}

void netProc_Accept(void)
{
#ifdef DEFAULT_LOG
	char host[NI_MAXHOST];
#endif // DEFAULT_LOG

	std::map<DWORD, st_SESSION*>::iterator iter;

	SOCKET Client_Socket;

	st_SESSION* st_New_Player;
	st_SESSION* pTempSession;
	sockaddr_in Clinet_Addr;
	int Client_Addr_Len = sizeof(Clinet_Addr);
	int Error;

	st_PACKET_HEADER header_SC_CRESTE_MY_CHARACTER;
	st_PACKET_HEADER header_SC_CREATE_OTHER_CHARACTER;
	st_PACKET_HEADER header_for_me_SC_CREATE_OTHER_CHARACTER;
	st_PACKET_HEADER header_Sync_Move_SC_MOVE_START;

	st_PACKET_SC_CREATE_MY_CHARACTER		packet_SC_CRESTE_MY_CHARACTER;
	st_PACKET_SC_CREATE_OTHER_CHARACTER		packet_SC_CREATE_OTHER_CHARACTER;
	st_PACKET_SC_CREATE_OTHER_CHARACTER		packet_for_me_SC_CREATE_OTHER_CHARACTER;
	st_PACKET_SC_MOVE_START					packet_Sync_Move_SC_MOVE_START;

	Client_Socket = accept(g_Listen_Socket, (sockaddr*)&Clinet_Addr, &Client_Addr_Len);
	if (Client_Socket == INVALID_SOCKET)
	{
		Error = WSAGetLastError();
		if (Error != WSAEWOULDBLOCK)
		{
			c_Save_Log.printfLog(L"ioctlsocket failed with error: %ld \n", Error);
			DebugBreak();
			__debugbreak();
		}
	}

	// ������ �ο��� ����
	// ������ �÷��̸� ���Ͽ�
	if (g_Session_List.size() > 30)
	{
#ifdef DEFAULT_LOG
		printf_s("full Server Cannt not accept!!! \n");
#endif // DEFAULT_LOG
		closesocket(Client_Socket);
		return;
	}

	st_New_Player = new st_SESSION;
	init_Session(Client_Socket, st_New_Player);
	// g_Session_List.push_back(st_New_Player);
	g_Session_List.insert(std::pair<DWORD, st_SESSION*>(st_New_Player->dwSessionID, st_New_Player));

#ifdef DEFAULT_LOG
	// ������ Ŭ���̾�Ʈ ���� ���
	//if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	//{
	//	std::cout << host << " : " << service << "�� ����Ǿ����ϴ�." << std::endl;
	//}
	inet_ntop(AF_INET, &Clinet_Addr.sin_addr, host, NI_MAXHOST);
	printf_s("Conncet # IP:%s : Port: %d / SessionID: %d \n", host, ntohs(Clinet_Addr.sin_port), st_New_Player->dwSessionID);
#endif // DEFAULT_LOG



	//---------------------------------------------------------------------------------
	// ĳ���� ���� �޽��� ����
	// 1. ���ο� �÷��̾� ���� �޽��� ������.
	// 2. ���� ������ ��� �÷��̾�� ���� ���� �޽��� ������. 
	// 3. ������ �ٸ� �÷��̾�鿡 ���� ���� �޽��� ������.
	//---------------------------------------------------------------------------------
#ifdef DEFAULT_LOG
	printf_s("# PACKET_CONNECT # SessionID: %d \n", st_New_Player->dwSessionID);
#endif // DEFAULT_LOG
#ifdef SERIALIZEBUFFER
	SerializeBuffer clPacket;
	SerializeBuffer_netPacketProc_SC_CREATE_MY_CHARACTER(st_New_Player, &clPacket);
	SerializeBuffer_netSendUnicast(st_New_Player, &clPacket);
#else
	netPacketProc_SC_CREATE_MY_CHARACTER(st_New_Player, (char*)&header_SC_CRESTE_MY_CHARACTER, (char*)&packet_SC_CRESTE_MY_CHARACTER);
	netSendUnicast(st_New_Player, (char*)&header_SC_CRESTE_MY_CHARACTER, (char*)&packet_SC_CRESTE_MY_CHARACTER, sizeof(packet_SC_CRESTE_MY_CHARACTER));
#endif	// SERIALIZEBUFFER


#ifdef DEFAULT_LOG
	printf_s("Create Character SessionID: %d    X:%d    Y:%d \n", st_New_Player->dwSessionID, st_New_Player->shX, st_New_Player->shY);
#endif // DEFAULT_LOG

#ifdef SERIALIZEBUFFER
	SerializeBuffer clPacket2;
	SerializeBuffer_netPacketProc_SC_CREATE_OTHER_CHARACTER(st_New_Player, &clPacket2);
	SerializeBuffer_netSendBroadcast(st_New_Player, &clPacket2);
#else
	netPacketProc_SC_CREATE_OTHER_CHARACTER(st_New_Player, (char*)&header_SC_CREATE_OTHER_CHARACTER, (char*)&packet_SC_CREATE_OTHER_CHARACTER);
	netSendBroadcast(st_New_Player, (char*)&header_SC_CREATE_OTHER_CHARACTER, (char*)&packet_SC_CREATE_OTHER_CHARACTER, sizeof(packet_SC_CREATE_OTHER_CHARACTER));
#endif // SERIALIZEBUFFER

#ifdef DETAILS_LOG
	printf_s("Create Character SessionID : %d to Other \n", st_New_Player->dwSessionID);
#endif // DETAILS_LOG


	// 3. ������ �ٸ� �÷��̾�鿡 ���� ���� �޽��� ������.
	for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
	{
		pTempSession = (*iter).second;
		if (pTempSession == st_New_Player) continue;
		if (pTempSession->Disconnect) continue;

		// �ϴ� �׳� ���� �޽����� ������.
#ifdef SERIALIZEBUFFER
		SerializeBuffer clPacket3;
		clPacket3.Clear();
		SerializeBuffer_netPacketProc_SC_CREATE_OTHER_CHARACTER(pTempSession, &clPacket3);
		SerializeBuffer_netSendUnicast(st_New_Player, &clPacket3);
#else
		netPacketProc_SC_CREATE_OTHER_CHARACTER(pTempSession, (char*)&header_for_me_SC_CREATE_OTHER_CHARACTER, (char*)&packet_for_me_SC_CREATE_OTHER_CHARACTER);
		netSendUnicast(st_New_Player, (char*)&header_for_me_SC_CREATE_OTHER_CHARACTER, (char*)&packet_for_me_SC_CREATE_OTHER_CHARACTER, sizeof(packet_for_me_SC_CREATE_OTHER_CHARACTER));
#endif // SERIALIZEBUFFER



		///////////////////////////////
		// ���� ���ο� �����ڰ� ��� ���� �� ������ �����̰� �־��ٸ� �����̴� ������ �̾ �����־�� �Ѵ�.
		//  
		// �׷����� ���� ��ġ���� dwAction�� ������ ��Ŷ �޽����� ������ �Ѵ�.
		///////////////////////////////
		if (pTempSession->dwAction != dfPACKET_CS_MOVE_STOP)
		{
#ifdef SERIALIZEBUFFER
			SerializeBuffer clPacket4;
			SerializeBuffer_netPacketProc_SC_MOVE_START(&clPacket4, (char)pTempSession->dwAction, pTempSession->dwSessionID, pTempSession->shX, pTempSession->shY);
			SerializeBuffer_netSendUnicast(st_New_Player, &clPacket4);
#else
			netPacketProc_SC_MOVE_START((char*)&header_Sync_Move_SC_MOVE_START, (char*)&packet_Sync_Move_SC_MOVE_START, (char)pTempSession->dwAction, pTempSession->dwSessionID, pTempSession->shX, pTempSession->shY);
			netSendUnicast(st_New_Player, (char*)&header_Sync_Move_SC_MOVE_START, (char*)&packet_Sync_Move_SC_MOVE_START, sizeof(packet_Sync_Move_SC_MOVE_START));
#endif // SERIALIZEBUFFER

		}
	}
#ifdef DETAILS_LOG
	printf_s("Create Character SessionID : %d from Other \n", st_New_Player->dwSessionID);
#endif // DETAILS_LOG
}

void init_Session(SOCKET Client_Socket, st_SESSION* st_New_Player)
{
	st_New_Player->Socket = Client_Socket;
	st_New_Player->chHP = 100;
	st_New_Player->byDirection = dfPACKET_MOVE_DIR_LL;
	st_New_Player->dwSessionID = ++g_Session_ID;
	st_New_Player->dwAction = dfPACKET_CS_MOVE_STOP;
	st_New_Player->shY = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP + 1) + dfRANGE_MOVE_TOP;
	st_New_Player->shX = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT + 1) + dfRANGE_MOVE_LEFT;
	st_New_Player->Disconnect = false;
}

void netProc_Recv(st_SESSION* pSession)
{
	// Recv������ ��� �޽����� Recv�����۷� ������ �´�. 
	
	int Recv_Size;
	int err;
	int Ret_Peek;
	int Ret_Dq;

	st_PACKET_HEADER header;
	char Temp_Packet_Buffer[64];
	SerializeBuffer Temp_Packet_SerializeBuffer;

	Recv_Size = recv(pSession->Socket, pSession->RecvQ.GetRearBufferPtr(), pSession->RecvQ.DirectEnqueueSize(), 0);
	if (Recv_Size == 0)
	{
		// �Ƹ��� ������ ���� ����
#ifdef DEFAULT_LOG
		printf_s("Disconnect Recv 0 # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
		PushDisconnectList(pSession);
		return;
	}
	else if (Recv_Size == SOCKET_ERROR)
	{
		////////////////////////////////////
		// ���⼭ ���� �ʾƵ� �Ʒ����� ������ �����.
		////////////////////////////////////
		err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			// Selete�� Recv �� �� �ִ� ��Ȳ���� WSAEWOULDBLOCK�� ����� �߸� �ȵȴ�. �׷��ϱ� �����Ѵ�. 
			// 32�� �ѹ��� �����ѵ�, ���ο� �����ڰ� ������ ��� �߻�����.
			c_Save_Log.printfLog(L"Recv failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
			printf_s("WSAEWOULDBLOCK # Nect Frame Recv try %d # SessionID: %d \n", err, pSession->dwSessionID);
			//return;
			__debugbreak();
		}
		else if (err == 10054)
		{
			// ������ ������ ������.
			c_Save_Log.printfLog(L"Recv failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
			printf_s("Disconnect Recv %d # SessionID: %d \n", err, pSession->dwSessionID);
			PushDisconnectList(pSession);
			return;
		}
		else if (err == 10053)
		{
			// ����Ʈ����� ���� ������ �ߴܵ�
			// ������ ���� �ð� ���� �Ǵ� �������� ����
			c_Save_Log.printfLog(L"Recv failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
			printf_s("Disconnect Recv %d # SessionID: %d \n", err, pSession->dwSessionID);
			PushDisconnectList(pSession);
			return;
		}
		else
		{
			c_Save_Log.printfLog(L"Recv failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
			printf_s("Disconnect Recv %d # SessionID: %d \n", err, pSession->dwSessionID);
			__debugbreak();
		}
	}
	pSession->RecvQ.MoveRear(Recv_Size);


	// *�Ϸ���Ŷ ó�� ��
	// �Ϸ���Ŷ ó�� �κ��� Recv()�� ����ִ� ��� �ϼ� ��Ŷ�� ó�� �ؾ� ��
	while (1)
	{
		// ���� ���1. RecvQ�� �ּ����� ����� �ִ��� Ȯ��. ���� - ��������� �ʰ�
		if (pSession->RecvQ.GetUseSize() < sizeof(st_PACKET_HEADER)) break;
		
		// ����� ���� ���̷ε� ����� Ȯ���Ѵ�.
		Ret_Peek = pSession->RecvQ.Peek((char*)&header, sizeof(st_PACKET_HEADER), true);

		// ���� ���2. Header���� byCode�� Ȯ���Ѵ�. �ٸ��� ���� ����
		if (header.byCode != dfNETWORK_PACKET_CODE)
		{
#ifdef DEFAULT_LOG
			printf_s("Header code Errer # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
			PushDisconnectList(pSession);
			break;
		}

		// ���� ���3. (��� + �޽���)�� �ϼ����� ���ߴٸ� ��ȯ�Ѵ�. 
		if (pSession->RecvQ.GetUseSize() < sizeof(st_PACKET_HEADER) + header.bySize) break;

		// ���� ���4. (��� + �޽���)ũ�Ⱑ �������� �ִ� ũ�⺸�� ũ�ٸ� ���� ����
		if (pSession->RecvQ.GetBufferSize() < sizeof(st_PACKET_HEADER) + header.bySize)
		{
#ifdef DEFAULT_LOG
			printf_s("Header Size Errer # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
			PushDisconnectList(pSession);
			break;
		}

		//---------------------------------------------
		// �޽��� �ϼ��� == �޽����� ������ ó���Ѵ�. '
		pSession->RecvQ.MoveFront(sizeof(st_PACKET_HEADER));

#ifdef SERIALIZEBUFFER
		Ret_Dq = pSession->RecvQ.Dequeue(Temp_Packet_SerializeBuffer.GetBufferPtr(), header.bySize);
		if (Ret_Dq < header.bySize)
		{
			// Ȥ�� �� ����üũ, �̱� �����忡���� �Ͼ ���� ����. 
			c_Save_Log.printfLog(L"Dequeue failed with error \n");
			__debugbreak();
		};


		SerializeBuffer_PacketProc(pSession, header.byType, &Temp_Packet_SerializeBuffer);
#else
		Ret_Dq = pSession->RecvQ.Dequeue(Temp_Packet_Buffer, header.bySize);
		if (Ret_Dq < header.bySize)
		{
			// Ȥ�� �� ����üũ, �̱� �����忡���� �Ͼ ���� ����. 
			c_Save_Log.printfLog(L"Dequeue failed with error \n");
			__debugbreak();
		}
		PacketProc(pSession, header.byType, Temp_Packet_Buffer);
#endif // SERIALIZEBUFFER

	}
}

void netProc_Send(st_SESSION* pSession)
{
	// Send�� ��� �ѹ��� �����ۿ� �ִ� ��� �޽����� �����ϸ� �ȴ�.
	
	int Send_Size;
	int err;

	while (1)
	{
		if (pSession->SendQ.GetUseSize() == 0)
			break;

		Send_Size = send(pSession->Socket, pSession->SendQ.GetFrontBufferPtr(), pSession->SendQ.DirectDequeueSize(), 0);

		if (Send_Size == SOCKET_ERROR)
		{
			err = WSAGetLastError();
			
			if (err == WSAEWOULDBLOCK)
			{
				// Send �� �� WSAEWOULDBLOCK�� ���´ٸ�, ���� �����ӿ� �����͸� ������.
				c_Save_Log.printfLog(L"Send failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
				printf_s("WSAEWOULDBLOCK # Nect Frame Send try %d # SessionID: %d \n", err, pSession->dwSessionID);
				break;
			}
			else if (err == 10054)
			{
				// ���� ������ ���� ȣ��Ʈ�� ���� ������ �����.
				c_Save_Log.printfLog(L"Send failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
				PushDisconnectList(pSession);
				break;
			}
			else if (err == 10053)
			{
				c_Save_Log.printfLog(L"Send failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
				PushDisconnectList(pSession);
				break;
			}

			// Send�� WSAEWOULDBLOCK �� ������ Send Buffer�� ���� á�� -> ��� Recv Buffer�� ���� á�� == �׳� ������ ������ �ȴ� 
			c_Save_Log.printfLog(L"Send failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
			__debugbreak();
		}
		pSession->SendQ.MoveFront(Send_Size);
	}
}

#ifdef SERIALIZEBUFFER
void SerializeBuffer_netSendUnicast(st_SESSION* pSession, SerializeBuffer* clpPacket)
{
	int Ret_Packet;

	Ret_Packet = pSession->SendQ.Enqueue(clpPacket->GetBufferPtr(), clpPacket->GetDataSize());
	if (Ret_Packet == 0)
	{
		c_Save_Log.printfLog(L"Packet Unicast failed with error: \n");
#ifdef DEFAULT_LOG
		printf_s("Disconnect Packet Unicast failed with error # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
		PushDisconnectList(pSession);
		return;
	}
}
void SerializeBuffer_netSendBroadcast(st_SESSION* pSession, SerializeBuffer* clpPacket)
{
	std::map<DWORD, st_SESSION*>::iterator iter;
	st_SESSION* p_Temp_Session;

	for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
	{
		p_Temp_Session = (*iter).second;

		if (p_Temp_Session->Disconnect) continue;
		if (p_Temp_Session == pSession) continue;

		SerializeBuffer_netSendUnicast(p_Temp_Session, clpPacket);
	}
}
#else
void netSendUnicast(st_SESSION* pSession, char* header, char* packet, int Packet_Len)
{
	int Ret_Header;
	int Ret_Packet;

	Ret_Header = pSession->SendQ.Enqueue(header, sizeof(st_PACKET_HEADER));
	if (Ret_Header == 0)
	{
		c_Save_Log.printfLog(L"Header Unicast failed with error: \n" );
#ifdef DEFAULT_LOG
		printf_s("Disconnect Header Unicast failed with error # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
		PushDisconnectList(pSession);
		return ;
	}

	Ret_Packet = pSession->SendQ.Enqueue(packet, Packet_Len);
	if(Ret_Packet == 0)
	{
		c_Save_Log.printfLog(L"Packet Unicast failed with error: \n");
#ifdef DEFAULT_LOG
		printf_s("Disconnect Packet Unicast failed with error # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
		PushDisconnectList(pSession);
		return;
	}
}
void netSendBroadcast(st_SESSION* pSession, char* header, char* packet, int Packet_Len)
{
	// CList<st_SESSION*>::iterator iter;
	std::map<DWORD, st_SESSION*>::iterator iter;
	st_SESSION* p_Temp_Session;

	for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
	{
		p_Temp_Session = (*iter).second;
		
		if (p_Temp_Session->Disconnect) continue;
		if (p_Temp_Session == pSession) continue;

		netSendUnicast(p_Temp_Session, header, packet, Packet_Len);
	}
}
#endif // SERIALIZEBUFFER
void PushDisconnectList(st_SESSION* pSession)
{
	if (pSession->Disconnect) return;
	pSession->Disconnect = true;
	g_Disconnect_List.push_back(pSession);
}

void Disconnect()
{
	CList<st_SESSION*>::iterator iter;
	st_SESSION* p_Session;
	st_PACKET_SC_DELETE_CHARACTER packet_SC_DELETE;
	st_PACKET_HEADER header_SC_DELETE;
	SerializeBuffer clPacket;

#ifdef SERIALIZEBUFFER
	for (iter = g_Disconnect_List.begin(); iter != g_Disconnect_List.end(); ++iter)
	{
		p_Session = *iter;
		SerializeBuffer_netPacketProc_SC_DELETE_CHARACTER(p_Session, &clPacket);
		SerializeBuffer_netSendBroadcast(NULL, &clPacket);
	}
#else
	for (iter = g_Disconnect_List.begin(); iter != g_Disconnect_List.end(); ++iter)
	{
		p_Session = *iter;
		netPacketProc_SC_DELETE_CHARACTER(p_Session, (char*)&header_SC_DELETE, (char*)&packet_SC_DELETE);
		netSendBroadcast(NULL, (char*)&header_SC_DELETE, (char*)&packet_SC_DELETE, sizeof(st_PACKET_SC_DELETE_CHARACTER));
	}
#endif // SERIALIZEBUFFER


	

	for (iter = g_Disconnect_List.begin(); iter != g_Disconnect_List.end();)
	{
		// ��� ���� �޽��� ������ ������ ���� 
		// ����Ǿ� �ִ� ��� ������ �����Ѵ�. 

		// 1. ������ �����. 
		// 2. �޸𸮸� delete�� �����Ѵ�. 
		// 3. map ���� �����. 
		// 4. g_Disconnect_List���� Node�� �����.


		// //���� �ڵ�
		// //�ѹ��� ���� close�� ������ g_Session_List���� ������ �ȵǴ� ���װ� �־���.
		//closesocket((*iter)->Socket);
		//// delete (*iter);
		//g_Session_List.erase((*iter)->dwSessionID);
		//iter = g_Disconnect_List.erase(iter);
		//delete (*iter);


		// �����ڵ�
		st_SESSION* Session = *iter;
		SOCKET Sock = (*iter)->Socket;
		DWORD SessionID = (*iter)->dwSessionID;

		iter = g_Disconnect_List.erase(iter);
		g_Session_List.erase(SessionID);
		closesocket(Sock);
		delete Session;
	}
	
}

#ifdef SERIALIZEBUFFER
bool SerializeBuffer_PacketProc(st_SESSION* pSession, BYTE byPacketType, SerializeBuffer* clpPacket)
{
	switch (byPacketType)
	{
	case dfPACKET_CS_MOVE_START:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_MOVE_START # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_MOVE_START*)clpPacket->GetBufferPtr())->Direction, ((st_PACKET_CS_MOVE_START*)clpPacket->GetBufferPtr())->X, ((st_PACKET_CS_MOVE_START*)clpPacket->GetBufferPtr())->Y);
#endif // DEFAULT_LOG
		return SerializeBuffer_netPacketProc_CS_MOVE_START(pSession, clpPacket);
		break;
	}
	case dfPACKET_CS_MOVE_STOP:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_MOVE_STOP # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_MOVE_STOP*)clpPacket->GetBufferPtr())->Direction, ((st_PACKET_CS_MOVE_STOP*)clpPacket->GetBufferPtr())->X, ((st_PACKET_CS_MOVE_STOP*)clpPacket->GetBufferPtr())->Y);
#endif // DEFAULT_LOG
		return SerializeBuffer_netPacketProc_CS_MOVE_STOP(pSession, clpPacket);
		break;
	}
	case dfPACKET_CS_ATTACK1:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_ATTACK1 # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_ATTACK1*)clpPacket->GetBufferPtr())->Direction, ((st_PACKET_CS_ATTACK1*)clpPacket->GetBufferPtr())->X, ((st_PACKET_CS_ATTACK1*)clpPacket->GetBufferPtr())->Y);
#endif // DEFAULT_LOG
		return SerializeBuffer_netPacketProc_CS_ATTACK1(pSession, clpPacket);
		break;
	}
	case dfPACKET_CS_ATTACK2:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_ATTACK2 # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_ATTACK2*)clpPacket->GetBufferPtr())->Direction, ((st_PACKET_CS_ATTACK2*)clpPacket->GetBufferPtr())->X, ((st_PACKET_CS_ATTACK2*)clpPacket->GetBufferPtr())->Y);
#endif // DEFAULT_LOG
		return SerializeBuffer_netPacketProc_CS_ATTACK2(pSession, clpPacket);
		break;
	}
	case dfPACKET_CS_ATTACK3:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_ATTACK3 # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_ATTACK3*)clpPacket->GetBufferPtr())->Direction, ((st_PACKET_CS_ATTACK3*)clpPacket->GetBufferPtr())->X, ((st_PACKET_CS_ATTACK3*)clpPacket->GetBufferPtr())->Y);
#endif // DEFAULT_LOG
		return SerializeBuffer_netPacketProc_CS_ATTACK3(pSession, clpPacket);
		break;
	}
	case dfPACKET_CS_SYNC:
	{
		return false;
		break;
	}
	default:
	{
		return false;
		break;
	}
	}

}
#else
bool PacketProc(st_SESSION* pSession, BYTE byPacketType, char* pPacket)
{
	switch (byPacketType)
	{
	case dfPACKET_CS_MOVE_START:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_MOVE_START # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_MOVE_START*)pPacket)->Direction, ((st_PACKET_CS_MOVE_START*)pPacket)->X, ((st_PACKET_CS_MOVE_START*)pPacket)->Y);
#endif // DEFAULT_LOG
		return netPacketProc_CS_MOVE_START(pSession, pPacket);
		break;
	}
	case dfPACKET_CS_MOVE_STOP:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_MOVE_STOP # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_MOVE_STOP*)pPacket)->Direction, ((st_PACKET_CS_MOVE_STOP*)pPacket)->X, ((st_PACKET_CS_MOVE_STOP*)pPacket)->Y);
#endif // DEFAULT_LOG
		return netPacketProc_CS_MOVE_STOP(pSession, pPacket);
		break;
	}
	case dfPACKET_CS_ATTACK1:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_ATTACK1 # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_ATTACK1*)pPacket)->Direction, ((st_PACKET_CS_ATTACK1*)pPacket)->X, ((st_PACKET_CS_ATTACK1*)pPacket)->Y);
#endif // DEFAULT_LOG
		return netPacketProc_CS_ATTACK1(pSession, pPacket);
		break;
	}
	case dfPACKET_CS_ATTACK2:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_ATTACK2 # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_ATTACK2*)pPacket)->Direction, ((st_PACKET_CS_ATTACK2*)pPacket)->X, ((st_PACKET_CS_ATTACK2*)pPacket)->Y);
#endif // DEFAULT_LOG
		return netPacketProc_CS_ATTACK2(pSession, pPacket);
		break;
	}
	case dfPACKET_CS_ATTACK3:
	{
#ifdef DEFAULT_LOG
		printf_s("# PACKET_ATTACK3 # SessionID:%d / Direction:%d / X:%d /Y:%d \n", pSession->dwSessionID, ((st_PACKET_CS_ATTACK3*)pPacket)->Direction, ((st_PACKET_CS_ATTACK3*)pPacket)->X, ((st_PACKET_CS_ATTACK3*)pPacket)->Y);
#endif // DEFAULT_LOG
		return netPacketProc_CS_ATTACK3(pSession, pPacket);
		break;
	}
	case dfPACKET_CS_SYNC:
	{
		return false;
		break;
	}
	default:
	{
		return false;
		break;
	}
	}

}
#endif // SERIALIZEBUFFER



