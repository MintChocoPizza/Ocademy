#pragma once

#include "C_Ring_Buffer.h"
#include "Packet.h"

#include "CDebug.h"

#define SESSION_ADDRESS_WCHAR_LENGTH 32



union IDUnion
{
	struct
	{
		uint64_t index : 16;		// 하위 2바이트 (index)
		uint64_t id : 48;			// 상위 6바이트 (ID)
	}parts;

	uint64_t full_id;		// 전체 8바이트
};
#define st_sessionID		parts.id;
#define st_sessionIndex		parts.index;




struct st_SESSION
{
	st_SESSION() : socket(INVALID_SOCKET), sessionID{0,0}, socketAddressIP(0), socketAddressPort(0), socketAddressString{ 0 },
		IOCount(0), SendFlags(0), SendBuffNum(0)
	{

	}
	SOCKET				socket;
	IDUnion				sessionID;
	DWORD				socketAddressIP;
	USHORT				socketAddressPort;
	WCHAR				socketAddressString[SESSION_ADDRESS_WCHAR_LENGTH];

	OVERLAPPED			RecvOverlapped;
	OVERLAPPED			SendOverlapped;

	C_RING_BUFFER		RecvQ;
	C_RING_BUFFER		SendQ;

	/// 
#ifdef MEM_DEUBG
	st_Debug			log[MAX_LOG_CNT];
#endif // MEM_DEUBG
	///

	alignas(64)LONG		IOCount;
	alignas(64)LONG		SendFlags;
	alignas(64)LONG		SendBuffNum;



	void Init(void)
	{
		socket = INVALID_SOCKET;
		sessionID = {0,0};
		socketAddressIP = 0;
		socketAddressPort = 0;
		socketAddressString[0] = '\0';

		ZeroMemory(&RecvOverlapped, sizeof(OVERLAPPED));
		ZeroMemory(&SendOverlapped, sizeof(OVERLAPPED));

		RecvQ.ClearBuffer();
		SendQ.ClearBuffer();
	}
};

