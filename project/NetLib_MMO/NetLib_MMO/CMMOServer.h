#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 여기 따로 세션이 있음.
// 
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <unordered_map>
#include "CLanServer.h"

class CMMOServer : public CLanServer
{
public:
	CMMOServer();
	~CMMOServer();



	virtual bool OnConnectionRequest(OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString);
	virtual void OnAccept(OUT DWORD64 SessionID, OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString);
	virtual void OnRelease(OUT DWORD64 SessionID);
	virtual void OnMessage(DWORD64 SessionID, Packet* pPacket);
	virtual void OnSend(void);

	virtual void OnWorkerThreadBegin(void);
	virtual void OnWorkerThreadEnd(void);

	virtual void OnError(int ErrorCode, wchar_t* ErrorMessage);

public:
	// 처음에 메시지의 재활용을 고려하여 밖에서 메시지를 한번만 생성하여 그걸로 모든 메시지를 보내는 형식을 하고 싶었는데
	// 메시지를 지울 때, 문제가 발생할거 같다.
	void mpCreateMyCharacter(DWORD64 DestID, st_CLIENT *pClient);
	void mpCreateOtherCharacter(DWORD64 DestID, st_CLIENT* pClient);
	void mpDeleteCharacter(DWORD64 DestID, st_CLIENT* pClient);
	void mpMoveStart(DWORD64 DestID, st_CLIENT* pClient);

public:
	bool netPacketProc_Movestart(st_CLIENT* pClient, Packet* pPacket);
	bool netPacketProc_MoveStop(st_CLIENT* pClient, Packet* pPacket);
	bool netPacketProc_Attack1(st_CLIENT* pClient, Packet* pPacket);
	bool netPacketProc_Attack2(st_CLIENT* pClient, Packet* pPacket);
	bool netPacketProc_Attack3(st_CLIENT* pClient, Packet* pPacket);
	bool netPacketProc_Echo(st_CLIENT* pClient, Packet* pPacket);


public:
	void ActionProc(st_CLIENT* pClient, DWORD64 DestId);

	void SendPacketAround(st_SECTOR_AROUND* pSectorAround, st_CLIENT* pClient, char MSG, bool bSendMe = false, bool bAction = false);
	void SendPacketOne(int sectorX, int sectorY, char MSG, st_CLIENT* pClient, DWORD ExceptID = NULL, bool bAction = false);

	void SendPacketAroundToMe(st_SECTOR_AROUND* pSectorAround, st_CLIENT* pDestClient, char MSG, bool bAction = false); // 주변에서 나에게로 메시지 보냄
	void SendPacketOneToMe(int sectorX, int sectorY, char MSG, st_CLIENT* pDestClient, bool bAction = false);
public:
	alignas(64) LONG		_IDCount;
};
