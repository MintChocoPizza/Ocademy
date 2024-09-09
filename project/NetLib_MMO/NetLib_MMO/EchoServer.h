#pragma once

#include "CLanServer.h"
#include "Client.h"

class CEchoServer : public CLanServer
{
public:
	CEchoServer();
	~CEchoServer();

public:

	virtual bool OnConnectionRequest(OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString);
	virtual void OnAccept(OUT DWORD64 SessionID, OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString);
	// OnClientJoin, OnSessionConnected
	virtual void OnRelease(OUT DWORD64 SessionID);
	// OnClientLeave, OnSessionDisconnected
	virtual void OnMessage(DWORD64 SessionID, Packet* pPacket);
	// OnRecv, OnRecvMessage
	virtual void OnSend(void);

	virtual void OnWorkerThreadBegin(void);
	virtual void OnWorkerThreadEnd(void);

	virtual void OnError(int ErrorCode, wchar_t* ErrorMessage);

	std::unordered_map<DWORD64, st_CLIENT*> _client_Hash;
	SRWLOCK _srw_client_Hash;

};
