
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
//-연결 요청 받지 못함
//
//closesocket
//- SO_LINGER옵션에 타임아웃
//
//WSAConnect, connect
//- 연결 시도
//
//recv함수들
//- 데이터 아직 못받음 : 다시 요청 해야함
//
//send함수들
//- 전송할 버퍼 공간 없음.나중에 다시 시도.
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
	// return 값 저장 변수 
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
	// 서버 주소 및 포트 확인
	Ret_getaddrinfo = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (Ret_getaddrinfo != 0)
	{
		c_Save_Log.printfLog(L"getaddrinfo failed with error: %d \n", Ret_getaddrinfo);
		WSACleanup();
		return false;
	}

	//---------------------------------------------------
	// Create a SOCKET for the server to listen for client connections.
	// 서버가 클라이언트 연결을 수신을 SOCKET을 만든다.
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
	// TCP listening socket 을 준비한다.
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
	//i_Result = listen(Temp_Listen_Socket, SOMAXCONN_HINT(65535));	// 양수 ~200 -> 기본 200개, 200~ 갯수 적용,    
	if (Ret_listen == SOCKET_ERROR)
	{
		c_Save_Log.printfLog(L"listen failed with error: %d \n", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		return false;
	}
	printf_s("Listen OK # \n");

	//---------------------------------------------------
	// 넌블로킹 소켓으로 전환
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
	// 리슨소켓 넣기
	//------------------------------------------
	FD_SET(g_Listen_Socket, &ReadSet);

	//------------------------------------------
	// 리슨소켓 및 접속중인 모든 클라이언트에 대해 SOCKET 을 체크한다. 
	//------------------------------------------
	for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
	{
		pSession = (*iter).second;

		if (pSession->Disconnect) continue;

		//------------------------------------------
		// 해당 클라이언트 Read Set 등록 / SendQ 에 데이터가 있다면 Write Set 등록
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
	// 리턴값이 0 이상이라면 누군가의 데이터가 왔다.
	//------------------------------------------
	while (i_Result > 0)
	{
		if (FD_ISSET(g_Listen_Socket, &ReadSet))
		{
			netProc_Accept();
			--i_Result;
		}

		//------------------------------------------
		// 전체 세션중 어떤 세션이 반응을 보였는지 다시 확인한다. 
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

	// 접속자 인원수 제한
	// 원할한 플레이를 위하여
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
	// 접속한 클라이언트 정보 출력
	//if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	//{
	//	std::cout << host << " : " << service << "에 연결되었습니다." << std::endl;
	//}
	inet_ntop(AF_INET, &Clinet_Addr.sin_addr, host, NI_MAXHOST);
	printf_s("Conncet # IP:%s : Port: %d / SessionID: %d \n", host, ntohs(Clinet_Addr.sin_port), st_New_Player->dwSessionID);
#endif // DEFAULT_LOG



	//---------------------------------------------------------------------------------
	// 캐릭터 생성 메시지 생성
	// 1. 새로운 플레이어 생성 메시지 보내기.
	// 2. 나를 제외한 모든 플레이어에게 나의 생성 메시지 보내기. 
	// 3. 나에게 다른 플레이어들에 대한 생성 메시지 보내기.
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


	// 3. 나에게 다른 플레이어들에 대한 생성 메시지 보내기.
	for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
	{
		pTempSession = (*iter).second;
		if (pTempSession == st_New_Player) continue;
		if (pTempSession->Disconnect) continue;

		// 일단 그냥 생성 메시지만 보낸다.
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
		// 추후 새로운 참가자가 들어 왔을 때 누군가 움직이고 있었다면 움직이는 동작을 이어서 보여주어야 한다.
		//  
		// 그러려면 현재 위치에서 dwAction과 동일한 패킷 메시지를 보내야 한다.
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
	// Recv버퍼의 모든 메시지를 Recv링버퍼로 가지고 온다. 
	
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
		// 아마도 상대방의 연결 종료
#ifdef DEFAULT_LOG
		printf_s("Disconnect Recv 0 # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
		PushDisconnectList(pSession);
		return;
	}
	else if (Recv_Size == SOCKET_ERROR)
	{
		////////////////////////////////////
		// 여기서 끊지 않아도 아래에서 강제로 끊긴다.
		////////////////////////////////////
		err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			// Selete로 Recv 할 수 있는 상황에서 WSAEWOULDBLOCK은 절대로 뜨면 안된다. 그러니까 중지한다. 
			// 32명 한번에 종료한뒤, 새로운 접속자가 움직인 경우 발생했음.
			c_Save_Log.printfLog(L"Recv failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
			printf_s("WSAEWOULDBLOCK # Nect Frame Recv try %d # SessionID: %d \n", err, pSession->dwSessionID);
			//return;
			__debugbreak();
		}
		else if (err == 10054)
		{
			// 상대방이 강제로 끊었다.
			c_Save_Log.printfLog(L"Recv failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
			printf_s("Disconnect Recv %d # SessionID: %d \n", err, pSession->dwSessionID);
			PushDisconnectList(pSession);
			return;
		}
		else if (err == 10053)
		{
			// 소프트웨어로 인해 연결이 중단됨
			// 데이터 전송 시간 제한 또는 프로토콜 오류
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


	// *완료패킷 처리 부
	// 완료패킷 처리 부분은 Recv()에 들어있는 모든 완성 패킷을 처리 해야 함
	while (1)
	{
		// 기저 사례1. RecvQ에 최소한의 사이즈가 있는지 확인. 조건 - 헤더사이즈 초과
		if (pSession->RecvQ.GetUseSize() < sizeof(st_PACKET_HEADER)) break;
		
		// 헤더를 열어 페이로드 사이즈를 확인한다.
		Ret_Peek = pSession->RecvQ.Peek((char*)&header, sizeof(st_PACKET_HEADER), true);

		// 기저 사례2. Header에서 byCode를 확인한다. 다르면 연결 끊기
		if (header.byCode != dfNETWORK_PACKET_CODE)
		{
#ifdef DEFAULT_LOG
			printf_s("Header code Errer # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
			PushDisconnectList(pSession);
			break;
		}

		// 기저 사례3. (헤더 + 메시지)를 완성하지 못했다면 반환한다. 
		if (pSession->RecvQ.GetUseSize() < sizeof(st_PACKET_HEADER) + header.bySize) break;

		// 기저 사례4. (헤더 + 메시지)크기가 링버퍼의 최대 크기보다 크다면 연결 끊기
		if (pSession->RecvQ.GetBufferSize() < sizeof(st_PACKET_HEADER) + header.bySize)
		{
#ifdef DEFAULT_LOG
			printf_s("Header Size Errer # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
			PushDisconnectList(pSession);
			break;
		}

		//---------------------------------------------
		// 메시지 완성됨 == 메시지를 꺼내서 처리한다. '
		pSession->RecvQ.MoveFront(sizeof(st_PACKET_HEADER));

#ifdef SERIALIZEBUFFER
		Ret_Dq = pSession->RecvQ.Dequeue(Temp_Packet_SerializeBuffer.GetBufferPtr(), header.bySize);
		if (Ret_Dq < header.bySize)
		{
			// 혹시 모를 에러체크, 싱글 스레드에서는 일어날 일이 없다. 
			c_Save_Log.printfLog(L"Dequeue failed with error \n");
			__debugbreak();
		};


		SerializeBuffer_PacketProc(pSession, header.byType, &Temp_Packet_SerializeBuffer);
#else
		Ret_Dq = pSession->RecvQ.Dequeue(Temp_Packet_Buffer, header.bySize);
		if (Ret_Dq < header.bySize)
		{
			// 혹시 모를 에러체크, 싱글 스레드에서는 일어날 일이 없다. 
			c_Save_Log.printfLog(L"Dequeue failed with error \n");
			__debugbreak();
		}
		PacketProc(pSession, header.byType, Temp_Packet_Buffer);
#endif // SERIALIZEBUFFER

	}
}

void netProc_Send(st_SESSION* pSession)
{
	// Send의 경우 한번에 링버퍼에 있는 모든 메시지를 전송하면 된다.
	
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
				// Send 할 때 WSAEWOULDBLOCK이 나온다면, 다음 프레임에 데이터를 보낸다.
				c_Save_Log.printfLog(L"Send failed with error: %ld / SessionID:%d \n", err, pSession->dwSessionID);
				printf_s("WSAEWOULDBLOCK # Nect Frame Send try %d # SessionID: %d \n", err, pSession->dwSessionID);
				break;
			}
			else if (err == 10054)
			{
				// 현재 연결은 원격 호스트에 의해 강제로 끊겼다.
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

			// Send의 WSAEWOULDBLOCK 이 나오면 Send Buffer이 가득 찼다 -> 상대 Recv Buffer도 가득 찼다 == 그냥 연결을 끊으면 된다 
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
		// 모든 삭제 메시지 전송이 끝나고 나면 
		// 저장되어 있는 모든 소켓을 삭제한다. 

		// 1. 소켓을 지운다. 
		// 2. 메모리를 delete로 해지한다. 
		// 3. map 에서 지운다. 
		// 4. g_Disconnect_List에서 Node를 지운다.


		// //뻑난 코드
		// //한번에 많은 close가 들어오면 g_Session_List에서 삭제가 안되는 버그가 있었다.
		//closesocket((*iter)->Socket);
		//// delete (*iter);
		//g_Session_List.erase((*iter)->dwSessionID);
		//iter = g_Disconnect_List.erase(iter);
		//delete (*iter);


		// 정상코드
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



