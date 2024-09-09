

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#include <queue>
#include <unordered_map>
#include <new>
#include <cmath>

#include <string>
#include <stdio.h>

#include "Cdebug.h"
#include "C_Logger.h"
#include "LOG.h"
#include "Protocol.h"
#include "C_Ring_Buffer.h"
#include "SerializeBuffer.h"
#include "Disconnect.h"
#include "main.h"
#include "Protocol.h"
#include "Session.h"
#include "CMemoryPool.h"
#include "Define.h"
#include "Player.h"
#include "CList.h"
#include "Field.h"


constexpr int cstMAX_SESSION = dfMAX_SESSION;
// constexpr int dfNETWORK_PORT = 20000;
constexpr int dfNETWORK_PORT = 10901;

// std::unordered_map< DWORD, st_SESSION*> g_Session_Hash;
st_SESSION* g_Session_Arr[dfMAX_SESSION];

//----------------------------------------------------
// 디버깅
bool g_LoopCheck[dfMAX_SESSION][2];

WSADATA g_WsaData;
SOCKET g_Listen_Socket;
std::queue<DWORD> g_SessionID_Q;

OreoPizza::CMemoryPool<st_SESSION> st_SESSION_MemPool(dfMAX_SESSION, FALSE);

DWORD g_dwPreTime;



#pragma warning(disable : 26495)
st_SESSION::st_SESSION()
{
	RecvQ = new C_RING_BUFFER(50000);
	SendQ = new C_RING_BUFFER(50000);
}
st_SESSION::~st_SESSION()
{
	delete RecvQ;
	delete SendQ;
}
#pragma warning(default: 26495)

//--------------------------------------------------------------
// 따로 초기화를 한다. 
// 메모리풀을 사용하기 때문에
// 
//st_SESSION::st_SESSION(SOCKET New_Socket, DWORD dw_New_SessionID)
//{
//	Socket = New_Socket;
//	dwSessionID = dw_New_SessionID;
//
//	RecvQ = new C_RING_BUFFER();
//	SendQ = new C_RING_BUFFER();
//
//	dwLastRecvTime = g_Start_Time;
//
//	Disconnect = false;
//}
//--------------------------------------------------------------
void st_SESSION::Init(SOCKET New_Socket, DWORD dw_New_SessionID)
{
	Socket = New_Socket;
	dwSessionID = dw_New_SessionID;

	RecvQ->ClearBuffer();
	SendQ->ClearBuffer();

	dwLastRecvTime = timeGetTime();
	dwCreateTime = dwLastRecvTime;
	dwLastFD_ISSETTime = 0;
	dwLastSelectSetTime = 0;

	//Disconnect = false;
}

st_SESSION* FindSession(DWORD dwSessionID)
{
	return g_Session_Arr[dwSessionID];
}

void SetSession(void)
{
	int iCnt;

	ZeroMemory(g_Session_Arr, cstMAX_SESSION);

	for (iCnt = 1; iCnt < cstMAX_SESSION; ++iCnt)
	{
		g_SessionID_Q.push(iCnt);
	}
}

void DeleteSession(DWORD dwSessionID)
{
	st_SESSION* pSession;

	pSession = g_Session_Arr[dwSessionID];

	closesocket(pSession->Socket);

	pSession->dwCreateTime = 0;
	pSession->dwLastFD_ISSETTime = 0;
	pSession->dwLastRecvTime = 0;
	pSession->dwLastSelectSetTime = 0;
	pSession->dwSessionID = 0;
	pSession->Socket = 0;

	st_SESSION_MemPool.Free(pSession);
	g_Session_Arr[dwSessionID] = NULL;
	g_SessionID_Q.push(dwSessionID);
}

void netStartUp(void)
{
	// g_SessionID = 0;

	struct addrinfo hints;
	struct addrinfo* result = NULL;
	SOCKET Temp_Listen_Socket = g_Listen_Socket;
	char Port[6];
	u_long on;
	linger Linger_Opt;
	st_SESSION* st_p_New_Session;


	//---------------------------------------------------
	// return 값 저장 변수 
	int Ret_WSAStartup;
	int Ret_getaddrinfo;
	int Ret_bind;
	int Ret_listen;
	int Ret_ioctlsocket;
	int Ret_setsockopt;
	errno_t Ret_itoa_s;


	Temp_Listen_Socket = g_Listen_Socket;


	//---------------------------------------------------
	// Initialize Winsock
	Ret_WSAStartup = WSAStartup(MAKEWORD(2, 2), &g_WsaData);
	if (Ret_WSAStartup != 0)
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"WSAStartup failed with error: %d", Ret_WSAStartup);
		__debugbreak();
	}
	_LOG(dfLOG_LEVEL_SYSTEM, L"WSAStartup # ");

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
	// 아래의 Port 코드는 C6054 : 'Port' 문자열이 0으로 종료되지 않을 수 있습니다. 경고를 해결하기 위하여 작성하였다. 
	Ret_itoa_s = _itoa_s(dfNETWORK_PORT, Port, sizeof(Port), 10);
	if (Ret_itoa_s != NULL)
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"_itoa_s failed with error");
		WSACleanup();
		__debugbreak();
	}
	Port[sizeof(Port) - 1] = '\0';
	Ret_getaddrinfo = getaddrinfo(NULL, Port, &hints, &result);
	if (Ret_getaddrinfo != 0)
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"getaddrinfo failed with error: %d", Ret_getaddrinfo);
		WSACleanup();
		__debugbreak();
	}

	//---------------------------------------------------
	// Create a SOCKET for the server to listen for client connections.
	// 서버가 클라이언트 연결을 수신을 SOCKET을 만든다.
	Temp_Listen_Socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (Temp_Listen_Socket == INVALID_SOCKET)
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"socket failed with error : %ld", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		__debugbreak();
	}

	//---------------------------------------------------
	// setsockopt_Linger
	Linger_Opt.l_onoff = 1;
	Linger_Opt.l_linger = 0;
	Ret_setsockopt = setsockopt(Temp_Listen_Socket, SOL_SOCKET, SO_LINGER, (char*)&Linger_Opt, sizeof(Linger_Opt));
	if (Ret_setsockopt == SOCKET_ERROR)
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"setsockopt failed with error: %ld", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}

	//---------------------------------------------------
	// Setup the TCP listening socket
	// TCP listening socket 을 준비한다.
	Ret_bind = bind(Temp_Listen_Socket, result->ai_addr, (int)result->ai_addrlen);
	if (Ret_bind == SOCKET_ERROR)
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"bind failed with error: %d", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	_LOG(dfLOG_LEVEL_SYSTEM, L"Bind OK # Port: %d", dfNETWORK_PORT);

	freeaddrinfo(result);

	//---------------------------------------------------
	// Listen a client socket
	Ret_listen = listen(Temp_Listen_Socket, SOMAXCONN);
	//i_Result = listen(Temp_Listen_Socket, SOMAXCONN_HINT(65535));	// 양수 ~200 -> 기본 200개, 200~ 갯수 적용,    
	if (Ret_listen == SOCKET_ERROR)
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"listen failed with error: %d", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	_LOG(dfLOG_LEVEL_SYSTEM, L"Listen OK # ");


	//---------------------------------------------------
	// 넌블로킹 소켓으로 전환
	on = 1;
	Ret_ioctlsocket = ioctlsocket(Temp_Listen_Socket, FIONBIO, &on);
	if (Ret_ioctlsocket == SOCKET_ERROR)
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"ioctlsocket failed with error: %ld", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}

	//------------------------------------------------------------
	// Listen_Socket을 전역 변수에 저장하고
	// Select를 사용하기 위해서 g_Session_Hash에 저장한다.
	g_Listen_Socket = Temp_Listen_Socket;
	st_p_New_Session = st_SESSION_MemPool.Alloc();
	st_p_New_Session->Init(Temp_Listen_Socket, 0);
	st_p_New_Session->dwLastRecvTime = INFINITE- 32000;
	g_Session_Arr[0] = st_p_New_Session;
}

void netCleanUp(void)
{
	st_SESSION* st_Temp_Session;
	int iSessionID;

	WSACleanup();
	_LOG(dfLOG_LEVEL_SYSTEM, L"WSACleanup # \n");

	// _Session_Map 순회를 돌면서 삭제해야 하는데,,,, 삭제 구조가 꼬이기 시작했다.
	// 아마도 할당한 쪽에서 데이터에 대한 삭제를 마무리 하는게 좋아 보인다. 
	// 여기서는 여기 클래스에서 생성된 것들만 정리하고 지워준다. 
	// Socket, RecvQ, SendQ
	_LOG(dfLOG_LEVEL_SYSTEM, L"Close Listen_Socket # \n");
	for (iSessionID = 0; iSessionID < cstMAX_SESSION; ++iSessionID)
	{
		st_Temp_Session = g_Session_Arr[iSessionID];

		if (st_Temp_Session == NULL) continue;

		closesocket(st_Temp_Session->Socket);
		delete st_Temp_Session->RecvQ;
		delete st_Temp_Session->SendQ;

		st_SESSION_MemPool.Free(st_Temp_Session);
	}
	_LOG(dfLOG_LEVEL_SYSTEM, L"CleanUp Session_Hash # \n");


}

void netIOProcess(void)
{
	st_SESSION* st_pSession;
	FD_SET ReadSet;
	FD_SET WriteSet;
	timeval Time;

	DWORD TempTime;

	int iSessionID;
	int iBeginSessionID;
	int i_FD_SET_Count;
	int iCnt;
	int i_Result;
	int iSelectCnt;

	bool isSendFPS;


	//------------------------------------------
	// NULL: 한개라도 응답 올 때까지 무한히 기다림
	// 0,0: 응답올때까지 기다리지 않고 바로 끝낸다.
	// 양수: 한개라도 응답하거나, 지정된 시간이 지나면 리턴
	//------------------------------------------
	Time.tv_sec = 0;
	Time.tv_usec = 0;
	TempTime = timeGetTime();

	//-------------------------------------------------------------------------
	// Send 횟수를 줄이기 위한 프레임 체크
	if (TempTime - g_dwPreTime >= 40)
	{
		isSendFPS = true;
		g_dwPreTime = TempTime - ((TempTime - g_dwFirst) % 40);
	}
	else
	{
		isSendFPS = false;
	}

	//--------------------------------------------------------------------------------------------------------------------
	// _Session 전부를 Select에 등록하면 반복문을 종료한다. 
	// 
	//--------------------------------------------------------------------------------------------------------------------
	for (iSessionID = 0; iSessionID < cstMAX_SESSION; )
	{
		FD_ZERO(&ReadSet);
		FD_ZERO(&WriteSet);

		iBeginSessionID = iSessionID;
		for (i_FD_SET_Count = 0; i_FD_SET_Count < 64 && iSessionID < cstMAX_SESSION; ++iSessionID)
		{
			st_pSession = g_Session_Arr[iSessionID];

			if (st_pSession == NULL) continue;

			////--------------------------------------------------
			//// 하트비트 판단.
			//if (st_pSession->dwLastRecvTime + 30020 < TempTime)
			//{
			//	_LOG(dfLOG_LEVEL_ERROR, L"# Heartbeat Over / SessionID: %d / HeartTime:%d / ServerTime:%d", iSessionID, st_pSession->dwLastRecvTime, TempTime);
			//	enqueueForDeletion(iSessionID);
			//	continue;
			//}

#ifdef DISCONNECT_SERVER
			st_pSession->dwLastSelectSetTime = timeGetTime();
#endif // DISCONNECT_SERVER


			st_pSession->dwLastSelectSetTime = timeGetTime();
			FD_SET(st_pSession->Socket, &ReadSet);
			if (st_pSession->SendQ->GetUseSize() > 0 && isSendFPS)
			{
				FD_SET(st_pSession->Socket, &WriteSet);
			}

			++i_FD_SET_Count;
		}

#ifdef SELECT_DEBUG
		SessionCount += i_FD_SET_Count;
#endif // SELECT_DEBUG


		if (i_FD_SET_Count == 0) break;





		i_Result = select(0, &ReadSet, &WriteSet, 0, &Time);

		if (i_Result == SOCKET_ERROR)
		{
			_LOG(dfLOG_LEVEL_SYSTEM, L"select failed with error: %ld \n", WSAGetLastError());
			__debugbreak();
		}

		for (iCnt = 0, iSelectCnt = 0; i_Result != iSelectCnt; ++iCnt)
		{
			st_pSession = g_Session_Arr[iBeginSessionID + iCnt];

			if (st_pSession == NULL) continue;

			if (FD_ISSET(st_pSession->Socket, &ReadSet))
			{
#ifdef DISCONNECT_SERVER
				st_pSession->dwLastFD_ISSETTime = timeGetTime();
#endif // DISCONNECT_SERVER

				//------------------------------------------
				// Listen_Socket인 경우 == SessionID == 0
				// 하트비트가 INFINTE이기 때문에 체크하지 않는다. 
				if (iBeginSessionID + iCnt == 0)
				{
					netProc_Accept();
					++iSelectCnt;
					continue;
				}

				netProc_Recv(st_pSession);
				st_pSession->dwLastRecvTime = timeGetTime();
				++iSelectCnt;
			}

			if (FD_ISSET(st_pSession->Socket, &WriteSet))
			{
#ifdef DISCONNECT_SERVER
				st_pSession->dwLastFD_ISSETTime = timeGetTime();
#endif // DISCONNECT_SERVER
				netProc_Send(st_pSession);
				++iSelectCnt;
			}
		}

		if (i_Result != iSelectCnt)
			__debugbreak();
	}

	Disconnect();
}


//// Listen_Socket = g_Listen_Socket;
	//iter = g_Session_Hash.begin();
	////--------------------------------------------------------------------------------------------------------------------
	//// _Session 전부를 Select에 등록하면 while문을 종료한다. 
	//// 
	////--------------------------------------------------------------------------------------------------------------------
	//while (iter != g_Session_Hash.end())
	//{
	//	FD_ZERO(&ReadSet);
	//	FD_ZERO(&WriteSet);
	//
	//
	//	//------------------------------------------
	//	// Listen_Socket은 0번 키에 저장되어 있음
	//	// 
	//	// 리슨소켓 및 접속중인 모든 클라이언트에 대해 SOCKET을 체크한다.
	//	// 마지막 _Session이거나 64개의 소켓을 Select에 등록했다면, 반복문을 종료한다.
	//	// 
	//	// 시작 iter을 저장하여 FD_ISSET을 확인할 때 사용한다. 
	//	//------------------------------------------
	//	iter_FD_ISSET = iter;
	//	for (iCnt = 0; iCnt < 64 && iter != g_Session_Hash.end(); ++iter)
	//	{
	//		st_pSession = iter->second;
	//
	//		//------------------------------------------
	//		// 해당 클라이언트 Resd Set 등록 / SendQ에 데이터가 있다면 Write Set 등록
	//		// Listen_Socket의 경우 WriteSet에 등록 안된다.
	//		//------------------------------------------
	//		FD_SET(st_pSession->Socket, &ReadSet);
	//		if (st_pSession->SendQ->GetUseSize() > 0)
	//			FD_SET(st_pSession->Socket, &WriteSet);
	//		++iCnt;
	//	}
	//
	//	//------------------------------------------
	//	// NULL: 한개라도 응답 올 때까지 무한히 기다림
	//	// 0,0: 응답올때까지 기다리지 않고 바로 끝낸다.
	//	// 양수: 한개라도 응답하거나, 지정된 시간이 지나면 리턴
	//	//------------------------------------------
	//	i_Result = select(0, &ReadSet, &WriteSet, 0, &Time);
	//
	//	if (i_Result == SOCKET_ERROR)
	//	{
	//		_LOG(dfLOG_LEVEL_DEBUG, L"select failed with error: %ld \n", WSAGetLastError());
	//		__debugbreak();
	//	}
	//
	//
	//	//------------------------------------------
	//	// 리턴값이 0 이상이라면 누군가의 데이터가 왔다.
	//	//
	//	// 매 루프 Listen_Socket을 넣고 확인하는게 좋은지
	//	// 프레임에 1번만 넣는데, 매번 확인하는게 빠른지 잘 모르겠음
	//	//
	//	// 전체 세션중 어떤 세션이 반응을 보였는지 다시 확인한다.
	//	// 
	//	// 조건 1. i_Result > 0
	//	// 조건 2. 0부터 iCnt개 만큼 소켓을 확인한다. 
	//	// 조건 3. 소켓은 iter_FD_ISSET 부터 시작한다. 
	//	//------------------------------------------
	//	for (iCnt_FD_ISSET = 0; (iCnt_FD_ISSET < iCnt) && i_Result > 0; ++iter_FD_ISSET)
	//	{
	//		st_pSession = (*iter_FD_ISSET).second;
	//
	//		if (FD_ISSET(st_pSession->Socket, &ReadSet))
	//		{
	//			//-----------------------------
	//			// Listen_Socket인 경우
	//			// 하트비트가 INFINTE이기 때문에 체크하지 않는다. 
	//			if (st_pSession->Socket == g_Listen_Socket)
	//			{
	//				netProc_Accept();
	//				--i_Result;
	//				goto ICNT_FD_ISSET;
	//			}
	//
	//			//-----------------------------
	//			// 일반 소켓인 경우
	//			// 하트비트도 업데이트 해야한다. 
	//			netProc_Recv(st_pSession);
	//			st_pSession->dwLastRecvTime = g_Start_Time;
	//			--i_Result;
	//		}
	//
	//		if (FD_ISSET(st_pSession->Socket, &WriteSet))
	//		{
	//			netProc_Send(st_pSession);
	//			--i_Result;
	//		}
	//
	//	ICNT_FD_ISSET:
	//		++iCnt_FD_ISSET;
	//	}
	//}


//	do
//	//while (iter != g_Session_Hash.end())
//	{
//		FD_ZERO(&ReadSet);
//		FD_ZERO(&WriteSet);
//
//		//------------------------------------------
//		// Listen_Socket 넣기
//		//------------------------------------------
//		FD_SET(g_Listen_Socket, &ReadSet);
//
//		//------------------------------------------
//		// 리슨소켓 및 접속중인 모든 클라이언트에 대해 SOCKET 을 체크한다. 
//		// 
//		// 마지막 _Session이거나 or _Listen_Socket 포함 64개의 소켓을 Select에 등록했다면, 반복문을 종료한다. 
//		//------------------------------------------
//		for (iCnt = 0; iCnt < 64 - 1 && iter != g_Session_Hash.end(); ++iter)
//		{
//			st_pSession = iter->second;
//
//#ifndef _DEBUG
//			//---------------------------------
//			// 하트비트 체크
//			if (st_pSession->dwLastRecvTime + 30000 < g_Start_Time)
//			{
//				_LOG(0, L"# Heartbeat Over / SessionID:%d", st_pSession->dwSessionID);
//				enqueueForDeletion(st_pSession->dwSessionID);
//				continue;
//			}
//#endif // !_DEBUG
//
//			//------------------------------------------
//			// 해당 클라이언트 Read Set 등록 / SendQ 에 데이터가 있다면 Write Set 등록
//			//------------------------------------------
//			FD_SET(st_pSession->Socket, &ReadSet);
//			if (st_pSession->SendQ->GetUseSize() > 0)
//				FD_SET(st_pSession->Socket, &WriteSet);
//			++iCnt;
//		}
//
//		//------------------------------------------
//		// NULL: 한개라도 응답 올 때까지 무한히 기다림
//		// 0,0: 응답올때까지 기다리지 않고 바로 끝낸다.
//		// 양수: 한개라도 응답하거나, 지정된 시간이 지나면 리턴
//		//------------------------------------------
//		Time.tv_sec = 0;
//		Time.tv_usec = 0;
//
//		i_Result = select(0, &ReadSet, &WriteSet, 0, &Time);
//
//		if (i_Result == SOCKET_ERROR)
//		{
//			_LOG(0, L"select failed with error: %ld \n", WSAGetLastError());
//			__debugbreak();
//		}
//
//
//		//------------------------------------------
//		// 리턴값이 0 이상이라면 누군가의 데이터가 왔다.
//		//------------------------------------------
//		while (i_Result > 0)
//		{
//			if (FD_ISSET(g_Listen_Socket, &ReadSet))
//			{
//				netProc_Accept();
//				--i_Result;
//			}
//
//			//------------------------------------------
//			// 전체 세션중 어떤 세션이 반응을 보였는지 다시 확인한다. 
//			//------------------------------------------
//			for (iter_FD_ISSET = g_Session_Hash.begin(); iter_FD_ISSET != g_Session_Hash.end(); ++iter_FD_ISSET)
//			{
//				st_pSession = (*iter_FD_ISSET).second;
//
//				if (FD_ISSET(st_pSession->Socket, &ReadSet))
//				{
//					netProc_Recv(st_pSession);
//
//					//----------------------------
//					// 하트비트 업데이트
//					st_pSession->dwLastRecvTime = g_Start_Time;
//					--i_Result;
//				}
//
//				if (FD_ISSET(st_pSession->Socket, &WriteSet))
//				{
//					netProc_Send(st_pSession);
//					--i_Result;
//				}
//			}
//		}
//	} while (iter != g_Session_Hash.end());
// }

void netProc_Accept(void)
{
	//---------------------------------------------------------------------------------
	// 새로운 연결에 대하여 accept를 하고,
	// SessionID를 할당하고, 
	// 내 눈에 보이는 시야에 한정하여 캐릭터 생성을 보낸다. 
	//---------------------------------------------------------------------------------

	int Error;

	SOCKET New_Client_Socket;
	sockaddr_in Clinet_Addr;

	st_SECTOR_AROUND st_Sector_Around;

	st_SESSION* st_p_New_Session;
	st_PLAYER* st_p_New_Player;

	DWORD SessionID;
	int Client_Addr_Len;

	char host[NI_MAXHOST];

	Client_Addr_Len = sizeof(Clinet_Addr);
	New_Client_Socket = accept(g_Listen_Socket, (sockaddr*)&Clinet_Addr, &Client_Addr_Len);
	if (New_Client_Socket == INVALID_SOCKET)
	{
		Error = WSAGetLastError();

		if (Error == WSAEWOULDBLOCK)
		{
			// Seletc로 거르고 들어왔는데 WSAEWOULDBLOCK이 나오는지 모르겠다. 
			_LOG(dfLOG_LEVEL_SYSTEM, L"accept failed with error: %ld \n", Error);
			//__debugbreak();
			// 원래는 에러로 처리하면 안된다. 
			return;
		}
		else
		{
			_LOG(dfLOG_LEVEL_SYSTEM, L"accept failed with error: %ld \n", Error);
			WSACleanup();
			__debugbreak();
		}
	}


	// 접속자 인원수 제한. 순간적인 피크를 포함하여 약 8000명으로 가정한다. 
	if (g_SessionID_Q.empty())
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"Full Server cannot accept!!! \n");
		closesocket(New_Client_Socket);
		return;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. 새로운 세션 만들기.
	// 2. 새로운 플레이어 만들기. 
	// 3. 나에게 생성 메시지 보내기.
	// 4. 주변에 나에대한 생성 메시지를 보낸다. 
	// 5. 행동중인 플레이어가 있다면 동작을 이어서 보여준다. 
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1.
	SessionID = g_SessionID_Q.front();
	g_SessionID_Q.pop();
	// st_p_New_Session = new st_SESSION(New_Client_Socket, SessionID);
	st_p_New_Session = st_SESSION_MemPool.Alloc();
	st_p_New_Session->Init(New_Client_Socket, SessionID);
	//st_p_New_Session->Init(New_Client_Socket, SessionID);
	//new(st_p_New_Session) st_SESSION(New_Client_Socket, SessionID);

	g_Session_Arr[SessionID] = st_p_New_Session;
	// 2.
	st_p_New_Player = CreateNewPlayer(SessionID, st_p_New_Session);
	C_Field::GetInstance()->AddPlayerToSector(st_p_New_Player);
	// 3.

	mpCreateMyCharacter(&g_Packet, SessionID, st_p_New_Player->_byDirection, st_p_New_Player->_X, st_p_New_Player->_Y, st_p_New_Player->_HP);
	SendPacket_Unicast(st_p_New_Session, &g_Packet);
	g_Packet.Clear();
	// 4.
	mpCreateOtherCharacter(&g_Packet, SessionID, st_p_New_Player->_byDirection, st_p_New_Player->_X, st_p_New_Player->_Y, st_p_New_Player->_HP);
	C_Field::GetInstance()->GetSectorAround(st_p_New_Player->_CurSector->iX, st_p_New_Player->_CurSector->iY, &st_Sector_Around);
	C_Field::GetInstance()->SendPacket_Around(st_p_New_Session, &g_Packet, &st_Sector_Around);
	g_Packet.Clear();
	// 5. 
	C_Field::GetInstance()->SendPacket_Around_To_Session_CreateCharacter(st_p_New_Session, &g_Packet, &st_Sector_Around);

	inet_ntop(AF_INET, &Clinet_Addr.sin_addr, host, NI_MAXHOST);
	_c_LOG(dfLOG_LEVEL_DEBUG, "Connect # IP:%s : Port: %d / SessionID: %d", host, ntohs(Clinet_Addr.sin_port), SessionID);
}

int netProc_tempRecv(st_SESSION* pSession)
{
	st_PACKET_HEADER st_Header;

	int i_Ret_Recv_Size = NULL;
	int i_Ret_WSAError = NULL;

	size_t ret_Q_UseSize = NULL;
	size_t ret_Peek = NULL;
	size_t ret_Dq_Size = NULL;

	//----------------------------------------------------------------------
	// 디버깅용 변수들
	char* InBufferPtr;
	size_t DirectEqSize;
	size_t PrevIn;
	size_t PrevOut;

	InBufferPtr = pSession->RecvQ->GetInBufferPtr();
	DirectEqSize = pSession->RecvQ->DirectEnqueueSize();
	PrevIn = pSession->RecvQ->_In;
	PrevOut = pSession->RecvQ->_Out;

	i_Ret_Recv_Size = recv(pSession->Socket, pSession->RecvQ->GetInBufferPtr(), (int)pSession->RecvQ->DirectEnqueueSize(), 0);
	if (i_Ret_Recv_Size == 0)
	{
		// 아마도 상대방의 연결 종료
		_LOG(dfLOG_LEVEL_ERROR, L"Disconnect recv 0 # SessionID: %d", pSession->dwSessionID);
	
		// 연결 끊기
		//
		enqueueForDeletion(pSession->dwSessionID);
	
		return -1;
	}
	else if (i_Ret_Recv_Size == SOCKET_ERROR)
	{
		//////////////////////////////////////////////////////////////////
		// 여기서 연결을 끊지 않아도 아래에서 강제로 연결이 끊긴다.
		//////////////////////////////////////////////////////////////////
	
		i_Ret_WSAError = WSAGetLastError();
		if (i_Ret_WSAError == WSAEWOULDBLOCK)
		{
			//----------------------------------------------------------------------------------------
			// Selete로 걸러서 Recv 할 수 있는 상황에서 WSAEWOULDBLOCK은 절대로 뜨면 안된다.
			// 하지만 원장님에게 물어본 결과 싱글스레드에서는 맞는 말 일 수 있지만,
			// 멀티스레드인 경우는 발생할 수 있다고 한다. 그러므로 넘어간다.
			//----------------------------------------------------------------------------------------
			_LOG(0, L"WSAEWOULDBLOCK # Nect Frame Recv try %d # SessionID: %d", i_Ret_WSAError, pSession->dwSessionID);
			return 0;
		}
		else if (i_Ret_WSAError == 10054)
		{
			// 상대방이 강제로 연결을 끊었다. 
			_LOG(dfLOG_LEVEL_ERROR, L"Disconnect Recv %d # SessionID: %d \n", i_Ret_WSAError, pSession->dwSessionID);
	
			// 연결 끊기
			//
			enqueueForDeletion(pSession->dwSessionID);
	
			return -1;
		}
		else if (i_Ret_WSAError == 10053)
		{
			// 소프트웨어로 인해 연결이 중단됨
			// 데이터 전송 시간 제한 또는 프로토콜 오류
			_LOG(dfLOG_LEVEL_ERROR, L"Disconnect Recv %d # SessionID: %d \n", i_Ret_WSAError, pSession->dwSessionID);
	
			// 연결 끊기
			//
			enqueueForDeletion(pSession->dwSessionID);
	
			return -1;
		}
		else
		{
			// 알 수 없는 이유로 연결이 끊겼다. 
			// 로그를 확인하고 원인을 파악한다음 처리한다. 
	
			_LOG(dfLOG_LEVEL_SYSTEM, L"Disconnect Recv %d # SessionID: %d \n", i_Ret_WSAError, pSession->dwSessionID);
			__debugbreak();
		}
	}
	pSession->RecvQ->MoveIn(i_Ret_Recv_Size);

	//////////////////////////////////////////////////////////////////
// * 완료 패킷 처리부
// 
// 수신 버퍼에서 모든 메시지를 꺼낸 다음에 
// 반복문을 돌면서 완성된 메시지를 처리한다.
//////////////////////////////////////////////////////////////////
	while (1)
	{
		//---------------------------------------------------
		// 기저 사례
		//---------------------------------------------------

		// 기저 사례1. RecvQ에 최소한의 사이즈가 있는지 확인. 조건: 헤더 사이즈 초과
		ret_Q_UseSize = pSession->RecvQ->GetUseSize();

		if (ret_Q_UseSize == 0)
		{
			return 0;
		}

		if (ret_Q_UseSize < sizeof(st_PACKET_HEADER))
		{
			__debugbreak();
		};

		// 헤더를 열어서 페이로드 사이즈를 확인한다. 
		ret_Peek = pSession->RecvQ->Peek((char*)&st_Header, sizeof(st_PACKET_HEADER), true);

		// 기저 사례2. st_Header에서 byCode를 확인한다. 다르면 연결 끊기
		if (st_Header.byCode != (char)dfPACKET_CODE)
		{
			_LOG(dfLOG_LEVEL_ERROR, L"Header code Error # SessionID: %d", pSession->dwSessionID);

			// 연결 끊기
			//
			enqueueForDeletion(pSession->dwSessionID);

			__debugbreak();
		}

		// 기저 사례3. 페이로드가 완성되지 않았다면 반환한다.
		if (ret_Q_UseSize < sizeof(st_PACKET_HEADER) + st_Header.bySize)
		{
			// 메시지가 완성되지 않았으므로 그냥 반환, 다음 프레임에 동작 
			__debugbreak();
			return 0;
		}


		//---------------------------------------------------
		// 메시지 완성됨 -> 꺼내서 처리한다. 
		//---------------------------------------------------
		pSession->RecvQ->MoveOut(sizeof(st_PACKET_HEADER));
		ret_Dq_Size = pSession->RecvQ->Dequeue(g_Packet.GetBufferWritePtr(), st_Header.bySize);
		g_Packet.MoveWritePos(ret_Dq_Size);

		_LOG(0, L"netProc_Recv - Recd Byte(Peek + DQSize):%Iu", (ret_Peek + ret_Dq_Size));

		if (!PacketProc(pSession, st_Header.byType, &g_Packet))
		{
			_LOG(dfLOG_LEVEL_SYSTEM, L"PacketProc Fail # SessionID:%d", pSession->dwSessionID);
			enqueueForDeletion(pSession->dwSessionID);
			g_Packet.Clear();
			return -1;
		}

		g_Packet.Clear();
	}
}

int netProc_Recv(st_SESSION* pSession)
{

	st_PACKET_HEADER st_Header;

	DWORD i_ret_Recv_Size = NULL;
	int i_ret_WSAError = NULL;
	size_t ret_Q_UseSize = NULL;
	size_t ret_Peek = NULL;
	size_t ret_Dq_Size = NULL;
	
	WSABUF wsaBuf[2];
	int Ret_WSARecv = NULL;
	DWORD dwFlags = 0;

	//----------------------------------------------------------------
	// 디버깅용
	size_t Debug_In = NULL;
	size_t Debug_Out = NULL;
	DWORD Debug_UpdateFPS = dwUpdateFPS;
	DWORD Debug_Loop = dwDebug_Loop;

	Debug_In = pSession->RecvQ->_In;
	Debug_Out = pSession->RecvQ->_Out;

	wsaBuf[0].buf = pSession->RecvQ->GetInBufferPtr();
	wsaBuf[0].len = pSession->RecvQ->DirectEnqueueSize();
	wsaBuf[1].buf = pSession->RecvQ->GetBeginBufferPtr();
	wsaBuf[1].len = pSession->RecvQ->GetFreeSize() - pSession->RecvQ->DirectEnqueueSize();

	Ret_WSARecv = WSARecv(pSession->Socket, wsaBuf, 2, &i_ret_Recv_Size, &dwFlags, NULL, NULL);

	if (Ret_WSARecv == SOCKET_ERROR)
	{
		i_ret_WSAError = WSAGetLastError();
		if (i_ret_WSAError == WSAEWOULDBLOCK)
		{
			//----------------------------------------------------------------------------------------
			// Selete로 걸러서 Recv 할 수 있는 상황에서 WSAEWOULDBLOCK은 절대로 뜨면 안된다.
			// 하지만 원장님에게 물어본 결과 싱글스레드에서는 맞는 말 일 수 있지만,
			// 멀티스레드인 경우는 발생할 수 있다고 한다. 그러므로 넘어간다.
			//----------------------------------------------------------------------------------------
			_LOG(0, L"WSAEWOULDBLOCK # Nect Frame Recv try %d # SessionID: %d", i_ret_WSAError, pSession->dwSessionID);
			return 0;
		}
		else if (i_ret_WSAError == 10054)
		{
			// 상대방이 강제로 연결을 끊었다. 
			_LOG(dfLOG_LEVEL_ERROR, L"Disconnect Recv %d # SessionID: %d \n", i_ret_WSAError, pSession->dwSessionID);

			// 연결 끊기
			//
			enqueueForDeletion(pSession->dwSessionID);

			return -1;
		}
		else if (i_ret_WSAError == 10053)
		{
			// 소프트웨어로 인해 연결이 중단됨
			// 데이터 전송 시간 제한 또는 프로토콜 오류
			_LOG(dfLOG_LEVEL_ERROR, L"Disconnect Recv %d # SessionID: %d \n", i_ret_WSAError, pSession->dwSessionID);

			// 연결 끊기
			//
			enqueueForDeletion(pSession->dwSessionID);

			return -1;
		}
		else
		{
			// 알 수 없는 이유로 연결이 끊겼다. 
			// 로그를 확인하고 원인을 파악한다음 처리한다. 

			_LOG(dfLOG_LEVEL_SYSTEM, L"Disconnect Recv %d # SessionID: %d \n", i_ret_WSAError, pSession->dwSessionID);
			__debugbreak();
		}
	}

	if (i_ret_Recv_Size == 0)
	{
		// 아마도 상대방의 연결 종료
		_LOG(dfLOG_LEVEL_ERROR, L"Disconnect recv 0 # SessionID: %d", pSession->dwSessionID);

		// 연결 끊기
		//
		enqueueForDeletion(pSession->dwSessionID);

		return -1;
	}
	pSession->RecvQ->MoveIn(i_ret_Recv_Size);
	_LOG(dfLOG_LEVEL_DEBUG, L"# SessionID:%d / RecvByte:%d / DirectEQSize:%lu", pSession->dwSessionID, i_ret_Recv_Size, pSession->RecvQ->DirectEnqueueSize());



	//i_ret_Recv_Size = recv(pSession->Socket, pSession->RecvQ->GetInBufferPtr(), (int)pSession->RecvQ->DirectEnqueueSize(), 0);
	//if (i_ret_Recv_Size == 0)
	//{
	//	// 아마도 상대방의 연결 종료
	//	_LOG(dfLOG_LEVEL_ERROR, L"Disconnect recv 0 # SessionID: %d", pSession->dwSessionID);
	//
	//	// 연결 끊기
	//	//
	//	enqueueForDeletion(pSession->dwSessionID);
	//
	//	return -1;
	//}
	//else if (i_ret_Recv_Size == SOCKET_ERROR)
	//{
	//	//////////////////////////////////////////////////////////////////
	//	// 여기서 연결을 끊지 않아도 아래에서 강제로 연결이 끊긴다.
	//	//////////////////////////////////////////////////////////////////
	//
	//	i_ret_WSAError = WSAGetLastError();
	//	if (i_ret_WSAError == WSAEWOULDBLOCK)
	//	{
	//		//----------------------------------------------------------------------------------------
	//		// Selete로 걸러서 Recv 할 수 있는 상황에서 WSAEWOULDBLOCK은 절대로 뜨면 안된다.
	//		// 하지만 원장님에게 물어본 결과 싱글스레드에서는 맞는 말 일 수 있지만,
	//		// 멀티스레드인 경우는 발생할 수 있다고 한다. 그러므로 넘어간다.
	//		//----------------------------------------------------------------------------------------
	//		_LOG(0, L"WSAEWOULDBLOCK # Nect Frame Recv try %d # SessionID: %d", i_ret_WSAError, pSession->dwSessionID);
	//		return 0;
	//	}
	//	else if (i_ret_WSAError == 10054)
	//	{
	//		// 상대방이 강제로 연결을 끊었다. 
	//		_LOG(dfLOG_LEVEL_ERROR, L"Disconnect Recv %d # SessionID: %d \n", i_ret_WSAError, pSession->dwSessionID);
	//
	//		// 연결 끊기
	//		//
	//		enqueueForDeletion(pSession->dwSessionID);
	//
	//		return -1;
	//	}
	//	else if (i_ret_WSAError == 10053)
	//	{
	//		// 소프트웨어로 인해 연결이 중단됨
	//		// 데이터 전송 시간 제한 또는 프로토콜 오류
	//		_LOG(dfLOG_LEVEL_ERROR, L"Disconnect Recv %d # SessionID: %d \n", i_ret_WSAError, pSession->dwSessionID);
	//
	//		// 연결 끊기
	//		//
	//		enqueueForDeletion(pSession->dwSessionID);
	//
	//		return -1;
	//	}
	//	else
	//	{
	//		// 알 수 없는 이유로 연결이 끊겼다. 
	//		// 로그를 확인하고 원인을 파악한다음 처리한다. 
	//
	//		_LOG(dfLOG_LEVEL_SYSTEM, L"Disconnect Recv %d # SessionID: %d \n", i_ret_WSAError, pSession->dwSessionID);
	//		__debugbreak();
	//	}
	//}
	//pSession->RecvQ->MoveIn(i_ret_Recv_Size);

	//////////////////////////////////////////////////////////////////
	// * 완료 패킷 처리부
	// 
	// 수신 버퍼에서 모든 메시지를 꺼낸 다음에 
	// 반복문을 돌면서 완성된 메시지를 처리한다.
	//////////////////////////////////////////////////////////////////
	while (1)
	{
		//---------------------------------------------------
		// 기저 사례
		//---------------------------------------------------
		
		// 기저 사례1. RecvQ에 최소한의 사이즈가 있는지 확인. 조건: 헤더 사이즈 초과
		ret_Q_UseSize = pSession->RecvQ->GetUseSize();

		if (ret_Q_UseSize == 0)
			return 0;

		if (ret_Q_UseSize < sizeof(st_PACKET_HEADER))
			__debugbreak();

		if (ret_Q_UseSize > 8 * 3)
			__debugbreak();

		// 헤더를 열어서 페이로드 사이즈를 확인한다. 
		ret_Peek = pSession->RecvQ->Peek((char*)&st_Header, sizeof(st_PACKET_HEADER), true);

		// 기저 사례2. st_Header에서 byCode를 확인한다. 다르면 연결 끊기
		if (st_Header.byCode != (char)dfPACKET_CODE)
		{
			_LOG(dfLOG_LEVEL_ERROR, L"Header code Error # SessionID: %d", pSession->dwSessionID);

			// 연결 끊기
			//
			enqueueForDeletion(pSession->dwSessionID);

			__debugbreak();
		}

		// 기저 사례3. 페이로드가 완성되지 않았다면 반환한다.
		if (ret_Q_UseSize < sizeof(st_PACKET_HEADER) + st_Header.bySize)
		{
			// 메시지가 완성되지 않았으므로 그냥 반환, 다음 프레임에 동작 

			__debugbreak();
		}

		
		//---------------------------------------------------
		// 메시지 완성됨 -> 꺼내서 처리한다. 
		//---------------------------------------------------
		pSession->RecvQ->MoveOut(sizeof(st_PACKET_HEADER));
		ret_Dq_Size = pSession->RecvQ->Dequeue(g_Packet.GetBufferWritePtr(), st_Header.bySize);
		g_Packet.MoveWritePos(ret_Dq_Size);
		
		_LOG(0, L"netProc_Recv - Recd Byte(Peek + DQSize):%Iu", (ret_Peek + ret_Dq_Size));


		


		if (!PacketProc(pSession, st_Header.byType, &g_Packet))
		{
			_LOG(dfLOG_LEVEL_SYSTEM, L"PacketProc Fail # SessionID:%d", pSession->dwSessionID);
			enqueueForDeletion(pSession->dwSessionID);
			g_Packet.Clear();
			return -1;
		}

		g_Packet.Clear();
	}
}

void netProc_Send(st_SESSION* pSession)
{
	// Send의 경우 한번에 링버퍼에 있는 모든 메시지를 전송하면 된다.

	DWORD Send_Size;
	int i_Error;
	int Ret_WSASend;

	WSABUF wsaBuf[2];

	wsaBuf[0].buf = pSession->SendQ->GetOutBufferPtr();
	wsaBuf[0].len = pSession->SendQ->DirectDequeueSize();
	wsaBuf[1].buf = pSession->SendQ->GetBeginBufferPtr();
	wsaBuf[1].len = pSession->SendQ->GetUseSize() - pSession->SendQ->DirectDequeueSize();
	if (wsaBuf[1].len >= pSession->SendQ->GetBufferSize() || wsaBuf[1].len < 0)
		wsaBuf[1].len = 0;


	Ret_WSASend = WSASend(pSession->Socket, wsaBuf, 2, &Send_Size, 0, NULL, NULL);
	if (Ret_WSASend == SOCKET_ERROR)
	{
		i_Error = WSAGetLastError();

		if (i_Error == WSAEWOULDBLOCK)
		{
			// Send 할 때 WSAEWOULDBLOCK이 나온다면, 다음 프레임에 데이터를 보낸다. 
			return;
		}
		else if (i_Error == 10054)
		{
			_LOG(dfLOG_LEVEL_ERROR, L"Send failed with error: %ld / SessionID:%d \n", i_Error, pSession->dwSessionID);
			enqueueForDeletion(pSession->dwSessionID);
		}
		else if (i_Error == 10053)
		{
			_LOG(dfLOG_LEVEL_ERROR, L"Send failed with error: %ld / SessionID:%d \n", i_Error, pSession->dwSessionID);
			enqueueForDeletion(pSession->dwSessionID);
		}
		else
		{
			_LOG(dfLOG_LEVEL_SYSTEM, L"Send failed with error: %ld / SessionID:%d \n", i_Error, pSession->dwSessionID);
			enqueueForDeletion(pSession->dwSessionID);
			__debugbreak();
		}
	}
	pSession->SendQ->MoveOut(Send_Size);

	//while (1)
	//{
	//	if (pSession->SendQ->GetUseSize() == 0)
	//		break;
	//
	//	Send_Size = send(pSession->Socket, pSession->SendQ->GetOutBufferPtr(), (int)(pSession->SendQ->DirectDequeueSize()), 0);
	//
	//	if (Send_Size == SOCKET_ERROR)
	//	{
	//		i_Error = WSAGetLastError();
	//
	//		if (i_Error == WSAEWOULDBLOCK)
	//		{
	//			// Send 할 때 WSAEWOULDBLOCK이 나온다면, 다음 프레임에 데이터를 보낸다. 
	//			continue;
	//		}
	//		else if (i_Error == 10054)
	//		{
	//			_LOG(dfLOG_LEVEL_ERROR, L"Send failed with error: %ld / SessionID:%d \n", i_Error, pSession->dwSessionID);
	//			enqueueForDeletion(pSession->dwSessionID);
	//			break;
	//		}
	//		else if (i_Error == 10053)
	//		{
	//			_LOG(dfLOG_LEVEL_ERROR, L"Send failed with error: %ld / SessionID:%d \n", i_Error, pSession->dwSessionID);
	//			enqueueForDeletion(pSession->dwSessionID);
	//			break;
	//		}
	//		else
	//		{
	//			_LOG(dfLOG_LEVEL_SYSTEM, L"Send failed with error: %ld / SessionID:%d \n", i_Error, pSession->dwSessionID);
	//			enqueueForDeletion(pSession->dwSessionID);
	//			__debugbreak();
	//		}
	//	}
	//
	//	pSession->SendQ->MoveOut(Send_Size);
	//}
}

void netHeartbeat(DWORD CurTime)
{
	int iSessionID;
	st_SESSION* pSession;


#ifdef DISCONNECT_SERVER
#endif // DISCONNECT_SERVER


	for (iSessionID = 0; iSessionID < cstMAX_SESSION; ++iSessionID)
	{
		pSession = g_Session_Arr[iSessionID];

		if (pSession == NULL) continue;
		
		//--------------------------------------------------------------------------------
		// 현재 서버와의 RTT가 400ms 이상이 발견되었다. 
		// 또한 하트비트 체크가 2~3초 (2000 ~ 3000) 늦게 들어와도 상관 없다고 하니
		// 하트비트의 간격을 늘린다. 



		if (pSession->dwLastRecvTime + 32000 < CurTime)
		{
			_LOG(dfLOG_LEVEL_DEBUG, L"# Heartbeat Over / SessionID: %d / SessionLastRecvTime:%d / ServerTime:%d / Diff:%d \n", pSession->dwSessionID, pSession->dwLastRecvTime, CurTime, CurTime - pSession->dwLastRecvTime);
#ifdef DISCONNECT_SERVER
			//LOG(L"Disconnect", LEVEL_SYSTEM, false, true, L"# Heartbeat Over / SessionID:%d / SessionLastrecvTime:%d / ServerTime:%d / SessionCreateTime:%d / SessionSelectSetTime:%d",
			//	pSession->dwSessionID, pSession->dwLastRecvTime, CurTime, pSession->dwCreateTime, pSession->dwLastSelectSetTime);

			LOG(L"Disconnect", DEBUG, TEXTFILE, L"# Hearbeat Over / SessionID:%d / ServerTime:%d / SessionLastRecvTime:%d / SessionCreateTime:%d / SessionSelectSetTime:%d / SessionISSETTime:%d / TimeDiff:%d",
				pSession->dwSessionID, CurTime, pSession->dwLastRecvTime, pSession->dwCreateTime, pSession->dwLastSelectSetTime, pSession->dwLastFD_ISSETTime, CurTime - pSession->dwLastRecvTime);
#endif // DISCONNECT_SERVER
			enqueueForDeletion(pSession->dwSessionID);
		}
	}

#ifdef DISCONNECT_SERVER
#endif // DISCONNECT_SERVER


}

void SendPacket_Unicast(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	size_t ret_EnQ_Size;

	ret_EnQ_Size = pSession->SendQ->Enqueue(pPacket->GetBufferPtr(), pPacket->GetDataSize());

	if (ret_EnQ_Size == 0)
	{
		// SendQ 가득참 => SendBuff 가득참 => 클라이언트 RecvBuff 가득참 == 연결을 끊는다.

		_LOG(dfLOG_LEVEL_SYSTEM, L"Disconnect Packet Unicast failed with error # SessionID: %d \n", pSession->dwSessionID);
		enqueueForDeletion(pSession->dwSessionID);
		return;
	}
}


bool PacketProc(st_SESSION* pSession, unsigned char byPacketType, SerializeBuffer* pPacket)
{
	switch (byPacketType)
	{
	case dfPACKET_CS_MOVE_START:
		return netPacketProc_Movestart(pSession, pPacket);
		break;
	case dfPACKET_CS_MOVE_STOP:
		return netPacketProc_MoveStop(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK1:
		return netPacketProc_Attack1(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK2:
		return netPacketProc_Attack2(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK3:
		return netPacketProc_Attack3(pSession, pPacket);
		break;
	case dfPACKET_CS_ECHO:
		return netPacketProc_Echo(pSession, pPacket);
		break;
	default:
		__debugbreak();
		break;
	}

	return true;
}






bool netPacketProc_Movestart(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	char byDirection;
	short shX;
	short shY;
	st_PLAYER* pPlayer;
	st_SECTOR_AROUND st_Sector_Around;


	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	pPacket->Clear();

	//---------------------------------------------------------------------------------------------------------------
	// ID로 캐릭터를 검색한다. 
	//---------------------------------------------------------------------------------------------------------------
	pPlayer = FindCharacter(pSession->dwSessionID);
	if (pPlayer == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTART > SessionID:%d Player Not Found!", pSession->dwSessionID);
		return false;
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTART # SessionID:%d / Direction:%d / Client(X:%d , Y:%d) _ Server(X:%d , Y:%d) / ServerFPS:%d" , 
		pSession->dwSessionID, byDirection, shX, shY, pPlayer->_X, pPlayer->_Y, dwDebug_Loop);

	//---------------------------------------------------------------------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 싱크 패킷을 보내어 좌표 보정.
	// 
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로 
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음. 
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함
	// 지금은 좌표에 대해서는 간단한 구현을 목적으로 하고 있으므로 
	// 서버는 클라이언트의 좌표를 그대로 믿지만, 서버와 너무 큰 차이가 있다면 강제 좌표 동기화 하도록 함
	//---------------------------------------------------------------------------------------------------------------
	if (abs(pPlayer->_X - shX) > dfERROR_RANGE || abs(pPlayer->_Y - shY) > dfERROR_RANGE)
	{

		//_LOG(dfLOG_LEVEL_SYSTEM, L"# SYNC!!!! MOVESTART # SessionID:%d / Direction:%d / Client(X:%d , Y:%d) _ Server(X:%d , Y:%d) / ServerFPS:%d",
		//	pSession->dwSessionID, byDirection, shX, shY, pPlayer->_X, pPlayer->_Y, dwDebug_Loop);

		//LOG(L"SYNC", LEVEL_SYSTEM, true, true, L"# SYNC!!!! MOVESTART # SessionID:%d / PreX:%d _ PreY:%d / PreFPS:%d", pSession->dwSessionID, pPlayer->_PreX, pPlayer->_PreY, pPlayer->_PreFPS);
		//LOG(L"SYNC", LEVEL_SYSTEM, true, true, L"# SYNC!!!! MOVESTART # SessionID:%d / Direction:%d / Client(X:%d , Y:%d) _ Server(X:%d , Y:%d) / ServerFPS:%d",
		//	pSession->dwSessionID, byDirection, shX, shY, pPlayer->_X, pPlayer->_Y, dwDebug_Loop);

#ifdef SYNC_DEBUG
		LOG(L"SYNC", DEBUG, TEXTFILE, L"# SYNC MOVESTART # SessionID:%d / (PreX:%d _ PreY:%d) / PreFPS:%d / PreDir:%d", pSession->dwSessionID, pPlayer->_PreX, pPlayer->_PreY, pPlayer->_PreFPS, pPlayer->_PreAction);
		LOG(L"SYNC", DEBUG, TEXTFILE, L"# SYNC MOBESTART # SessionID:%d / (CurX:%d _ CurX:%d) / CurFPS:%d / CurDir:%d", pSession->dwSessionID, pPlayer->_X, pPlayer->_Y, dwDebug_Loop, byDirection);
#endif // SYNC_DEBUG


		mpSync(pPacket, pPlayer->_SessionID, pPlayer->_X, pPlayer->_Y);
		C_Field::GetInstance()->GetSectorAround(pPlayer->_CurSector->iX, pPlayer->_CurSector->iY, &st_Sector_Around);
		C_Field::GetInstance()->SendPacket_Around(pSession, pPacket, &st_Sector_Around, true);
		pPacket->Clear();

		shX = pPlayer->_X;
		shY = pPlayer->_Y;


		//---------------------------------------------------------------------------------------------------------------
		// 좌표가 틀어진 이유를 찾아야 한다.
		//---------------------------------------------------------------------------------------------------------------

	}


	//---------------------------------------------------------------------------------------------------------------
	// 동작을 변경. 동작번호와, 방향값이 같다.
	//---------------------------------------------------------------------------------------------------------------
	pPlayer->_dwAction = byDirection;

	//---------------------------------------------------------------------------------------------------------------
	// 단순 방향표시용 byDirection (LL, RR)과 
	// 이동시 8방향 (LL, LU, UU, RU, RR, RD, DD, LD) 용 MoveDirecion 이 있음
	//---------------------------------------------------------------------------------------------------------------
	pPlayer->_byMoveDirection = byDirection;

	//---------------------------------------------------------------------------------------------------------------
	// 방향을 변경
	//---------------------------------------------------------------------------------------------------------------
	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pPlayer->_byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RD:
		pPlayer->_byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	default:
		break;
	}
#ifdef SYNC_DEBUG
	pPlayer->_PreX = pPlayer->_X;
	pPlayer->_PreY = pPlayer->_Y;
	pPlayer->_PreFPS = dwDebug_Loop;
	pPlayer->_PreAction = byDirection;
#endif // SYNC_DEBUG

	pPlayer->_X = shX;
	pPlayer->_Y = shY;

	//---------------------------------------------------------------------------------------------------------------
	// 섹터 처리
	// 정지를 하면서 좌표가 약간 조절된 경우 섹터 업데이트를 함
	// 위에서 좌표를 변경하였으면 섹터도 다시 재설정 해야한다. 
	//---------------------------------------------------------------------------------------------------------------
	if (C_Field::GetInstance()->Sector_UpdateCharacter(pPlayer))
	{
		//---------------------------------------------------------------------------------------------------------------
		// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다. 
		//---------------------------------------------------------------------------------------------------------------
		C_Field::GetInstance()->CharacterSectorUpdatePacket(pPlayer);

	}

	mpMoveStart(pPacket, pSession->dwSessionID, byDirection, pPlayer->_X, pPlayer->_Y);

	//---------------------------------------------------------------------------------------------------------------
	// 현재 접속주인 사용자에게 모든 패킷을 뿌린다. (섹터 단위 패킷 전송 함수)
	//---------------------------------------------------------------------------------------------------------------
	C_Field::GetInstance()->GetSectorAround(pPlayer->_CurSector->iX, pPlayer->_CurSector->iY, &st_Sector_Around);
	C_Field::GetInstance()->SendPacket_Around(pSession, pPacket, &st_Sector_Around);
	pPacket->Clear();

	return true;
}
bool netPacketProc_MoveStop(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	char byDirection;
	short shX;
	short shY;
	st_PLAYER* pPlayer;
	st_SECTOR_AROUND st_Sector_Around;

	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	pPacket->Clear();

	//---------------------------------------------------------------------------------------------------------------
	// ID로 캐릭터를 검색한다. 
	//---------------------------------------------------------------------------------------------------------------
	pPlayer = FindCharacter(pSession->dwSessionID);
	if (pPlayer == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTOP > SessionID:%d Player Not Found!", pSession->dwSessionID);
		return false;
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTOP # SessionID:%d / Direction:%d / Client(X:%d , Y:%d) _ Server(X:%d , Y:%d) / ServerFPS:%d",
		pSession->dwSessionID, byDirection, shX, shY, pPlayer->_X, pPlayer->_Y, dwDebug_Loop);



	//---------------------------------------------------------------------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 싱크 패킷을 보내어 좌표 보정.
	// 
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로 
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음. 
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함
	// 지금은 좌표에 대해서는 간단한 구현을 목적으로 하고 있으므로 
	// 서버는 클라이언트의 좌표를 그대로 믿지만, 서버와 너무 큰 차이가 있다면 강제 좌표 동기화 하도록 함
	//---------------------------------------------------------------------------------------------------------------
	if (abs(pPlayer->_X - shX) > dfERROR_RANGE || abs(pPlayer->_Y - shY) > dfERROR_RANGE)
	{
		//_LOG(dfLOG_LEVEL_SYSTEM, L"# SYNC!!!! MOVESTOP # SessionID:%d / Direction:%d / Client(X:%d , Y:%d) _ Server(X:%d , Y:%d) / ServerFPS:%d",
		//	pSession->dwSessionID, byDirection, shX, shY, pPlayer->_X, pPlayer->_Y, dwDebug_Loop);

		//LOG(L"SYNC", LEVEL_SYSTEM, true, true, L"# SYNC!!!! MOVESTOP # SessionID:%d / PreX:%d _ PreY:%d / PreFPS:%d", pSession->dwSessionID, pPlayer->_PreX, pPlayer->_PreY, pPlayer->_PreFPS);
		//LOG(L"SYNC", LEVEL_SYSTEM, true, true, L"# SYNC!!!! MOVESTOP # SessionID:%d / Direction:%d / Client(X:%d , Y:%d) _ Server(X:%d , Y:%d) / ServerFPS:%d",
		//	pSession->dwSessionID, byDirection, shX, shY, pPlayer->_X, pPlayer->_Y, dwDebug_Loop);


#ifdef SYNC_DEBUG
		LOG(L"SYNC", DEBUG, TEXTFILE, L"# SYNC MOVESTOP # SessionID:%d / (PreX:%d _ PreY:%d) / PreFPS:%d / PreDir:%d", pSession->dwSessionID, pPlayer->_PreX, pPlayer->_PreY, pPlayer->_PreFPS, pPlayer->_PreAction);
		LOG(L"SYNC", DEBUG, TEXTFILE, L"# SYNC MOVESTOP # SessionID:%d / (CurX:%d _ CurX:%d) / CurFPS:%d / CurDir:%d", pSession->dwSessionID, pPlayer->_X, pPlayer->_Y, dwDebug_Loop, byDirection);
#endif // SYNC_DEBUG

		mpSync(pPacket, pPlayer->_SessionID, pPlayer->_X, pPlayer->_Y);
		SendPacket_Unicast(pSession, pPacket);
		 pPacket->Clear();

		shX = pPlayer->_X;
		shY = pPlayer->_Y;



		//---------------------------------------------------------------------------------------------------------------
		// 좌표가 틀어진 이유를 찾아야 한다.
	}

	
	//---------------------------------------------------------------------------------------------------------------
	// 동작을 변경. 동작번호와, 방향값이 같다.
	//---------------------------------------------------------------------------------------------------------------
	pPlayer->_dwAction = dfPACKET_CS_MOVE_STOP;

	//---------------------------------------------------------------------------------------------------------------
	// 단순 방향표시용 byDirection (LL, RR)과 
	// 이동시 8방향 (LL, LU, UU, RU, RR, RD, DD, LD) 용 MoveDirecion 이 있음
	//---------------------------------------------------------------------------------------------------------------
	pPlayer->_byMoveDirection = dfPACKET_CS_MOVE_STOP;

	//---------------------------------------------------------------------------------------------------------------
	// 방향을 변경
	//---------------------------------------------------------------------------------------------------------------
	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pPlayer->_byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RD:
		pPlayer->_byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	default:
		break;
	}

#ifdef SYNC_DEBUG
	pPlayer->_PreX = pPlayer->_X;
	pPlayer->_PreY = pPlayer->_Y;
	pPlayer->_PreFPS = dwDebug_Loop;
	pPlayer->_PreAction = byDirection;
#endif // SYNC_DEBUG
	

	pPlayer->_X = shX;
	pPlayer->_Y = shY;

	//---------------------------------------------------------------------------------------------------------------
	// 섹터 처리
	// 정지를 하면서 좌표가 약간 조절된 경우 섹터 업데이트를 함
	// 위에서 좌표를 변경하였으면 섹터도 다시 재설정 해야한다. 
	//---------------------------------------------------------------------------------------------------------------
	if (C_Field::GetInstance()->Sector_UpdateCharacter(pPlayer))
	{
		//---------------------------------------------------------------------------------------------------------------
		// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다. 
		//---------------------------------------------------------------------------------------------------------------
		C_Field::GetInstance()->CharacterSectorUpdatePacket(pPlayer);
	}

	mpMoveStop(pPacket, pSession->dwSessionID, pPlayer->_byDirection, pPlayer->_X, pPlayer->_Y);


	//---------------------------------------------------------------------------------------------------------------
	// 현재 접속주인 사용자에게 모든 패킷을 뿌린다. (섹터 단위 패킷 전송 함수)
	//---------------------------------------------------------------------------------------------------------------
	C_Field::GetInstance()->GetSectorAround(pPlayer->_CurSector->iX, pPlayer->_CurSector->iY, &st_Sector_Around);
	C_Field::GetInstance()->SendPacket_Around(pSession, pPacket, &st_Sector_Around);
	pPacket->Clear();

	return true;

}
bool netPacketProc_Attack1(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	// 공격 하는 순간 판정까지 떨어진다. 
	char byDirection;
	short shX;
	short shY;
	st_PLAYER* pHitPlayer;
	st_PLAYER* pTempHitPlayer;
	st_SECTOR_AROUND st_Sector_Around;
	CList<st_PLAYER*>* pCList;
	CList<st_PLAYER*>::iterator iter;
	int iCnt;


	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	pPacket->Clear();

	_LOG(dfLOG_LEVEL_DEBUG, L"# Attack1 # SessionID:%d / Direction:%d / X:%d / Y:%d", pSession->dwSessionID, byDirection, shX, shY);

	////---------------------------------------------------------------------------------------------------------------
	//// ID로 캐릭터를 검색한다. 
	////---------------------------------------------------------------------------------------------------------------
	//pPlayer = FindCharacter(pSession->dwSessionID);
	//if (pPlayer == NULL)
	//{
	//	_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTART > SessionID:%d Player Not Found!", pSession->dwSessionID);
	//	return false;
	//}


	// 공격 모션에 대한 패킷은 해당 플레이어가 보이는 모든 세션에게 보내야 한다. 
	mpAttack1(pPacket, pSession->dwSessionID, byDirection, shX, shY);
	C_Field::GetInstance()->GetSectorAround(shX/dfGRID_X_SIZE, shY/dfGRID_Y_SIZE, &st_Sector_Around);
	C_Field::GetInstance()->SendPacket_Around(pSession, pPacket, &st_Sector_Around);
	pPacket->Clear();

	//---------------------------------------------------------------------------------------------------------------
	// 데미지에 대한 처리
	// 
	// 바라보는 방향을 기준으로 0 ~ _dfATTACK1_RANGE_X 범위의 섹터를 계산
	// 공격 범위 또한 계산해야 한다. 
	// 
	// 1. 같은 섹터에서 검색한다. 
	// 2. 섹터를 넘어간다면 다른 섹터에서도 계산한다.
	// 3. 3개의 섹터에 걸치는 경우도 있다.
	// 4. 공격 범위에 있는 섹터를 구했다면, 가장 가까이 있는 플레이어를 구한다.
	// 5. 해당 플레이어의 HP를 깍고
	// 6. 피격자 기준으로 섹터를 계산하여 메시지를 보낸다. 
	//---------------------------------------------------------------------------------------------------------------
	pHitPlayer = NULL;
	C_Field::GetInstance()->GetAttackSectorAround(shX, shY, byDirection, dfATTACK1_RANGE_X, dfATTACK1_RANGE_Y, &st_Sector_Around);
	for (iCnt = 0; iCnt < st_Sector_Around.iCount; ++iCnt)
	{
		pCList = C_Field::GetInstance()->GetPlayerInSectorCList(st_Sector_Around.Around[iCnt].iX, st_Sector_Around.Around[iCnt].iY);

		for (iter = (*pCList).begin(); iter != (*pCList).end(); ++iter)
		{
			pTempHitPlayer = (*iter);
			
			// 타격자와 피격자가 같으면 넘어간다. 
			if (pSession->dwSessionID == pTempHitPlayer->_SessionID) continue;

			// Y좌표 공격 범위에 들어갔는지 체크
			if (shY - dfATTACK1_RANGE_Y < pTempHitPlayer->_Y && pTempHitPlayer->_Y < shY + dfATTACK1_RANGE_Y)
			{
				// X좌표 공격 범위를 체크한다. 
				if (byDirection == dfPACKET_MOVE_DIR_LL)
				{
					if (shX - dfATTACK1_RANGE_X <= pTempHitPlayer->_X && pTempHitPlayer->_X <= shX)
					{
						pHitPlayer = pTempHitPlayer;
						goto FINDATTACK1;
					}
				}
				else
				{
					if (shX<=pTempHitPlayer->_X && pTempHitPlayer->_X<=shX+dfATTACK1_RANGE_X)
					{
						pHitPlayer = pTempHitPlayer;
						goto FINDATTACK1;
					}
				}
			}
		}
	}

FINDATTACK1:

	// 공격 범위 내에 플레이어가 없다.
	if (pHitPlayer == NULL)
		return true;

	_LOG(0, L"#Attack1 Direction:%d / SessionID:%d -> SessionID:%d", byDirection, pSession->dwSessionID, pHitPlayer->_SessionID);

	//---------------------------------------------------------------------------------------------------------------
	// HP를 깍고, 
	// 피격자 기준으로 섹터를 계산하여 메시지를 보낸다.
#ifdef DISCONNECT_SERVER
	pHitPlayer->_PreHP = pHitPlayer->_HP;
#endif // DISCONNECT_SERVER
	pHitPlayer->_HP -= dfATTACK1_DAMAGE;
	mpDamge(pPacket, pSession->dwSessionID, pHitPlayer->_SessionID, pHitPlayer->_HP);
	C_Field::GetInstance()->GetSectorAround(pHitPlayer->_CurSector->iX, pHitPlayer->_CurSector->iY, &st_Sector_Around);
	C_Field::GetInstance()->SendPacket_Around(pSession, pPacket, &st_Sector_Around, true);
	pPacket->Clear();


	return true;
}
bool netPacketProc_Attack2(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	// 공격 하는 순간 판정까지 떨어진다. 
	char byDirection;
	short shX;
	short shY;
	st_PLAYER* pHitPlayer;
	st_PLAYER* pTempHitPlayer;
	st_SECTOR_AROUND st_Sector_Around;
	CList<st_PLAYER*>* pCList;
	CList<st_PLAYER*>::iterator iter;
	int iCnt;

	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	pPacket->Clear();

	_LOG(dfLOG_LEVEL_DEBUG, L"# Attack2 # SessionID:%d / Direction:%d / X:%d / Y:%d", pSession->dwSessionID, byDirection, shX, shY);
	
	// 공격 모션에 대한 패킷은 해당 플레이어가 보이는 모든 세션에게 보내야 한다. 
	mpAttack2(pPacket, pSession->dwSessionID, byDirection, shX, shY);
	C_Field::GetInstance()->GetSectorAround(shX / dfGRID_X_SIZE, shY / dfGRID_Y_SIZE, &st_Sector_Around);
	C_Field::GetInstance()->SendPacket_Around(pSession, pPacket, &st_Sector_Around, true);
	pPacket->Clear();

	//---------------------------------------------------------------------------------------------------------------
	// 데미지에 대한 처리
	// 
	// 바라보는 방향을 기준으로 0 ~ _dfATTACK1_RANGE_X 범위의 섹터를 계산
	// 공격 범위 또한 계산해야 한다. 
	// 
	// 1. 같은 섹터에서 검색한다. 
	// 2. 섹터를 넘어간다면 다른 섹터에서도 계산한다.
	// 3. 3개의 섹터에 걸치는 경우도 있다.
	// 4. 공격 범위에 있는 섹터를 구했다면, 가장 가까이 있는 플레이어를 구한다.
	// 5. 해당 플레이어의 HP를 깍고
	// 6. 피격자 기준으로 섹터를 계산하여 메시지를 보낸다. 
	//---------------------------------------------------------------------------------------------------------------
	pHitPlayer = NULL;
	C_Field::GetInstance()->GetAttackSectorAround(shX, shY, byDirection, dfATTACK2_RANGE_X, dfATTACK2_RANGE_Y, &st_Sector_Around);
	for (iCnt = 0; iCnt < st_Sector_Around.iCount; ++iCnt)
	{
		pCList = C_Field::GetInstance()->GetPlayerInSectorCList(st_Sector_Around.Around[iCnt].iX, st_Sector_Around.Around[iCnt].iY);

		for (iter = (*pCList).begin(); iter != (*pCList).end(); ++iter)
		{
			pTempHitPlayer = (*iter);

			// 타격자와 피격자가 같으면 넘어간다. 
			if (pSession->dwSessionID == pTempHitPlayer->_SessionID) continue;

			// Y좌표 공격 범위에 들어갔는지 체크
			if (shY - dfATTACK2_RANGE_Y < pTempHitPlayer->_Y && pTempHitPlayer->_Y < shY + dfATTACK2_RANGE_Y)
			{
				// X좌표 공격 범위를 체크한다. 
				if (byDirection == dfPACKET_MOVE_DIR_LL)
				{
					if (shX - dfATTACK2_RANGE_X <= pTempHitPlayer->_X && pTempHitPlayer->_X <= shX)
					{
						pHitPlayer = pTempHitPlayer;
						goto FINDATTACK2;
					}
				}
				else
				{
					if (shX <= pTempHitPlayer->_X && pTempHitPlayer->_X <= shX + dfATTACK2_RANGE_X)
					{
						pHitPlayer = pTempHitPlayer;
						goto FINDATTACK2;
					}
				}
			}

		}
	}

FINDATTACK2:
	// 공격 범위 내에 플레이어가 없다.
	if (pHitPlayer == NULL)
		return true;

	_LOG(0, L"#Attack2 Direction:%d / SessionID:%d -> SessionID:%d", byDirection, pSession->dwSessionID, pHitPlayer->_SessionID);

	//---------------------------------------------------------------------------------------------------------------
	// HP를 깍고, 
	// 피격자 기준으로 섹터를 계산하여 메시지를 보낸다.
#ifdef DISCONNECT_SERVER
	pHitPlayer->_PreHP = pHitPlayer->_HP;
#endif // DISCONNECT_SERVE
	pHitPlayer->_HP -= dfATTACK2_DAMAGE;
	mpDamge(pPacket, pSession->dwSessionID, pHitPlayer->_SessionID, pHitPlayer->_HP);
	C_Field::GetInstance()->GetSectorAround(pHitPlayer->_CurSector->iX, pHitPlayer->_CurSector->iY, &st_Sector_Around);
	C_Field::GetInstance()->SendPacket_Around(pSession, pPacket, &st_Sector_Around, true);
	pPacket->Clear();


	return true;
}
bool netPacketProc_Attack3(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	// 공격 하는 순간 판정까지 떨어진다. 
	char byDirection;
	short shX;
	short shY;
	st_PLAYER* pHitPlayer;
	st_PLAYER* pTempHitPlayer;
	st_SECTOR_AROUND st_Sector_Around;
	CList<st_PLAYER*>* pCList;
	CList<st_PLAYER*>::iterator iter;
	int iCnt;

	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	pPacket->Clear();

	_LOG(dfLOG_LEVEL_DEBUG, L"# Attack3 # SessionID:%d / Direction:%d / X:%d / Y:%d", pSession->dwSessionID, byDirection, shX, shY);

	// 공격 모션에 대한 패킷은 해당 플레이어가 보이는 모든 세션에게 보내야 한다. 
	mpAttack3(pPacket, pSession->dwSessionID, byDirection, shX, shY);
	C_Field::GetInstance()->GetSectorAround(shX / dfGRID_X_SIZE, shY / dfGRID_Y_SIZE, &st_Sector_Around);
	C_Field::GetInstance()->SendPacket_Around(pSession, pPacket, &st_Sector_Around, true);
	pPacket->Clear();

	//---------------------------------------------------------------------------------------------------------------
	// 데미지에 대한 처리
	// 
	// 바라보는 방향을 기준으로 0 ~ _dfATTACK1_RANGE_X 범위의 섹터를 계산
	// 공격 범위 또한 계산해야 한다. 
	// 
	// 1. 같은 섹터에서 검색한다. 
	// 2. 섹터를 넘어간다면 다른 섹터에서도 계산한다.
	// 3. 3개의 섹터에 걸치는 경우도 있다.
	// 4. 공격 범위에 있는 섹터를 구했다면, 가장 가까이 있는 플레이어를 구한다.
	// 5. 해당 플레이어의 HP를 깍고
	// 6. 피격자 기준으로 섹터를 계산하여 메시지를 보낸다. 
	//---------------------------------------------------------------------------------------------------------------
	pHitPlayer = NULL;
	C_Field::GetInstance()->GetAttackSectorAround(shX, shY, byDirection, dfATTACK3_RANGE_X, dfATTACK3_RANGE_Y, &st_Sector_Around);
	for (iCnt = 0; iCnt < st_Sector_Around.iCount; ++iCnt)
	{
		pCList = C_Field::GetInstance()->GetPlayerInSectorCList(st_Sector_Around.Around[iCnt].iX, st_Sector_Around.Around[iCnt].iY);

		for (iter = (*pCList).begin(); iter != (*pCList).end(); ++iter)
		{
			pTempHitPlayer = (*iter);

			// 타격자와 피격자가 같으면 넘어간다. 
			if (pSession->dwSessionID == pTempHitPlayer->_SessionID) continue;

			// Y좌표 공격 범위에 들어갔는지 체크
			if (shY - dfATTACK3_RANGE_Y < pTempHitPlayer->_Y && pTempHitPlayer->_Y < shY + dfATTACK3_RANGE_Y)
			{
				// X좌표 공격 범위를 체크한다. 
				if (byDirection == dfPACKET_MOVE_DIR_LL)
				{
					if (shX - dfATTACK3_RANGE_X <= pTempHitPlayer->_X && pTempHitPlayer->_X <= shX)
					{
						pHitPlayer = pTempHitPlayer;
						goto FINDATTACK3;
					}
				}
				else
				{
					if (shX <= pTempHitPlayer->_X && pTempHitPlayer->_X <= shX + dfATTACK3_RANGE_X)
					{
						pHitPlayer = pTempHitPlayer;
						goto FINDATTACK3;
					}
				}
			}
		}
	}

FINDATTACK3:
	// 공격 범위 내에 플레이어가 없다.
	if (pHitPlayer == NULL)
		return true;

	_LOG(0, L"#Attack3 Direction:%d / SessionID:%d -> SessionID:%d", byDirection, pSession->dwSessionID, pHitPlayer->_SessionID);

	//---------------------------------------------------------------------------------------------------------------
	// HP를 깍고, 
	// 피격자 기준으로 섹터를 계산하여 메시지를 보낸다.
#ifdef DISCONNECT_SERVER
	pHitPlayer->_PreHP = pHitPlayer->_HP;
#endif // DISCONNECT_SERVE
	pHitPlayer->_HP -= dfATTACK3_DAMAGE;
	mpDamge(pPacket, pSession->dwSessionID, pHitPlayer->_SessionID, pHitPlayer->_HP);
	C_Field::GetInstance()->GetSectorAround(pHitPlayer->_CurSector->iX, pHitPlayer->_CurSector->iY, &st_Sector_Around);
	C_Field::GetInstance()->SendPacket_Around(pSession, pPacket, &st_Sector_Around, true);
	pPacket->Clear();


	return true;
}
bool netPacketProc_Echo(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	DWORD Time;

	*pPacket >> Time;
	pPacket->Clear();

	 // _LOG(0, L"# ECHO # SessionID:%d / Time:%d", pSession->dwSessionID, Time);

	//---------------------------------------------------------------------------------------------------------------
	// 에코 패킷이 오면 받아서 바로 해당 유저에게 전달해준다.
	//---------------------------------------------------------------------------------------------------------------
	mpEcho(pPacket, Time);

	SendPacket_Unicast(pSession, pPacket);
	pPacket->Clear();

	return true;
}




void mpSync(SerializeBuffer* pPacket, DWORD dwSessionID, short shX, short shY)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 8;
	New_Header.byType = (char)dfPACKET_SC_SYNC;
	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << dwSessionID << shX << shY;
}
void mpMoveStart(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 9;
	New_Header.byType = (char)dfPACKET_SC_MOVE_START;

	// 사용 후 초기화가 패턴이다.
	//pPacket->Clear();

	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << dwSessionID << byDirection << shX << shY;
}
void mpMoveStop(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 9;
	New_Header.byType = (char)dfPACKET_SC_MOVE_STOP;

	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << dwSessionID << byDirection << shX << shY;
}
void mpCreateMyCharacter(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY, char HP)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 10;
	New_Header.byType = (char)dfPACKET_SC_CREATE_MY_CHARACTER;

	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));

	(*pPacket) << dwSessionID << byDirection << shX << shY << HP;
}
void mpCreateOtherCharacter(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY, char HP)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 10;
	New_Header.byType = (char)dfPACKET_SC_CREATE_OTHER_CHARACTER;

	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));

	(*pPacket) << dwSessionID << byDirection << shX << shY << HP;
}
void mpEcho(SerializeBuffer* pPacket, DWORD Time)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 4;
	New_Header.byType = (char)dfPACKET_SC_ECHO;

	pPacket->PutData((char*)&New_Header, sizeof(st_PACKET_HEADER));
	(*pPacket) << Time;
}
void mpAttack1(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 9;
	New_Header.byType = (char)dfPAKCET_SC_ATTACK1;

	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << dwSessionID << byDirection << shX << shY;
}
void mpAttack2(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 9;
	New_Header.byType = (char)dfPAKCET_SC_ATTACK2;

	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << dwSessionID << byDirection << shX << shY;
}
void mpAttack3(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 9;
	New_Header.byType = (char)dfPAKCET_SC_ATTACK3;

	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << dwSessionID << byDirection << shX << shY;
}
void mpDamge(SerializeBuffer* pPacket, DWORD dwAttackID, DWORD dwDamageID, char DamageHP)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 9;
	New_Header.byType = (char)dfPACKET_SC_DAMAGE;

	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << dwAttackID << dwDamageID << DamageHP;
}
void mpDeleteCharacter(SerializeBuffer* pPacket, DWORD dwSessionID)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 4;
	New_Header.byType = (char)dfPACKET_SC_DELETE_CHATACTER;
	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << dwSessionID;
}








//C_Session C_Session::_C_Session;
void ForwardDecl(int DestID, SerializeBuffer* sb)
{
	//st_SESSION* pSession = C_Session::GetInstance()->_Session_Hash.find(DestID)->second;

	//C_Session::GetInstance()->SendPacket_Unicast(pSession, sb);
}
#if C_SESSION == 1
C_Session* C_Session::GetInstance(void)
{
	//return &_C_Session;
	return NULL;
}

void C_Session::netIOProcess(void)
{
	std::unordered_map<DWORD, st_SESSION*>::iterator	iter;
	std::unordered_map<DWORD, st_SESSION*>::iterator	iter_FD_ISSET;
	st_SESSION* st_pSession;
	SOCKET Listen_Socket;
	FD_SET ReadSet;	
	FD_SET WriteSet;
	timeval Time;

	int iCnt;
	int i_Result;

	Listen_Socket = _Listen_Socket;
	iter = _Session_Hash.begin();
	//--------------------------------------------------------------------------------------------------------------------
	// _Session 전부를 Select에 등록하면 while문을 종료한다. 
	// 
	//--------------------------------------------------------------------------------------------------------------------
	while (iter != _Session_Hash.end())
	{
		FD_ZERO(&ReadSet);
		FD_ZERO(&WriteSet);

		//------------------------------------------
		// Listen_Socket 넣기
		//------------------------------------------
		FD_SET(Listen_Socket, &ReadSet);

		//------------------------------------------
		// 리슨소켓 및 접속중인 모든 클라이언트에 대해 SOCKET 을 체크한다. 
		// 
		// 마지막 _Session이거나 or _Listen_Socket 포함 64개의 소켓을 Select에 등록했다면, 반복문을 종료한다. 
		//------------------------------------------
		for (iCnt = 0; iCnt < 64 - 1 && iter != _Session_Hash.end(); ++iter, ++iCnt)
		{
			st_pSession = iter->second;

			if (st_pSession->Disconnect) continue;
			
			//------------------------------------------
			// 해당 클라이언트 Read Set 등록 / SendQ 에 데이터가 있다면 Write Set 등록
			//------------------------------------------
			FD_SET(st_pSession->Socket, &ReadSet);
			if (st_pSession->SendQ->GetUseSize() > 0)
				FD_SET(st_pSession->Socket, &WriteSet);
		}

		//------------------------------------------
		// NULL: 한개라도 응답 올 때까지 무한히 기다림
		// 0,0: 응답올때까지 기다리지 않고 바로 끝낸다.
		// 양수: 한개라도 응답하거나, 지정된 시간이 지나면 리턴
		//------------------------------------------
		Time.tv_sec = 0;
		Time.tv_usec = 0;

		i_Result = select(0, &ReadSet, &WriteSet, 0, &Time);

		if (i_Result == SOCKET_ERROR)
		{
			_LOG(0, L"select failed with error: %ld \n", WSAGetLastError());
			__debugbreak();
		}


		//------------------------------------------
		// 리턴값이 0 이상이라면 누군가의 데이터가 왔다.
		//------------------------------------------
		while (i_Result > 0)
		{
			if (FD_ISSET(Listen_Socket, &ReadSet))
			{
				netProc_Accept();
				--i_Result;
			}

			//------------------------------------------
			// 전체 세션중 어떤 세션이 반응을 보였는지 다시 확인한다. 
			//------------------------------------------
			for (iter_FD_ISSET = _Session_Hash.begin(); iter_FD_ISSET != _Session_Hash.end(); ++iter_FD_ISSET)
			{
				st_pSession = (*iter_FD_ISSET).second;

				if (FD_ISSET(st_pSession->Socket, &ReadSet))
				{
					netProc_Recv(st_pSession);
					--i_Result;
				}

				if (FD_ISSET(st_pSession->Socket, &WriteSet))
				{
					netProc_Send(st_pSession);
					--i_Result;
				}
			}
		}
	}

	
}

void C_Session::netProc_Accept(void)
{
	//---------------------------------------------------------------------------------
	// 새로운 연결에 대하여 accept를 하고,
	// SessionID를 할당하고, 
	// 내 눈에 보이는 시야에 한정하여 캐릭터 생성을 보낸다. 
	//---------------------------------------------------------------------------------

	int Error;

	SOCKET New_Client_Socket;
	sockaddr_in Clinet_Addr;
	
	st_PACKET_HEADER st_New_Header_MY_CHAPACTER;

	st_PACKET_HEADER st_New_Header_OTHER_CHAPACTER;
	SerializeBuffer New_Packet_OTHER_CHAPACTER;
	st_SECTOR_AROUND st_Sector_Around;

	st_SESSION* st_p_New_Session;
	st_PLAYER* st_p_New_Player;
	int Client_Addr_Len;

	Client_Addr_Len = sizeof(Clinet_Addr);
	New_Client_Socket = accept(_Listen_Socket, (sockaddr*)&Clinet_Addr, &Client_Addr_Len);
	if (New_Client_Socket == INVALID_SOCKET)
	{
		Error = WSAGetLastError();

		if (Error == WSAEWOULDBLOCK)
		{
			// Seletc로 거르고 들어왔는데 WSAEWOULDBLOCK이 나오는지 모르겠다. 
			_LOG(1, L"accept failed with error: %ld \n", Error);
			//__debugbreak();
			// 원래는 에러로 처리하면 안된다. 
			return;
		}
		else
		{
			_LOG(0, L"accept failed with error: %ld \n", Error);
			WSACleanup();
			__debugbreak();
		}
	}

	
	// 접속자 인원수 제한. 순간적인 피크를 포함하여 약 8000명으로 가정한다. 
	if (_Session_Hash.size() > 8000)
	{
		_LOG(2, L"Full Server cannot accept!!! \n");
		closesocket(New_Client_Socket);
		return;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. 새로운 세션 만들기.
	// 2. 새로운 플레이어 만들기. 
	// 3. 나에게 생성 메시지 보내기.
	// 4. 주변에 나에대한 생성 메시지를 보낸다. 
	// 5. 행동중인 플레이어가 있다면 동작을 이어서 보여준다. 
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1.
	// st_p_New_Session = new st_SESSION(New_Client_Socket, ++_SessionID);
	st_p_New_Session = st_SESSION_MemPool.Alloc();
	st_p_New_Session->Init(New_Client_Socket, ++_SessionID);
	_Session_Hash.insert({ _SessionID, st_p_New_Session });
	// 2.
	st_p_New_Player = CreateNewPlayer(_SessionID, st_p_New_Session);
	// 3.
	st_New_Header_MY_CHAPACTER.byCode = (char)dfPACKET_CODE;
	st_New_Header_MY_CHAPACTER.bySize = 10;
	st_New_Header_MY_CHAPACTER.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	g_Packet.PutData((char*)&st_New_Header_MY_CHAPACTER, sizeof(st_PACKET_HEADER));
	g_Packet << _SessionID << st_p_New_Player->_byDirection << st_p_New_Player->_X << st_p_New_Player->_Y << st_p_New_Player->_HP;
	SendPacket_Unicast(st_p_New_Session, &g_Packet);
	g_Packet.Clear();
	// 4.
	st_New_Header_OTHER_CHAPACTER.byCode = (char)dfPACKET_CODE;
	st_New_Header_OTHER_CHAPACTER.bySize = 10;
	st_New_Header_OTHER_CHAPACTER.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	g_Packet.PutData((char*)&st_New_Header_OTHER_CHAPACTER, sizeof(st_PACKET_HEADER));
	g_Packet << _SessionID << st_p_New_Player->_byDirection << st_p_New_Player->_X << st_p_New_Player->_Y << st_p_New_Player->_HP;
	C_Field::GetInstance()->GetSectorAround(st_p_New_Player->_CurSector->iX, st_p_New_Player->_CurSector->iY, &st_Sector_Around);
	SendPacket_Around(st_p_New_Session, &New_Packet_OTHER_CHAPACTER, &st_Sector_Around);
	// 5. 


	
}

void C_Session::netProc_Send(st_SESSION* pSession)
{
	// Send의 경우 한번에 링버퍼에 있는 모든 메시지를 전송하면 된다. 
	// field를 기준으로 전송할 대상을 판단해야 한다. 
	// 걍 Send 호출

	int Send_Size;
	int err;

	while (1)
	{
		if (pSession->SendQ->GetUseSize() == 0)
			break;
		
		Send_Size = send(pSession->Socket, pSession->SendQ->GetFrontBufferPtr(), (int)pSession->SendQ->DirectDequeueSize(), 0);

		if (Send_Size == SOCKET_ERROR)
		{
			err = WSAGetLastError();

			if (err == WSAEWOULDBLOCK)
			{
				// 비동기 Send에서 WSAEWOULDBLOCK 가 나오는 경우: 소켓 송신 버퍼가 가득차서 즉시 보낼 수 없을 때
				// 그렇다면 그냥 다음 프레임에 전송하면 된다. 

				break;
			}
			else if (err == 10054)
			{
				// 현재 연결은 원격 호스트에 의해 강제로 끊겼다.

				// PushDisconnectList(pSession);
				break;
			}
			else if (err == 10053)
			{
				// 소프트웨어로 인해 연결이 중단되었다. 


				// PushDisconnectList(pSession);
				break;
			}

			// 에러로그 찍기
			__debugbreak();
		}

		pSession->SendQ->MoveFront(Send_Size);
	}
}

void C_Session::netProc_Recv(st_SESSION* pSession)
{
	
}

void C_Session::SendPacket_SectorOne(int iSectorX, int iSectorY, SerializeBuffer* pPacket, st_SESSION* pExceptSession)
{
	//std::list<st_PLAYER*> *pTemp_Player_List;
	//std::list<st_PLAYER*>::iterator iter;

	//if(iSectorX< dfRANGE_MOVE_LEFT || dfRANGE_MO)

	//pTemp_Player_List = C_Field::GetInstance()->_Sector[iSectorY][iSectorX];

	//if (pExceptSession == NULL)
	//{
	//	for (iter = pTemp_Player_List->begin(); iter != pTemp_Player_List->end(); ++iter)
	//	{
	//		SendPacket_Unicast((*iter)->_pSession, pPacket);
	//	}
	//}
	//else
	//{
	//	for (iter = pTemp_Player_List->begin(); iter != pTemp_Player_List->end(); ++iter)
	//	{
	//		if (pExceptSession == (*iter)->_pSession)
	//			continue;
	//		SendPacket_Unicast((*iter)->_pSession, pPacket);
	//	}
	//}
}

void C_Session::SendPacket_Unicast(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	// 세션에서 SendQ가 가득 차있다면 연결을 끊는다.

	size_t Ret_Packet;

	Ret_Packet = pSession->SendQ->Enqueue(pPacket->GetBufferPtr(), pPacket->GetDataSize());

	if (Ret_Packet == 0)
	{
		// 연결을 끊는다. 
		return;
	}
}

void C_Session::SendPacket_Around(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around, bool bSendMe)
{
	int iCnt;

	if (bSendMe == false)
	{
		for (iCnt = 0; iCnt < pSector_Around->iCount; ++iCnt)
		{
			SendPacket_SectorOne(pSector_Around->Around[iCnt].iX, pSector_Around->Around[iCnt].iY, pPacket, pSession);
		}
	}
	else
	{
		for (iCnt = 0; iCnt < pSector_Around->iCount; ++iCnt)
		{
			SendPacket_SectorOne(pSector_Around->Around[iCnt].iX, pSector_Around->Around[iCnt].iY, pPacket, NULL);
		}
	}
}

void C_Session::SendPacket_Broadcast(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	return;
}

bool C_Session::PacketProc(st_SESSION* pSession, unsigned char byPacketType, SerializeBuffer* pPacket)
{
	switch (byPacketType)
	{
	case dfPACKET_CS_MOVE_START:
		break;
	case dfPACKET_CS_MOVE_STOP:
		break;
	case dfPACKET_CS_ATTACK1:
		break;
	case dfPACKET_CS_ATTACK2:
		break;
	case dfPACKET_CS_ATTACK3:
		break;
	}

	return true;
}

bool C_Session::netPacketProc_MoveStart(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	char byDirection;
	short X;
	short Y;
	st_PLAYER* pPlayer;
	st_PACKET_HEADER New_Header;
	st_SECTOR_AROUND st_Sector_Around;

	*pPacket >> byDirection;
	*pPacket >> X;
	*pPacket >> Y;
	pPacket->Clear();

	_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTART # SessionID:%d / Direction:%d / X:%d / Y:%d", pSession->dwSessionID, byDirection, X, Y);

	//---------------------------------------------------------------------------------------------------------------
	// ID로 캐릭터를 검색한다. 
	//---------------------------------------------------------------------------------------------------------------
	pPlayer = g_CharacterHash.find(pSession->dwSessionID)->second;
	if (pPlayer == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTART > SessionID:%d Player Not Found!", pSession->dwSessionID);
		return false;
	}

	//---------------------------------------------------------------------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 싱크 패킷을 보내어 좌표 보정.
	// 
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로 
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음. 
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함
	// 지금은 좌표에 대해서는 간단한 구현을 목적으로 하고 있으므로 
	// 서버는 클라이언트의 좌표를 그대로 믿지만, 서버와 너무 큰 차이가 있다면 강제 좌표 동기화 하도록 함
	//---------------------------------------------------------------------------------------------------------------
	if (abs(pPlayer->_X - X) > dfERROR_RANGE || abs(pPlayer->_Y - Y) > dfERROR_RANGE)
	{
		New_Header.byCode = (char)dfPACKET_CODE;
		New_Header.bySize = 8;
		New_Header.byType = (char)dfPACKET_SC_SYNC;
		(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
		(*pPacket) << pPlayer->_SessionID << pPlayer->_X << pPlayer->_Y;
		C_Field::GetInstance()->GetSectorAround(pPlayer->_CurSector->iX, pPlayer->_CurSector->iY, &st_Sector_Around);
		SendPacket_Around(pSession, pPacket, &st_Sector_Around, true);
		pPacket->Clear();
	}

	//---------------------------------------------------------------------------------------------------------------
	// 동작을 변경. 동작번호와, 방향값이 같다.
	//---------------------------------------------------------------------------------------------------------------
	pPlayer->_dwAction = byDirection;

	//---------------------------------------------------------------------------------------------------------------
	// 방향을 변경
	//---------------------------------------------------------------------------------------------------------------
	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pPlayer->_byDirection = dfPACKET_MOVE_DIR_LL;
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RD:
		pPlayer->_byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	default:
		break;
	}
	pPlayer->_X = X;
	pPlayer->_Y = Y;

	//---------------------------------------------------------------------------------------------------------------
	// 섹터 처리
	// 정지를 하면서 좌표가 약간 조절된 경우 섹터 업데이트를 함
	// 위에서 좌표를 변경하였으면 섹터도 다시 재설정 해야한다. 
	//---------------------------------------------------------------------------------------------------------------
	if(C_Field::GetInstance()->Sector_UpdateCharacter(pPlayer))
	{
		//---------------------------------------------------------------------------------------------------------------
		// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다. 
		//---------------------------------------------------------------------------------------------------------------
		C_Field::GetInstance()->CharacterSectorUpdatePacket(pPlayer);
		
	}
	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 9;
	New_Header.byType = (char)dfPACKET_SC_MOVE_START;
	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << pPlayer->_SessionID << byDirection << pPlayer->_X << pPlayer->_Y;
	C_Field::GetInstance()->GetSectorAround(pPlayer->_CurSector->iX, pPlayer->_CurSector->iY, &st_Sector_Around);
	SendPacket_Around(pSession, pPacket, &st_Sector_Around);
	pPacket->Clear();

	return true;
}

bool C_Session::netPacketProc_MoveStop(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	return false;
}

bool C_Session::netPacketProc_Attack1(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	return false;
}

bool C_Session::netPacketProc_Attack2(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	return false;
}

bool C_Session::netPacketProc_Attack3(st_SESSION* pSession, SerializeBuffer* pPacket)
{
	return false;
}

C_Session::C_Session(void) : _SessionID(0)
{
	struct addrinfo hints;
	struct addrinfo* result = NULL;
	SOCKET Temp_Listen_Socket = _Listen_Socket;
	char Port[6];
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
	errno_t Ret_itoa_s;


	Temp_Listen_Socket = _Listen_Socket;


	//---------------------------------------------------
	// Initialize Winsock
	Ret_WSAStartup = WSAStartup(MAKEWORD(2, 2), &_WsaData);
	if (Ret_WSAStartup != 0)
	{
		_LOG(0, L"WSAStartup failed with error: %d", Ret_WSAStartup);
		__debugbreak();
	}
	_LOG(0, L"WSAStartup # \n");

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
	// 아래의 Port 코드는 C6054 : 'Port' 문자열이 0으로 종료되지 않을 수 있습니다. 경고를 해결하기 위하여 작성하였다. 
	Ret_itoa_s = _itoa_s(dfNETWORK_PORT, Port, sizeof(Port), 10);
	if (Ret_itoa_s != NULL)
	{
		_LOG(0, L"_itoa_s failed with error");
		WSACleanup();
		__debugbreak();
	}
	Port[sizeof(Port) - 1] = '\0';
	Ret_getaddrinfo = getaddrinfo(NULL, Port, &hints, &result);
	if (Ret_getaddrinfo != 0)
	{
		_LOG(0, L"getaddrinfo failed with error: %d", Ret_getaddrinfo);
		WSACleanup();
		__debugbreak();
	}

	//---------------------------------------------------
	// Create a SOCKET for the server to listen for client connections.
	// 서버가 클라이언트 연결을 수신을 SOCKET을 만든다.
	Temp_Listen_Socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (Temp_Listen_Socket == INVALID_SOCKET)
	{
		_LOG(0, L"socket failed with error : %ld", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		__debugbreak();
	}

	//---------------------------------------------------
	// setsockopt_Linger
	Linger_Opt.l_onoff = 1;
	Linger_Opt.l_linger = 0;
	Ret_setsockopt = setsockopt(Temp_Listen_Socket, SOL_SOCKET, SO_LINGER, (char*)&Linger_Opt, sizeof(Linger_Opt));
	if (Ret_setsockopt == SOCKET_ERROR)
	{
		_LOG(0, L"setsockopt failed with error: %ld", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}

	//---------------------------------------------------
	// Setup the TCP listening socket
	// TCP listening socket 을 준비한다.
	Ret_bind = bind(Temp_Listen_Socket, result->ai_addr, (int)result->ai_addrlen);
	if (Ret_bind == SOCKET_ERROR)
	{
		_LOG(0, L"bind failed with error: %d", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	_LOG(0, L"Bind OK # Port: %d", dfNETWORK_PORT);

	freeaddrinfo(result);

	//---------------------------------------------------
	// Listen a client socket
	Ret_listen = listen(Temp_Listen_Socket, SOMAXCONN);
	//i_Result = listen(Temp_Listen_Socket, SOMAXCONN_HINT(65535));	// 양수 ~200 -> 기본 200개, 200~ 갯수 적용,    
	if (Ret_listen == SOCKET_ERROR)
	{
		_LOG(0, L"listen failed with error: %d", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	wprintf_s(L"Listen OK # \n");

	//---------------------------------------------------
	// 넌블로킹 소켓으로 전환
	on = 1;
	Ret_ioctlsocket = ioctlsocket(Temp_Listen_Socket, FIONBIO, &on);
	if (Ret_ioctlsocket == SOCKET_ERROR)
	{
		_LOG(0, L"ioctlsocket failed with error: %ld", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}

	_Listen_Socket = Temp_Listen_Socket;
}

C_Session::~C_Session(void)
{
	closesocket(_Listen_Socket);
	_LOG(0, L"Close Listen_Socket # \n");

	WSACleanup();
	_LOG(0, L"WSACleanup # \n");

	// _Session_Map 순회를 돌면서 삭제해야 하는데,,,, 삭제 구조가 꼬이기 시작했다.
	// 아마도 할당한 쪽에서 데이터에 대한 삭제를 마무리 하는게 좋아 보인다. 
	// 여기서는 여기 클래스에서 생성된 것들만 정리하고 지워준다. 
	// Socket, RecvQ, SendQ
	std::unordered_map<DWORD, st_SESSION*>::iterator iter;
	st_SESSION* st_Temp_Session;
	for (iter = _Session_Hash.begin(); iter != _Session_Hash.end();)
	{
		st_Temp_Session = iter->second;
		
		closesocket(st_Temp_Session->Socket);
		delete st_Temp_Session->RecvQ;
		delete st_Temp_Session->SendQ;
		delete st_Temp_Session;

		iter = _Session_Hash.erase(iter);
	}
	_LOG(0, L"CleanUp Session_Hash # \n");
}

#endif
