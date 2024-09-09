#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")

#include <process.h>
#include <tchar.h>

#include "CLanServer.h"

#define dfSENDPOST_RET_FLAG_FALSE 0
#define dfSENDPOST_RET_SUCCESS 1
#define dfSENDPOST_RET_SENDQ_USESIZE_ZERO 2

unsigned __stdcall WorkerThreadProc(void* param);
unsigned __stdcall AcceptThreadProc(void* param);

bool CLanServer::Start(st_SETTINGS* setting)
{
	InitLogger(L"LOG");

	//-----------------------------------------------------
	// 서버 세팅
	InitServer(setting);

	//-----------------------------------------------------
	// index 마스크 초기화
	InitSessionIndexMask();

	//-----------------------------------------------------
	// 삭제될 인텍스 저장할 스택 초기화
	InitDeleteIndexStack();

	//-----------------------------------------------------
	// 네트워크 초기화
	InitNetwork();


	//-----------------------------------------------------
	// 워커 스레드 생성
	CreateWorkerThread();


	//-----------------------------------------------------
	// Accept 스레드 생성
	CreateAcceptThread();

	return true;
}

void CLanServer::Stop(void)
{
}

int CLanServer::GetSessionCount(void)
{
	return 0;
}

bool CLanServer::Disconnect(DWORD64 SessionID)
{
	st_SESSION* pSession;

	pSession = AcquireSession(SessionID);
	if (pSession == NULL)
		return false;

	CancelIoEx((HANDLE)pSession->socket, NULL);

	ReleaseSession(pSession);

	return true;
}

bool CLanServer::SendPacket(DWORD64 SessionID, Packet* pPacket)
{
	//---------------------------------------------------
	// variable
	st_SESSION* pSession;

	//---------------------------------------------------
	// return variable
	size_t Ret_Eq_Size;


	pSession = AcquireSession(SessionID);
	if (pSession == NULL)
	{
		return false;
	}

	// 헤더 넣기
	pPacket->PutHeader();
	if (pPacket->GetPayloadSize() != 8)
		__debugbreak();
	if (pPacket->GetHeaderSize() != 2)
		__debugbreak();

	// &pPacket를 하는 이유: pPacket에 객체의 주소가 데이터로 들어있음.
	Ret_Eq_Size = pSession->SendQ.Enqueue((char*)&pPacket, sizeof(pPacket));
	if(Ret_Eq_Size != sizeof(pPacket))
	{
		// 센드큐가 꽉찼거나, Enqueue가 문제 있음. 
		LOG(L"Session", LEVEL_ERROR, false, true, L"CLanServer::SendPacket() failed Eq_Size:%d", Ret_Eq_Size);
		LOG(L"Session", LEVEL_ERROR, false, true, L"CLanServer::SendPacket() failed SendQ.GetFreeSize():%d", pSession->SendQ.GetFreeSize());
		RemoveSession(pSession);
		__debugbreak();
	}

	// 링버퍼에 데이터 직접 넣음 
	//pSession->SendQ.Enqueue(pPacket->GetBufferHeaderBeginPtr(), pPacket->GetUseSize());

	SendPost(pSession);

	ReleaseSession(pSession);

	return true;
}





CLanServer::CLanServer() : _AcceptTPS(0), _RecvMessageTPS(0), _SendMessageTPS(0),
_sessionCount(0), _sessionAccepted(0), _acceptPerSecondCounter(0),
_hAccept(NULL), _hWorkers(NULL), _handleIOCP(NULL), _listenSocket(INVALID_SOCKET),
_timeBegin(0)

, _DebugCount(0)
{
	InitializeSRWLock(&_srw_releaseIndexStack);


	timeBeginPeriod(1);
}

CLanServer::~CLanServer()
{
	timeEndPeriod(1);
}




st_SESSION* CLanServer::CreateSession(SOCKET socket, DWORD64 SessionID, sockaddr_in Addr)
{
	//---------------------------------------------------
	// variable
	st_SESSION* pSession;
	DWORD clientAddressStringLen;
	short arrayIndex;

	pSession = NULL;
	clientAddressStringLen = 0;

	AcquireSRWLockExclusive(&_srw_releaseIndexStack);
	_releaseIndexStack.Pop(arrayIndex);
	ReleaseSRWLockExclusive(&_srw_releaseIndexStack);

	pSession = _sessionArray + arrayIndex;


	pSession->sessionID.parts.id = SessionID;
	pSession->sessionID.parts.index = arrayIndex;


	return pSession;
}

void CLanServer::InitServer(st_SETTINGS* setting)
{
	if (setting != NULL)
		memcpy(&_st_Setting, setting, sizeof(st_SETTINGS));

	SYSTEM_INFO systemInfo;


	GetSystemInfo(&systemInfo);
	if (_st_Setting.workerThreadTotal == 0)
	{
		_st_Setting.workerThreadTotal = systemInfo.dwNumberOfProcessors * 2;
	}

	if (_st_Setting.workerThreadRunning == 0)
	{
		_st_Setting.workerThreadRunning = systemInfo.dwNumberOfProcessors;
	}

}

void CLanServer::InitDeleteIndexStack(void)
{
	int iCnt;

	_releaseIndexStack.Init(sizeof(short), _st_Setting.sessionCountMax);

	for (iCnt = _st_Setting.sessionCountMax; iCnt > 0; --iCnt)
	{
		_releaseIndexStack.Push(&iCnt);
	}
}


void CLanServer::InitNetwork(void)
{
	//---------------------------------------------------
	// variable
#ifdef __CLIENT
	struct addrinfo hints;
	char Port[6];
	struct addrinfo* result;
#endif // __CLIENT
	WSADATA wsa;
	SOCKET Temp_Listen_Socket;
	SOCKADDR_IN serverAddr;
	int serverAddrLen;
	linger Linger_Opt;


	//---------------------------------------------------
	// return variable
	int Ret_WSAStartup;
#ifdef __CLIENT
	errno_t Ret_itoa_s;
	int Ret_getaddrinfo;
#endif // __CLIENT
	int Ret_setsockopt_Linger;
	int Ret_setsockopt_Nagle;
	int Ret_setsockopt_SendBuf;
	int Ret_bind;
	int Ret_listen;

	//---------------------------------------------------
	// Initialize variable
#ifdef __CLIENT
	result = NULL;
#endif // __CLIENT
	Temp_Listen_Socket = INVALID_SOCKET;



	//---------------------------------------------------
	// Initialize Winsock
	Ret_WSAStartup = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (Ret_WSAStartup != NULL)
	{
		// 실패 로그
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_WSAStartup() failed with Error %d", Ret_WSAStartup);
		__debugbreak();
	}
	// 성공 로그
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_WSAStartup() Success #");


#ifdef __CLIENT
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
	Ret_itoa_s = _itoa_s(OpenPort, Port, sizeof(Port), 10);
	if (Ret_itoa_s != NULL)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_itoa_s() failed with Error");
		WSACleanup();
		__debugbreak();
	}
	Port[sizeof(Port) - 1] = '\0';
	Ret_getaddrinfo = getaddrinfo(NULL, Port, &hints, &result);
	if (Ret_getaddrinfo != 0)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_getaddrinfo() failed with Error: %d", Ret_getaddrinfo);
		WSACleanup();
		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_getaddrinfo() Success #");

	//---------------------------------------------------
	// Create a SOCKET for the server to listen for client connections.
	// 서버가 클라이언트 연결을 수신을 SOCKET을 만든다.
	Temp_Listen_Socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (Temp_Listen_Socket == INVALID_SOCKET)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_socket() failed with Error: %d", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_socket() Success #");
#endif // __CLIENT

	Temp_Listen_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Temp_Listen_Socket == INVALID_SOCKET)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_socket() failed with Error: %d", WSAGetLastError());

#ifdef __CLIENT
		freeaddrinfo(result);
#endif // __CLIENT
		WSACleanup();
		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_socket() Success #");

#ifdef __CLIENT
	freeaddrinfo(result);
#endif // __CLIENT


#ifndef __CLIENT
	serverAddrLen = sizeof(SOCKADDR_IN);
	ZeroMemory(&serverAddr, serverAddrLen);

	//WSAStringToAddress()
	//WSAAddressToString()

	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = inet_addr(_st_Setting.OpenIP);
	//serverAddr.sin_addr.s_addr = _tinet_addr(_st_Setting.listenAddress);
	//inet_pton(AF_INET, _st_Setting.OpenIP, &(serverAddr.sin_addr.s_addr));
	_tinet_pton(_st_Setting.listenAddress, &serverAddr);
	InetPton(AF_INET, _st_Setting.listenAddress, &(serverAddr.sin_addr.s_addr));
	serverAddr.sin_port = htons(_st_Setting.listenPort);
#endif // !__CLIENT

	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_Bind Success #");



	//---------------------------------------------------
	// setsockopt_Linger
	Linger_Opt.l_onoff = 1;
	Linger_Opt.l_linger = 0;
	Ret_setsockopt_Linger = setsockopt(Temp_Listen_Socket, SOL_SOCKET, SO_LINGER, (char*)&Linger_Opt, sizeof(Linger_Opt));
	if (Ret_setsockopt_Linger == SOCKET_ERROR)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_setsockopt() Linget_Opt failed with Error: %d", WSAGetLastError());

		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_setsockopt() Linget_Opt Success #");


	//---------------------------------------------------
	// setsockopt_Nagle 
	Ret_setsockopt_Nagle = setsockopt(Temp_Listen_Socket, SOL_SOCKET, TCP_NODELAY, (const char*)&_st_Setting.nodelay, sizeof(_st_Setting.nodelay));
	if (Ret_setsockopt_Nagle == SOCKET_ERROR)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_setsockopt() Nagle_opt failed with Error: %d", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_setsockopt() Nagle_opt%d Success #", _st_Setting.nodelay);

	//---------------------------------------------------
	// sesocket_SendBuffer Size 0
	Ret_setsockopt_SendBuf = setsockopt(Temp_Listen_Socket, SOL_SOCKET, SO_SNDBUF, (char*)&_st_Setting.sendBufferSize, sizeof(_st_Setting.sendBufferSize));
	if (Ret_setsockopt_SendBuf == SOCKET_ERROR)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_setsockopt() Send_Buf_Size failed with Error: %d", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_setsockopt() Send_Buf_Size Success #");

#ifdef __NONBLOCK_SOCKET__
	-------------------------------------------------- -
		// 넌블로킹 소켓으로 전환
		opt = 1;
	Ret_ioctlsocket = ioctlsocket(Temp_Listen_Socket, FIONBIO, &opt);
	if (Ret_ioctlsocket == SOCKET_ERROR)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_setsockopt() FIONBIO failed with Error: %d", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_setsockopt() FIONBIO Success #");
#endif // __NONBLOCK_SOCKET__



	//---------------------------------------------------
	// Setup the TCP listening 
	// TCP listening socket 을 준비한다.
	Ret_bind = bind(Temp_Listen_Socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (Ret_bind == SOCKET_ERROR)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_Bind() failed with Error: %d", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_Bind() Success #");


	//---------------------------------------------------
	// Listen a client socket
	Ret_listen = listen(Temp_Listen_Socket, SOMAXCONN);
	// Ret_listen = listen(Temp_Listen_Socket, SOMAXCONN_HINT(65535)); // 양수 ~200 -> 기본 200개, 200~ 갯수 적용.
	if (Ret_listen == SOCKET_ERROR)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_listen() failed with Error: %d", WSAGetLastError());
		closesocket(Temp_Listen_Socket);
		WSACleanup();
		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_listen() Success #");

	_listenSocket = Temp_Listen_Socket;

	_timeBegin = timeGetTime();

	return;
}

void CLanServer::CreateWorkerThread(void)
{
	int iCnt;

	_handleIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _st_Setting.workerThreadRunning);
	if (_handleIOCP == NULL)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::CreateWorketThread CICP() failed with Error %d", GetLastError());

		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::CreateWorketThread CICP() Success #");

	_hWorkers = new HANDLE[_st_Setting.workerThreadTotal];
	if (_hWorkers == NULL)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::CreateWorkerThread new() failed with Error");

		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::CreateWorkerThread new() Success #");


	for (iCnt = 0; iCnt < _st_Setting.workerThreadTotal; ++iCnt)
	{
		_hWorkers[iCnt] = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadProc, this, NULL, NULL);

		if (_hWorkers[iCnt] == NULL || _hWorkers == INVALID_HANDLE_VALUE)
		{
			LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::CreateWorkerThread _beginthreadex() failed with Error");

			__debugbreak();
		}
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::CreateWorkerThread _beginthreadex() Success #");
	}
}

void CLanServer::CreateAcceptThread()
{
	_hAccept = (HANDLE)_beginthreadex(NULL, 0, AcceptThreadProc, this, NULL, NULL);

	if (_hAccept == NULL || _hAccept == INVALID_HANDLE_VALUE)
	{
		LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::CreateAcceptThread _beginthreadex() failed with Error");

		__debugbreak();
	}
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::CreateAcceptThread _beginthreadex() Success #");

}





void CLanServer::RecvPost(st_SESSION* pSession)
{
	///////
#ifdef MEM_DEUBG
	LONG Cnt = InterlockedIncrement(&_DebugCount);
	LONG idx = Cnt % MAX_LOG_CNT;
	pSession->log[idx].MemLog(Cnt, GetCurrentThreadId(), pSession->sessionID.full_id, enRecvPost, pSession->SendQ._In, pSession->SendQ._Out, pSession->RecvQ._In, pSession->RecvQ._Out, enNone ,0);
#endif
	///////

	//---------------------------------------------------
	// variable
	DWORD flags;
	WSABUF wsaBuf[2];

	//---------------------------------------------------
	// return variable
	int Ret_WSARecv;
	int Ret_WSARecv_Error;

	flags = 0;


	//---------------------------------------------------
	// wsabuf Setting
	// 커널에 copy 하기 때문에 지역으로 설정하고? , 따로 보관하지 않았다. 
	wsaBuf[0].buf = pSession->RecvQ.GetInBufferPtr();
	wsaBuf[0].len = (ULONG)pSession->RecvQ.DirectEnqueueSize();
	wsaBuf[1].buf = pSession->RecvQ.GetBeginBufferPtr();
	wsaBuf[1].len = (ULONG)pSession->RecvQ.GetFreeSize() - wsaBuf[0].len; 
		if (wsaBuf[1].len > (ULONG)pSession->RecvQ.GetBufferSize())
		__debugbreak();

	ZeroMemory(&pSession->RecvOverlapped, sizeof(OVERLAPPED));

	InterlockedIncrement(&pSession->IOCount);
	Ret_WSARecv = WSARecv(pSession->socket, wsaBuf, 2, NULL, &flags, &(pSession->RecvOverlapped), NULL);
	if (Ret_WSARecv == SOCKET_ERROR)
	{
		Ret_WSARecv_Error = WSAGetLastError();

		if (Ret_WSARecv_Error != ERROR_IO_PENDING)
		{
			if (SocketErrorCode(Ret_WSARecv_Error))
			{
				LOG(L"Session", LEVEL_ERROR, true, true, L"CLanServer::RecvPost _ WSARecv() failed with Error %d", Ret_WSARecv_Error);
				__debugbreak();
			}

			if (InterlockedDecrement(&pSession->IOCount) == 0)
			{
				RemoveSession(pSession);
			}
		}
	}
	return;
}

void CLanServer::RecvProc(st_SESSION* pSession, DWORD dwTransferred)
{
	Packet* pPacket = Packet::Alloc();;
	st_NETWORKHEADER networkHeader;
	bool packetCompleted;

	pSession->RecvQ.MoveIn(dwTransferred);

	///////
#ifdef MEM_DEUBG
	LONG Cnt = InterlockedIncrement(&_DebugCount);
	LONG idx = Cnt % MAX_LOG_CNT;
	pSession->log[idx].MemLog(Cnt, GetCurrentThreadId(), pSession->sessionID.full_id, enRecvProc, pSession->SendQ._In, pSession->SendQ._Out, pSession->RecvQ._In, pSession->RecvQ._Out, enNone, dwTransferred);
#endif
	///////

	while (1)
	{
		pPacket->Clear();
		ZeroMemory(&networkHeader, sizeof(st_NETWORKHEADER));

		packetCompleted = GetPacket(pSession, pPacket, &networkHeader);

		if (packetCompleted)
		{


			OnMessage(pSession->sessionID.full_id, pPacket);
		}
		else
		{
			break;
		}
	}
	Packet::Free(pPacket);

	RecvPost(pSession);
}

int CLanServer::SendPost(st_SESSION* pSession)
{
	///////
#ifdef MEM_DEUBG
	LONG Cnt = InterlockedIncrement(&_DebugCount);
	LONG idx = Cnt % MAX_LOG_CNT;
	pSession->log[idx].MemLog(Cnt, GetCurrentThreadId(), pSession->sessionID.full_id, enSendPost, pSession->SendQ._In, pSession->SendQ._Out, pSession->RecvQ._In, pSession->RecvQ._Out, enNone, 0);
#endif
	///////

	//---------------------------------------------------
	// variable
	WSABUF wsaBuf[50];
	size_t TempUseSize;
	size_t TempDirectSize;
	int iCnt;
	DWORD dwBufferNum;
	size_t sendQ_In;
	size_t sendQ_Out;
	Packet* pPacket;	// 주소를 저장하는 지역변수

	//---------------------------------------------------
	// return variable
	int Ret_WSASend;
	int Ret_WSASend_Error;

	if (InterlockedExchange(&pSession->SendFlags, true) == true)
	{	
		///////
#ifdef MEM_DEUBG	
		pSession->log[idx]._RetSendPost = enFlag_True;
#endif
		///////
		return dfSENDPOST_RET_FLAG_FALSE;
	}

	sendQ_In = pSession->SendQ._In;
	sendQ_Out = pSession->SendQ._Out;

	TempUseSize = pSession->SendQ.GetUseSize(sendQ_In, sendQ_Out);

	if (TempUseSize == 0)
	{
		///////
#ifdef MEM_DEUBG	
		pSession->log[idx]._RetSendPost = enUseSizeZero;
#endif
		///////

		InterlockedExchange(&pSession->SendFlags, false);
		return dfSENDPOST_RET_SENDQ_USESIZE_ZERO;
	}

	dwBufferNum = TempUseSize / sizeof(Packet*);
	for (iCnt = 0; iCnt < 50 && iCnt < dwBufferNum; ++iCnt)
	{
		pSession->SendQ.PeekAt((char*)&pPacket, sendQ_Out, sizeof(Packet*));
		wsaBuf[iCnt].buf = pPacket->GetHeaderBeginPtr();
		wsaBuf[iCnt].len = pPacket->GetUseSize();
		sendQ_Out = pSession->SendQ.MoveInOrOutPos(sendQ_Out, sizeof(Packet*));
	}
	///////
#ifdef MEM_DEUBG	
	pSession->log[idx]._DataSize = TempUseSize;
#endif
		///////


	LONG flag = InterlockedExchange(&pSession->SendBuffNum, iCnt);
	if (flag != 0)
		__debugbreak();
	InterlockedIncrement(&pSession->IOCount);
	ZeroMemory(&pSession->SendOverlapped, sizeof(OVERLAPPED));
	Ret_WSASend = WSASend(pSession->socket, wsaBuf, iCnt, NULL, 0, &pSession->SendOverlapped, NULL);
	if (Ret_WSASend == SOCKET_ERROR)
	{
		Ret_WSASend_Error = WSAGetLastError();

		if (Ret_WSASend_Error != ERROR_IO_PENDING)
		{
			if (SocketErrorCode(Ret_WSASend_Error))
			{
				LOG(L"Session", LEVEL_ERROR, true, true, L"CLanServer::SendPost _ WSASend() failed with Error %d", Ret_WSASend_Error);

				__debugbreak();
			}

			/////
			// 상대방이 연결을 끊어서 펜딩이 나오지 않는다면, 완료통지가 오지 않아서, 할당한 직렬화버퍼를 해지하지 않는다. 
			if (InterlockedDecrement(&pSession->IOCount) == 0)
			{
				RemoveSession(pSession);
			}
		}
	}

	///////
#ifdef MEM_DEUBG	
	pSession->log[idx]._RetSendPost = enSucces;
#endif
		///////
	return dfSENDPOST_RET_SUCCESS;
}

void CLanServer::SendProc(st_SESSION* pSession, DWORD dwTransferred)
{
	///////
#ifdef MEM_DEUBG
	LONG Cnt = InterlockedIncrement(&_DebugCount);
	LONG idx = Cnt % MAX_LOG_CNT;
	pSession->log[idx].MemLog(Cnt, GetCurrentThreadId(), pSession->sessionID.full_id, enSendProc, pSession->SendQ._In, pSession->SendQ._Out, pSession->RecvQ._In, pSession->RecvQ._Out, enNone, dwTransferred);
#endif
	///////

	ClearPacket(pSession);

	//pSession->SendQ.MoveOut(dwTransferred);

	InterlockedExchange(&pSession->SendFlags, false);
	while (pSession->SendQ.GetUseSize() > 0)
	{
		if (SendPost(pSession) != dfSENDPOST_RET_SENDQ_USESIZE_ZERO)
			break;
	}
}

void CLanServer::ClearPacket(st_SESSION* pSession)
{
	DWORD dwSendBuffNum;
	DWORD iCnt;
	Packet* pPacket;

	dwSendBuffNum = InterlockedExchange(&pSession->SendBuffNum, 0);
	
	for (iCnt = 0; iCnt < dwSendBuffNum; ++iCnt)
	{
		pSession->SendQ.Dequeue((char*)&pPacket, sizeof(Packet*));
		Packet::Free(pPacket);
	}
}

bool CLanServer::SocketErrorCode(int SocketError)
{
	switch (SocketError)
	{
	case 10053:
	case 10054:
	case 10064:
		return false;
	default:
		return true;
	}
}

bool CLanServer::GetPacket(st_SESSION* pSession, Packet* pPacket, st_NETWORKHEADER* pheader)
{
	//---------------------------------------------------
	// return variable
	size_t Ret_Peek;
	size_t Ret_Dq;

	if (pSession->RecvQ.GetUseSize() < NETWORK_HEADER_SIZE)
		return false;

	Ret_Peek = pSession->RecvQ.Peek((char*)pheader, NETWORK_HEADER_SIZE);
	if (Ret_Peek != NETWORK_HEADER_SIZE)
	{
		LOG(L"Session", LEVEL_ERROR, false, true, L"CLanServer::GetPacketCompleted() failed with Error");

		return false;
	}
	LOG(L"Session", LEVEL_DEBUG, false, true, L"CLanServer::GetPacketCompleted() Success #");


	if (pSession->RecvQ.GetUseSize() < Ret_Peek + pheader->Len)
	{
		return false;
	}
	LOG(L"Session", LEVEL_DEBUG, false, true, L"CLanServer::GetPacketCompleted() Success / PayLoad Len:%d ", pheader->Len);


	pSession->RecvQ.MoveOut(Ret_Peek);

	Ret_Dq = pSession->RecvQ.Dequeue(pPacket->GetPayloadWritePtr(), pheader->Len);
	pPacket->MoveWritePos((int)Ret_Dq);

	return true;
}






st_SESSION* CLanServer::AcquireSession(DWORD64 sessionID)
{
	//------------------------------------------------------
	// 아이디에서 index로 세션을 찾아서 반환한다.
	st_SESSION* pSession = FindSession(sessionID);


	//------------------------------------
	// 찾고보니 세션이 없거나 다른 경우
	if (pSession == NULL)
	{
		return NULL;
	}

	//------------------------------------
	// 해당 세션을 꺼냈더니, index에 들어있는 세션 아이디가 다른경우
	if (pSession->sessionID.full_id != sessionID)
	{
		LOG(L"Session", LEVEL_DEBUG, false, false, L"CLanServer::AcquireSession() faise with Error: ReleaseSession");
		return NULL;
	}


	if ((InterlockedIncrement(&pSession->IOCount) & 0) != 0)
	{
		if (InterlockedDecrement(&pSession->IOCount) == 0)
		{
			RemoveSession(pSession);
		}
		return NULL;
	}

	return pSession;
}

st_SESSION* CLanServer::FindSession(DWORD64 sessionID)
{
	st_SESSION* pSession = NULL;
	short realSessionIndex = 0;

	//realSessionIndex = (short)sessionID & _sessionIndexMask;
	// 문제가 될까?
	realSessionIndex = (short)sessionID;
	pSession = _sessionArray + realSessionIndex;

	return pSession;
}

void CLanServer::ReleaseSession(st_SESSION* pSession)
{
	if (InterlockedDecrement(&pSession->IOCount) == 0)
	{
		RemoveSession(pSession);
	}
}

bool CLanServer::IsShoutdown(void)
{
	return false;
}

void CLanServer::RemoveSession(st_SESSION* pSession)
{
	//---------------------------------------------------
	// variable
	DWORD64 sessionID;
	SOCKET Temp_Socket;
	short sessionArrayIndex;

	if (InterlockedExchange(&pSession->IOCount, 0) != 0)
	{
		return;
	}

	// 1. closesocket
	// 2. Initsocket
	// 3. stack push
	// 4. 

	ReleaseSendFailPacket(pSession);


	////
	size_t sendQ_in = pSession->SendQ._In;
	size_t sendQ_out = pSession->SendQ._Out;
	size_t recvQ_in = pSession->RecvQ._In;
	size_t recvQ_out = pSession->RecvQ._Out;

	if (sendQ_in != sendQ_out)
		__debugbreak();
	if (recvQ_in != recvQ_out)
		__debugbreak();

	if (pSession->RecvQ.GetUseSize() != 0)
		__debugbreak();

	if (pSession->SendQ.GetUseSize() != 0)
		__debugbreak();
	////

	sessionArrayIndex = pSession->sessionID.parts.index;
	sessionID = pSession->sessionID.full_id;
	Temp_Socket = pSession->socket;

	closesocket(Temp_Socket);
	pSession->Init();

	AcquireSRWLockExclusive(&_srw_releaseIndexStack);
	_releaseIndexStack.Push((void**)&sessionArrayIndex); // 사실 내부에서 알아서 2byte만 짤라서 넣기 때문에 이렇게 하지 않아도 된다.
	ReleaseSRWLockExclusive(&_srw_releaseIndexStack);


}

void CLanServer::ReleaseSendFailPacket(st_SESSION* pSession)
{
	DWORD dwSendBufNum;
	DWORD cnt;
	Packet* pPacket;

	dwSendBufNum = pSession->SendQ.GetUseSize() / sizeof(Packet*);
	for (cnt = 0; cnt < dwSendBufNum; ++cnt)
	{
		pSession->SendQ.Dequeue((char*)&pPacket, sizeof(Packet*));
		Packet::Free(pPacket);
	}
}




void CLanServer::_tinet_pton(const TCHAR* cp, SOCKADDR_IN* Addr)
{
#ifdef UNICODE
	char IP[16];
	int Ret = 0;
	Ret = WideCharToMultiByte(CP_ACP, 0, cp, (int)_tcslen(cp), IP, 15, NULL, NULL);
	IP[Ret] = 0;
	inet_pton(AF_INET, IP, &(Addr->sin_addr.s_addr));
#endif // UNICODE

#ifndef UNICODE
	inet_pton(AF_INET, IP, &(Addr->sin_addr.s_addr));
#endif // !UNICODE
}





unsigned  __stdcall AcceptThreadProc(void* param)
{
	CLanServer* cLanServer = (CLanServer*)param;
	return cLanServer->AcceptThread(param);
}

unsigned __stdcall CLanServer::AcceptThread(void* param)
{
	DWORD64 SessionID;

	//---------------------------------------------------
	// variable
	int Client_Addr_Len;
	sockaddr_in Client_Addr;
	SOCKET New_Client_Socket;
	DWORD64 sessionCount;
	bool isSessionSlotAvailable;
	st_SESSION* pSession;

#ifdef __IP_PORT__
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	//char IP[NI_MAXHOST];
	unsigned short PORT;
#endif // __IP_PORT__

	WCHAR clientAddressString[SESSION_ADDRESS_WCHAR_LENGTH];
	DWORD clientAddressStringLen;
	DWORD clientAddressIP;
	USHORT clientAddressPORT;


	//---------------------------------------------------
	// return variable
	int Ret_Accept_Error;
	HANDLE Ret_CICP_Error;

	SessionID = 0;
	pSession = NULL;

	while (1)
	{
		New_Client_Socket = INVALID_SOCKET;
		Client_Addr_Len = sizeof(Client_Addr);
		clientAddressStringLen = 0;

		//---------------------------------------------------
		// accept()
		New_Client_Socket = accept(_listenSocket, (sockaddr*)&Client_Addr, &Client_Addr_Len);
		if (New_Client_Socket == INVALID_SOCKET)
		{
			if (IsShoutdown())
			{
				LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Shutdown #");

				IOCPDisconnect();
				return 0;
			}
			Ret_Accept_Error = WSAGetLastError();
			LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::AcceptThread _ accept() failed with Error %d", Ret_Accept_Error);
			continue;
		}
		LOG(L"Session", LEVEL_DEBUG, true, true, L"CLanServer::AcceptThread _ accept() Success / socket:%d #", New_Client_Socket);


		//---------------------------------------------------
		// MAX Accept Session
		AcquireSRWLockShared(&_srw_releaseIndexStack);
		isSessionSlotAvailable = _releaseIndexStack.isEmpty();
		ReleaseSRWLockShared(&_srw_releaseIndexStack);
		if (isSessionSlotAvailable)
		{
			LOG(L"Session", LEVEL_SYSTEM, true, true, L"CLanServer::AcceptThread _ MaxSession... Server is Full");
			closesocket(New_Client_Socket);
			// 여기에 Sleep을 넣어서 잠시 쉬는게 맞는지 고민 중....
			continue;
		}
#ifdef __IP_PORT__
		//---------------------------------------------------
		// printf Client info
		if (getnameinfo((sockaddr*)&Client_Addr, sizeof(Client_Addr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
		{
			LOG(L"Session", LEVEL_DEBUG, true, true, L"CLanServer::AcceptThread _ getnameinfo() # host:%s / service:%s Accept", host, service);
		}
		else
		{
			inet_ntop(AF_INET, &Client_Addr.sin_addr, IP, NI_MAXHOST);
			PORT = ntohs(Client_Addr.sin_port);
			LOG(L"Session", LEVEL_DEBUG, true, true, L"CLanServer::AcceptThread _ getnameinfo() # host:%s / service:%d Accept", IP, PORT);
		}
#endif // __IP_PORT_

		if (OnConnectionRequest(clientAddressIP, clientAddressPORT, clientAddressString))
		{
			LOG(L"Session", LEVEL_ERROR, false, true, L"CLanServer::AcceptThread _ OnConnectionRequest() failed with Error");
			closesocket(New_Client_Socket);
			continue;
		}


		//---------------------------------------------------
		// Session struct new
		pSession = CreateSession(New_Client_Socket, ++SessionID, Client_Addr);
		LOG(L"Session", LEVEL_DEBUG, true, true, L"CLanServer::AcceptThread _ CreateSession() Success / socket:%d / sessionID:%d #", New_Client_Socket, pSession->sessionID.full_id);


		//---------------------------------------------------
		// link Client_Socket to IOCP
		Ret_CICP_Error = CreateIoCompletionPort((HANDLE)New_Client_Socket, _handleIOCP, (ULONG_PTR)pSession, 0);
		if (Ret_CICP_Error == NULL)
		{
			LOG(L"Session", LEVEL_ERROR, true, true, L"CLanServer::AcceptThread _ CICP() failed with Error %d", GetLastError());
			AcquireSRWLockExclusive(&_srw_releaseIndexStack);
			_releaseIndexStack.Push((void**)&pSession->sessionID); // 사실 내부에서 알아서 2byte만 짤라서 넣기 때문에 이렇게 하지 않아도 된다.
			ReleaseSRWLockExclusive(&_srw_releaseIndexStack);
			closesocket(New_Client_Socket);
			delete pSession;
		}
		LOG(L"Session", LEVEL_DEBUG, true, true, L"CLanServer::AcceptThread _ CICP() Success #");



		OnAccept(pSession->sessionID.full_id, clientAddressIP, clientAddressPORT, clientAddressString);

		//--------------------------------------------------------
		// 비동기 입출력 시작, 해당 소켓을 비동기IO에 등록한다. 
		RecvPost(pSession);

		InterlockedIncrement(&_sessionAccepted);
		InterlockedIncrement(&_acceptPerSecondCounter);
	};
	return 0;
}

unsigned  __stdcall WorkerThreadProc(void* param)
{
	CLanServer* cLanServer = (CLanServer*)param;
	return cLanServer->WorkerThread(param);
}

unsigned __stdcall CLanServer::WorkerThread(void* param)
{
	//---------------------------------------------------
	// variable
	DWORD dwTransferred;
	ULONG_PTR completionKey;
	OVERLAPPED* lpOverlapped;
	st_SESSION* pSession;



	//---------------------------------------------------
	// return variable
	bool Ret_GQCS;

	while (1)
	{
		dwTransferred = 0;
		completionKey = NULL;
		lpOverlapped = NULL;
		pSession = NULL;


		Ret_GQCS = GetQueuedCompletionStatus(_handleIOCP, &dwTransferred, (ULONG_PTR*)&pSession, &lpOverlapped, INFINITE);
		
		if (lpOverlapped == NULL)
		{
		}

		if (Ret_GQCS != NULL && dwTransferred != 0)
		{
			if (lpOverlapped == &pSession->RecvOverlapped)
			{
				RecvProc(pSession, dwTransferred);
			}
			else if (lpOverlapped == &pSession->SendOverlapped)
			{
				SendProc(pSession, dwTransferred);
			}
			else
			{
				__debugbreak();
			}
		}

	}

	__debugbreak();
	return -1;
}

