#include "EchoServer.h"

#define LOGIN_PAYLOAD ((ULONGLONG)0x7fffffffffffffff)

CEchoServer::CEchoServer()
{
	InitializeSRWLock(&_srw_client_Hash);
}

CEchoServer::~CEchoServer()
{
}

bool CEchoServer::OnConnectionRequest(OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString)
{
	return false;
}

void CEchoServer::OnAccept(OUT DWORD64 SessionID, OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString)
{
	Packet* pPacket;

	pPacket = Packet::Alloc();
	*pPacket << 0x7fffffffffffffff;

	SendPacket(SessionID, pPacket);
}

void CEchoServer::OnRelease(OUT DWORD64 SessionID)
{

}

void CEchoServer::OnMessage(DWORD64 SessionID, Packet* pPacket)
{
	Packet* pNewPacket;

	pNewPacket = Packet::Alloc();
	pNewPacket->Clear();
	pNewPacket->PutData(pPacket->GetPayloadReadPtr(), pPacket->GetPayloadSize());

	SendPacket(SessionID, pNewPacket);
}

void CEchoServer::OnSend(void)
{
	return;
}

void CEchoServer::OnWorkerThreadBegin(void)
{
	return;
}

void CEchoServer::OnWorkerThreadEnd(void)
{
	return;
}

void CEchoServer::OnError(int ErrorCode, wchar_t* ErrorMessage)
{
	return;
}
