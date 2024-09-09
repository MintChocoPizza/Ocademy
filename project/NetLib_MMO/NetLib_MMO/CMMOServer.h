#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���� ���� ������ ����.
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
	// ó���� �޽����� ��Ȱ���� ����Ͽ� �ۿ��� �޽����� �ѹ��� �����Ͽ� �װɷ� ��� �޽����� ������ ������ �ϰ� �;��µ�
	// �޽����� ���� ��, ������ �߻��Ұ� ����.
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

	void SendPacketAroundToMe(st_SECTOR_AROUND* pSectorAround, st_CLIENT* pDestClient, char MSG, bool bAction = false); // �ֺ����� �����Է� �޽��� ����
	void SendPacketOneToMe(int sectorX, int sectorY, char MSG, st_CLIENT* pDestClient, bool bAction = false);
public:
	alignas(64) LONG		_IDCount;
};
