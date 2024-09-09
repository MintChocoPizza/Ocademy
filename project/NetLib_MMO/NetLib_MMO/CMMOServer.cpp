
#include <Windows.h>

#include "Define.h"
#include "Protocol.h"
#include "CMemPoolList.h"
#include "MOVE_DIR.h"
#include "Direction.h"
#include "Sector.h"
#include "Client.h"
#include "CMMOServer.h"

extern st_SECTOR_CLIENT_INFO g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];
extern std::unordered_map<DWORD64, st_CLIENT*> g_Client_Hash;
extern SRWLOCK g_srw_Client_Hash;


//------------------------------------------------------
// 메모리풀
OreoPizza::CMemoryPool<st_CLIENT> g_Client_MemPool(0, false);




CMMOServer::CMMOServer() : _IDCount(0)
{
}

CMMOServer::~CMMOServer()
{
}

bool CMMOServer::OnConnectionRequest(OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString)
{
	return true;
}

void CMMOServer::OnAccept(OUT DWORD64 SessionID, OUT DWORD IP, OUT USHORT PORT, OUT WCHAR* IPString)
{
	st_CLIENT* pNewClient;
	LONG id;
	Packet* pPacket;
	st_SECTOR_AROUND SectorAround;
	int iCnt;
	
	pNewClient = g_Client_MemPool.Alloc();

	//-----------------------------------------------------------
	// 클라이언트  초기화
	pNewClient->sessionID = SessionID;
	pNewClient->IP = IP;
	pNewClient->PORT = PORT;
	wcscpy_s(pNewClient->clientAddressString, IPString);

	id = InterlockedIncrement(&_IDCount);
	pNewClient->_dwID = id;

	pNewClient->_dwAction = dfMOVE_STOP;
	pNewClient->_byDirection = dfPACKET_MOVE_DIR_LL;
	pNewClient->_byMoveDirection = dfMOVEDIRECTION_STOP;

#if _DEBUG
	pNewClient->_shX = 300;
	pNewClient->_shY = 300;
#else
	pNewClient->_shX = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT + 1) + dfRANGE_MOVE_LEFT;
	pNewClient->_shY = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP + 1) + dfRANGE_MOVE_TOP;
#endif // _DEBUG

	pNewClient->_cHP = 100;

	pNewClient->_CurSector.Init_SECTOR_POS(pNewClient->_shX, pNewClient->_shY);
	pNewClient->_OldSector = pNewClient->_CurSector;

	
	//-----------------------------------------------------------
	// 메시지 뿌리기
	mpCreateMyCharacter(pNewClient->sessionID, pNewClient);

	LockSectorAroundExclusive(pNewClient->_CurSector);
	GetSectorAround(pNewClient->_CurSector, &SectorAround);


	// 이자리에 원하는거: 센트패킷어라운드
	// 역할 주변 섹터에 원하는 메시지를 만들어서 뿌린다. 
	// 이걸 함수로 만들지 모르겠음
	int i;
	int nSectorX;
	int nSectorY;
	CList<st_CLIENT*>::iterator iter;
	st_CLIENT* pOtherClient;
	for (i = 0; i < SectorAround.iCount; ++i)
	{
		nSectorX = SectorAround.Around[i].iX;
		nSectorY = SectorAround.Around[i].iY;

		iter = g_Sector[nSectorY][nSectorX].client_List.begin();
		for (; iter != g_Sector[nSectorY][nSectorX].client_List.end(); ++iter)
		{
			pOtherClient = *iter;

			mpCreateOtherCharacter(pNewClient->sessionID, pOtherClient);

			if (pOtherClient ->_dwAction == dfMOVE)
			{
				mpMoveStart(pNewClient->sessionID, pOtherClient);
			}
		}
	}

	UnLockSectorAroundExclusive(pNewClient->_CurSector);
}


void CMMOServer::OnRelease(OUT DWORD64 SessionID)
{

}

void CMMOServer::OnMessage(DWORD64 SessionID, Packet* pPacket)
{
	char byType;
	st_CLIENT* pClient;

	AcquireSRWLockShared(&g_srw_Client_Hash);
	pClient = g_Client_Hash.find(SessionID)->second;
	ReleaseSRWLockShared(&g_srw_Client_Hash);

	*pPacket >> byType;
	switch (byType)
	{
	case dfPACKET_CS_MOVE_START:
		netPacketProc_Movestart(pClient, pPacket);
		break;
	case dfPACKET_CS_MOVE_STOP:
		break;
	case dfPACKET_CS_ATTACK1:
		break;
	case dfPACKET_CS_ATTACK2:
		break;
	case dfPACKET_CS_ATTACK3:
		break;
	case dfPACKET_CS_ECHO:
		break;

	default:
		break;
	}
}

void CMMOServer::OnSend(void)
{
}

void CMMOServer::OnWorkerThreadBegin(void)
{
}

void CMMOServer::OnWorkerThreadEnd(void)
{
}

void CMMOServer::OnError(int ErrorCode, wchar_t* ErrorMessage)
{
}



void CMMOServer::mpCreateMyCharacter(DWORD64 DestID, st_CLIENT *pClient)
{
	Packet* pPacket = Packet::Alloc();


	*pPacket << (char)dfPACKET_SC_CREATE_MY_CHARACTER << pClient->_dwID << pClient->_byDirection << pClient->_shX << pClient->_shY << pClient->_cHP;

	SendPacket(DestID, pPacket);
}

void CMMOServer::mpCreateOtherCharacter(DWORD64 DestID,  st_CLIENT* pClient)
{
	Packet* pPacket = Packet::Alloc();

	*pPacket << (short)dfPACKET_SC_CREATE_OTHER_CHARACTER << pClient->_dwID << pClient->_byDirection << pClient->_shX << pClient->_shY << pClient->_cHP;

	SendPacket(DestID, pPacket);
}

void CMMOServer::mpDeleteCharacter(DWORD64 DestID, st_CLIENT* pClient)
{
	Packet* pPacket = Packet::Alloc();

	*pPacket << (short)dfPACKET_SC_DELETE_CHATACTER << pClient->_dwID;

	SendPacket(DestID, pPacket);
}

void CMMOServer::mpMoveStart(DWORD64 DestID, st_CLIENT* pClient)
{
	Packet* pPacket = Packet::Alloc();

	*pPacket << (short)dfPACKET_SC_MOVE_START << pClient->_dwID << pClient->_byMoveDirection << pClient->_shX << pClient->_shY;

	SendPacket(DestID, pPacket);
}


bool CMMOServer::netPacketProc_Movestart(st_CLIENT* pClient, Packet* pPacket)
{
	char byDirection;
	short shX;
	short shY;
	st_SECTOR_AROUND SectorAround;

	*pPacket >> byDirection >> shX >> shY;

	AcquireSRWLockExclusive(&pClient->srw_ClientLock);
	
	if (abs(pClient->_shX - shX) > dfERROR_RANGE || abs(pClient->_shY - shY) > dfERROR_RANGE)
	{
		LockSectorAroundShared(pClient->_CurSector);

		GetSectorAround(pClient->_CurSector, &SectorAround);
		SendPacketAround(&SectorAround, pClient, dfPACKET_SC_SYNC, true);

		UnLockSectorAroundShared(pClient->_CurSector);

		shX = pClient->_shX;
		shY = pClient->_shY;
	}
	
	//---------------------------------------------------------------------------------------------------------
	// 이거 너무 이상함...
	// Update에서는 client_Hash에 락을 걸고 업데이트 하는데
	// 그럼 이 데이터가 반영 안될 수 잇음.
	// 그럼 플레이어에 직접적인 락이 의미가 있는가?
	// 그렇다면 업데이트에서는 2개의 락을 걸어야함....
	pClient->_dwAction = dfMOVE;
	pClient->_byMoveDirection = byDirection;
	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pClient->_byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RD:
		pClient->_byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	default:
		break;
	}
	pClient->_shX = shX;
	pClient->_shY = shY;


	//---------------------------------------------------------------------------------------------------------
	// 여기서 문제가 되는게 필요한 부분을 좌상단에서 우하단으로 순차적으로 락을 걸어야 한다. 
	// 섹터가 바뀌었다면.....
	// 여기서 한번에 락을 걸고 그에대한 메시지를 보내야 할거 같다.
	// 서순: 무조건 클라이언트 락을 나중에 건다.
	if (IsSectorUpdateCharacter(pClient));
	{
		LOCK_CharacterSectorUpdatePacket(pClient);
	}
	


	ReleaseSRWLockExclusive(&pClient->srw_ClientLock);
	return false;
}



void CMMOServer::ActionProc(st_CLIENT* pClient, DWORD64 DestId)
{
	// 이것도 enum 으로 묶어서 하고 싶음
	switch (pClient->_dwAction)
	{
	case dfMOVE_STOP:
		break;
	case dfMOVE:
		mpMoveStart(DestId, pClient);

	default:
		break;
	}
}

void CMMOServer::SendPacketAround(st_SECTOR_AROUND* pSectorAround, st_CLIENT* pClient, char MSG, bool bSendMe, bool bAction)
{
	int iCnt;
	int nSectorX;
	int nSectorY;

	if (bSendMe == false)
	{
		for (iCnt = 0; iCnt < pSectorAround->iCount; ++iCnt)
		{
			nSectorX = pSectorAround->Around[iCnt].iX;
			nSectorY = pSectorAround->Around[iCnt].iY;

			SendPacketOne(nSectorX, nSectorY, MSG, pClient, pClient->_dwID, bAction);
		}
	}
	else
	{
		for (iCnt = 0; iCnt < pSectorAround->iCount; ++iCnt)
		{
			nSectorX = pSectorAround->Around[iCnt].iX;
			nSectorY = pSectorAround->Around[iCnt].iY;

			SendPacketOne(nSectorX, nSectorY, MSG, pClient, NULL, bAction);
		}
	}
}

void CMMOServer::SendPacketOne(int sectorX, int sectorY, char MSG, st_CLIENT* pClient, DWORD ExceptID, bool bAction)
{
	CList<st_CLIENT*>::iterator iter;
	st_CLIENT* pDestClient;

	iter = g_Sector[sectorY][sectorX].client_List.begin();

	switch (MSG)
	{
	case dfPACKET_SC_CREATE_MY_CHARACTER:
		for (; iter != g_Sector[sectorY][sectorX].client_List.end(); ++iter)
		{
			pDestClient = *iter;

			if (pDestClient->_dwID == ExceptID)
				continue;
			
			mpCreateMyCharacter(pDestClient->sessionID, pClient);

			if (bAction)
			{
				ActionProc(pClient, pDestClient->sessionID);
			}
		}
		break;
	case dfPACKET_SC_CREATE_OTHER_CHARACTER:
		for (; iter != g_Sector[sectorY][sectorX].client_List.end(); ++iter)
		{
			pDestClient = *iter;

			if (pDestClient->_dwID == ExceptID)
				continue;

			mpCreateOtherCharacter(pDestClient->sessionID, pClient);

			if (bAction)
			{
				ActionProc(pClient, pDestClient->sessionID);
			}
		}
		break;
	case dfPACKET_SC_DELETE_CHATACTER:
		for (; iter != g_Sector[sectorY][sectorX].client_List.end(); ++iter)
		{
			pDestClient = *iter;

			if (pDestClient->_dwID == ExceptID)
				continue;

			mpDeleteCharacter(pDestClient->sessionID, pClient);

			if (bAction)
			{
				ActionProc(pClient, pDestClient->sessionID);
			}
		}
		break;
	case dfPACKET_SC_MOVE_START:
		break;
	case dfPACKET_SC_MOVE_STOP:
		break;
	case dfPAKCET_SC_ATTACK1:
		break;
	case dfPAKCET_SC_ATTACK2:
		break;
	case dfPAKCET_SC_ATTACK3:
		break;
	case dfPACKET_SC_DAMAGE:
		break;
	case dfPACKET_SC_SYNC:
		break;
	case dfPACKET_SC_ECHO:
		break;
	default:
		break;
	}
}

void CMMOServer::SendPacketAroundToMe(st_SECTOR_AROUND* pSectorAround, st_CLIENT* pDestClient, char MSG, bool bAction)
{
	int iCnt;
	int nSectorX; 
	int nSectorY;

	for (iCnt = 0; iCnt < pSectorAround->iCount; ++iCnt)
	{
		nSectorX = pSectorAround->Around[iCnt].iX;
		nSectorY = pSectorAround->Around[iCnt].iY;

		SendPacketOneToMe(nSectorX, nSectorY, MSG, pDestClient, bAction);
	}
}

void CMMOServer::SendPacketOneToMe(int sectorX, int sectorY, char MSG, st_CLIENT* pDestClient, bool bAction)
{
	CList<st_CLIENT*>::iterator iter;
	st_CLIENT* pSourcetClient;

	iter = g_Sector[sectorY][sectorX].client_List.begin();

	switch (MSG)
	{
	case dfPACKET_SC_CREATE_MY_CHARACTER:
		for (; iter != g_Sector[sectorY][sectorX].client_List.end(); ++iter)
		{
			pSourcetClient = *iter;

			if (pSourcetClient->_dwID == pDestClient->_dwID)
				continue;

			mpCreateMyCharacter(pDestClient->sessionID, pSourcetClient);

			if (bAction)
			{
				ActionProc(pSourcetClient, pDestClient->sessionID);
			}
		}
		break;
	case dfPACKET_SC_CREATE_OTHER_CHARACTER:
		for (; iter != g_Sector[sectorY][sectorX].client_List.end(); ++iter)
		{
			pSourcetClient = *iter;

			if (pSourcetClient->_dwID == pDestClient->_dwID)
				continue;

			mpCreateOtherCharacter(pDestClient->sessionID, pSourcetClient);

			if (bAction)
			{
				ActionProc(pSourcetClient, pDestClient->sessionID);
			}
		}
		break;
	case dfPACKET_SC_DELETE_CHATACTER:
		for (; iter != g_Sector[sectorY][sectorX].client_List.end(); ++iter)
		{
			pSourcetClient = *iter;

			if (pSourcetClient->_dwID == pDestClient->_dwID)
				continue;

			mpDeleteCharacter(pDestClient->sessionID, pSourcetClient);

			if (bAction)
			{
				ActionProc(pSourcetClient, pDestClient->sessionID);
			}
		}
		break;
	case dfPACKET_SC_MOVE_START:
		break;
	case dfPACKET_SC_MOVE_STOP:
		break;
	case dfPAKCET_SC_ATTACK1:
		break;
	case dfPAKCET_SC_ATTACK2:
		break;
	case dfPAKCET_SC_ATTACK3:
		break;
	case dfPACKET_SC_DAMAGE:
		break;
	case dfPACKET_SC_SYNC:
		break;
	case dfPACKET_SC_ECHO:
		break;
	default:
		break;
	}
}



