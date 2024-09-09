#pragma once

////////////////////////////////////////////////////////
//
// 	//---------------------------------------------------
//	// Initialize Winsock
// Ret_WSAStartup = WSAStartup(MAKEWORD(2, 2), &_WsaData);
// if (Ret_WSAStartup != NULL)
// {
// 	// 실패 로그
// 	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_WSAStartup() failed with Error %d", Ret_WSAStartup);
// 	__debugbreak();
// }
// // 성공 로그
// LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"CLanServer::Start_WSAStartup() Success #");
// 
////////////////////////////////////////////////////////
#ifndef __CLANSERVER_H__
#define __CLANSERVER_H__

#define IN
#define OUT

#include <Windows.h>
#include <unordered_map>
#include "CDebug.h"
#include "C_Logger.h"
#include "CStack.h"
#include "C_Ring_Buffer.h"
#include "NetworkHeader.h"
#include "Session.h"
#include "CLanServerSettings.h"
#include "CMemoryPool.h"
#include "Packet.h"

#include "Logger.h"
#pragma comment(lib,"LoggerMT.lib")

#pragma comment(lib,"Winmm.lib")	// timeGetTime();

class CLanServer
{
public:
	CLanServer();
	virtual ~CLanServer();
	bool Start(st_SETTINGS* setting);
	void Stop(void);
	int GetSessionCount(void);

	bool Disconnect(DWORD64 SessionID);
	bool SendPacket(DWORD64 SessionID, Packet* pPacket);

	virtual bool OnConnectionRequest(OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString) = 0;
	virtual void OnAccept(OUT DWORD64 SessionID, OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString) = 0;
			// OnClientJoin, OnSessionConnected
	virtual void OnRelease(OUT DWORD64 SessionID) = 0;
			// OnClientLeave, OnSessionDisconnected
	virtual void OnMessage(DWORD64 SessionID, Packet* pPacket) = 0;
			// OnRecv, OnRecvMessage
	virtual void OnSend(void) = 0;

	virtual void OnWorkerThreadBegin(void) = 0;
	virtual void OnWorkerThreadEnd(void) = 0;

	virtual void OnError(int ErrorCode, wchar_t* ErrorMessage) = 0;
public:
	unsigned __stdcall AcceptThread(void* pArg);
	unsigned __stdcall WorkerThread(void* pArg);



public:
	__int64 _AcceptTPS;
	__int64 _RecvMessageTPS;
	__int64 _SendMessageTPS;


private:
	st_SETTINGS _st_Setting;
	SOCKET _listenSocket;
	HANDLE* _hWorkers;
	HANDLE _hAccept;
	HANDLE _handleIOCP;


	short _sessionIndexMask;
	st_SESSION* _sessionArray;	// 0으로 초기화 하여 세팅함, 0 초기화 인덱스... 안넘어가겠지...
	CStack	_releaseIndexStack;	// close 없음....
	SRWLOCK	_srw_releaseIndexStack;



private:
	st_SESSION* CreateSession(SOCKET socket, DWORD64 SessionID, sockaddr_in Addr);

	//----------------------------------------------------------------
	// 초기화	
private:
	void InitServer(st_SETTINGS* setting);
	void InitDeleteIndexStack(void);
	void InitSessionIndexMask(void);
	void InitNetwork(void);
	void CreateWorkerThread(void);
	void CreateAcceptThread(void);


private:
	void RecvPost(st_SESSION* pSession);
	void RecvProc(st_SESSION* pSession, DWORD dwTransferred);
	int SendPost(st_SESSION* pSession);
	void SendProc(st_SESSION* pSession, DWORD dwTransferred);
	void ClearPacket(st_SESSION* pSession);
	bool SocketErrorCode(int SocketError);
	bool GetPacket(st_SESSION* pSession, Packet* pPacket, st_NETWORKHEADER* pheader);

private:
	st_SESSION* AcquireSession(DWORD64 sessionID);
	st_SESSION* FindSession(DWORD64 sessionID);
	void ReleaseSession(st_SESSION* pSession);

private:
	bool IsShoutdown(void);
	void RemoveSession(st_SESSION* pSession);
	void ReleaseSendFailPacket(st_SESSION* pSession);
	void IOCPDisconnect(void);


private:
	void _tinet_pton(const TCHAR* cp, SOCKADDR_IN* Addr);

private:
	DWORD64 _timeBegin;
	alignas(64) DWORD64		_sessionCount;
	alignas(64) DWORD64		_sessionAccepted;
	alignas(64) DWORD64		_acceptPerSecondCounter;


	alignas(64)DWORD64		_DebugCount;
};


#endif // !__CLANSERVER_H__