
#include <Windows.h>

#include "Define.h"
#include "Protocol.h"
#include "CMemPoolList.h"
#include "MOVE_DIR.h"
#include "Direction.h"
#include "Sector.h"
#include "Client.h"
#include "CMMOServer.h"

extern CMMOServer g_MMOServer;

// 좌상단 부터 순회하는 좌표
int dx[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
int dy[9] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };


st_SECTOR_CLIENT_INFO g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void st_SECTOR_POS::Init_SECTOR_POS(int X, int Y)
{
	iX = X / (dfGRID_X_SIZE);
	iY = Y / (dfGRID_Y_SIZE);
}

void LockSectorAroundShared(st_SECTOR_POS sector)
{
	int iCnt;
	int nSectorX;
	int nSectorY;

	for (iCnt = 0; iCnt < 9; ++iCnt)
	{
		nSectorX = sector.iX + dx[iCnt];
		nSectorY = sector.iY + dy[iCnt];

		if (InValidPos(nSectorX, nSectorY))
			AcquireSRWLockShared(&g_Sector[nSectorY][nSectorX].srwSector);
	}
}

void UnLockSectorAroundShared(st_SECTOR_POS sector)
{
	int iCnt;
	int nSectorX;
	int nSectorY;

	for (iCnt = 0; iCnt < 9; ++iCnt)
	{
		nSectorX = sector.iX + dx[iCnt];
		nSectorY = sector.iY + dy[iCnt];

		if (InValidPos(nSectorX, nSectorY))
			ReleaseSRWLockShared(&g_Sector[nSectorY][nSectorX].srwSector);
	}
}

void LockSectorAroundExclusive(st_SECTOR_POS sector)
{
	int iCnt;
	int nSectorX;
	int nSectorY;

	for (iCnt = 0; iCnt < 9; ++iCnt)
	{
		nSectorX = sector.iX + dx[iCnt];
		nSectorY = sector.iY + dy[iCnt];

		if (InValidPos(nSectorX, nSectorY))
			AcquireSRWLockExclusive(&g_Sector[nSectorY][nSectorX].srwSector);
	}
}

void UnLockSectorAroundExclusive(st_SECTOR_POS sector)
{
	int iCnt;
	int nSectorX;
	int nSectorY;

	for (iCnt = 0; iCnt < 9; ++iCnt)
	{
		nSectorX = sector.iX + dx[iCnt];
		nSectorY = sector.iY + dy[iCnt];

		if (InValidPos(nSectorX, nSectorY))
			ReleaseSRWLockShared(&g_Sector[nSectorY][nSectorX].srwSector);
	}
}

void LockSectorAroundShared(st_SECTOR_AROUND* pSectorAround)
{
	int iCnt;
	int sectorX;
	int sectorY;
	for (iCnt = 0; iCnt < pSectorAround->iCount; ++iCnt)
	{
		sectorX = pSectorAround->Around[iCnt].iX;
		sectorY = pSectorAround->Around[iCnt].iY;
		AcquireSRWLockShared(&g_Sector[sectorY][sectorX].srwSector);
	}
}

void UnLockSectorAroundShared(st_SECTOR_AROUND* pSectorAround)
{
	int iCnt;
	int sectorX;
	int sectorY;
	for (iCnt = 0; iCnt < pSectorAround->iCount; ++iCnt)
	{
		sectorX = pSectorAround->Around[iCnt].iX;
		sectorY = pSectorAround->Around[iCnt].iY;
		ReleaseSRWLockShared(&g_Sector[sectorY][sectorX].srwSector);
	}
}

#pragma warning(disable : 6001)
void GetNewSector(MOVE_DIR dir, st_SECTOR_AROUND* pOutSectorAround, st_SECTOR_POS pos)
{
	MOVE_DIR sectorPosArr[5];
	BYTE byCnt = 0;

	int tempSectorX;
	int tempSectorY;

	switch (dir)
	{
	case MOVE_DIR_LL:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_LL;
		sectorPosArr[2] = MOVE_DIR_LD;
		byCnt = 3;
		break;
	case MOVE_DIR_LU:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_UU;
		sectorPosArr[2] = MOVE_DIR_RU;
		sectorPosArr[3] = MOVE_DIR_LL;
		sectorPosArr[4] = MOVE_DIR_LD;
		byCnt = 5;
		break;
	case MOVE_DIR_UU:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_UU;
		sectorPosArr[2] = MOVE_DIR_RU;
		byCnt = 3;
		break;
	case MOVE_DIR_RU:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_UU;
		sectorPosArr[2] = MOVE_DIR_RU;
		sectorPosArr[3] = MOVE_DIR_RR;
		sectorPosArr[4] = MOVE_DIR_RD;
		byCnt = 5;
		break;
	case MOVE_DIR_RR:
		sectorPosArr[0] = MOVE_DIR_RU;
		sectorPosArr[1] = MOVE_DIR_RR;
		sectorPosArr[2] = MOVE_DIR_RD;
		byCnt = 3;
		break;
	case MOVE_DIR_RD:
		sectorPosArr[0] = MOVE_DIR_RU;
		sectorPosArr[1] = MOVE_DIR_RR;
		sectorPosArr[2] = MOVE_DIR_LD;
		sectorPosArr[3] = MOVE_DIR_DD;
		sectorPosArr[4] = MOVE_DIR_RD;
		byCnt = 5;
		break;
	case MOVE_DIR_DD:
		sectorPosArr[0] = MOVE_DIR_LD;
		sectorPosArr[1] = MOVE_DIR_DD;
		sectorPosArr[2] = MOVE_DIR_RD;
		byCnt = 3;
		break;
	case MOVE_DIR_LD:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_LL;
		sectorPosArr[2] = MOVE_DIR_LD;
		sectorPosArr[3] = MOVE_DIR_DD;
		sectorPosArr[4] = MOVE_DIR_RD;
		byCnt = 5;
		break;
	case MOVE_DIR_NOMOVE:
		__debugbreak();
		break;
	default:
		__debugbreak();
		break;
	}

	BYTE bySectorPosCnt = 0;
	for (BYTE i = 0; i < byCnt; ++i)
	{

		tempSectorY = pos.iY + vArr[sectorPosArr[i]].shY;
		tempSectorX = pos.iX + vArr[sectorPosArr[i]].shX;


		if (InValidPos(tempSectorX, tempSectorY))
		{
			pOutSectorAround->Around[bySectorPosCnt].iX = tempSectorX;
			pOutSectorAround->Around[bySectorPosCnt].iY = tempSectorY;
			++bySectorPosCnt;
		}
	}
	pOutSectorAround->iCount = bySectorPosCnt;
}

void GetRemoveSector(MOVE_DIR dir, st_SECTOR_AROUND* pOutSectorAround, st_SECTOR_POS pos)
{
	MOVE_DIR sectorPosArr[5];
	BYTE byCnt = 0;

	int tempSectorX;
	int tempSectorY;

	switch (dir)
	{
	case MOVE_DIR_LL:
		sectorPosArr[0] = MOVE_DIR_RU;
		sectorPosArr[1] = MOVE_DIR_RR;
		sectorPosArr[2] = MOVE_DIR_RD;
		byCnt = 3;
		break;
	case MOVE_DIR_LU:
		sectorPosArr[0] = MOVE_DIR_RU;
		sectorPosArr[1] = MOVE_DIR_RR;
		sectorPosArr[2] = MOVE_DIR_LD;
		sectorPosArr[3] = MOVE_DIR_LL;
		sectorPosArr[4] = MOVE_DIR_RD;
		byCnt = 5;
		break;
	case MOVE_DIR_UU:
		sectorPosArr[0] = MOVE_DIR_LD;
		sectorPosArr[1] = MOVE_DIR_DD;
		sectorPosArr[2] = MOVE_DIR_RD;
		byCnt = 3;
		break;
	case MOVE_DIR_RU:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_LL;
		sectorPosArr[2] = MOVE_DIR_LD;
		sectorPosArr[3] = MOVE_DIR_DD;
		sectorPosArr[4] = MOVE_DIR_RD;
		byCnt = 5;
		break;
	case MOVE_DIR_RR:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_LL;
		sectorPosArr[2] = MOVE_DIR_LD;
		byCnt = 3;
		break;
	case MOVE_DIR_RD:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_UU;
		sectorPosArr[2] = MOVE_DIR_RU;
		sectorPosArr[3] = MOVE_DIR_LL;
		sectorPosArr[4] = MOVE_DIR_LD;
		byCnt = 5;
		break;
	case MOVE_DIR_DD:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_UU;
		sectorPosArr[2] = MOVE_DIR_RU;
		byCnt = 3;
		break;
	case MOVE_DIR_LD:
		sectorPosArr[0] = MOVE_DIR_LU;
		sectorPosArr[1] = MOVE_DIR_UU;
		sectorPosArr[2] = MOVE_DIR_RU;
		sectorPosArr[3] = MOVE_DIR_RR;
		sectorPosArr[4] = MOVE_DIR_RD;
		byCnt = 5;
		break;
	case MOVE_DIR_NOMOVE:
		__debugbreak();
		break;
	default:
		__debugbreak();
		break;
	}

	BYTE bySectorPosCnt = 0;
	for (BYTE i = 0; i < byCnt; ++i)
	{
		tempSectorY = pos.iY + vArr[sectorPosArr[i]].shY;
		tempSectorX = pos.iX + vArr[sectorPosArr[i]].shX;
		if (InValidPos(tempSectorX, tempSectorY))
		{
			pOutSectorAround->Around[bySectorPosCnt].iX = tempSectorX;
			pOutSectorAround->Around[bySectorPosCnt].iY = tempSectorY;
			++bySectorPosCnt;
		}
	}
	pOutSectorAround->iCount = bySectorPosCnt;
}
#pragma warning(default : 6001)
void DirLock(st_CLIENT *pClient, st_SECTOR_AROUND* pNewSector, st_SECTOR_AROUND* pRemoveSector)
{
	//
	// 현재는 뱡향에 따라 구분해서 간단하게 락을 거는데, 데드락 발생 여부는 몰라서 다른데서는 이렇게 하면 안됨
	switch ((MOVE_DIR)pClient->_byMoveDirection)
	{
	case MOVE_DIR_LL:
	case MOVE_DIR_LU:
	case MOVE_DIR_UU:
	case MOVE_DIR_RU:
		LockSectorAroundShared(pNewSector);
		LockSectorAroundExclusive(pClient->_CurSector);
		LockSectorAroundExclusive(pClient->_OldSector);
		LockSectorAroundShared(pRemoveSector);
		break;

	case MOVE_DIR_RR:
	case MOVE_DIR_RD:
	case MOVE_DIR_DD:
	case MOVE_DIR_LD:
		LockSectorAroundShared(pRemoveSector);
		LockSectorAroundExclusive(pClient->_OldSector);
		LockSectorAroundExclusive(pClient->_CurSector);
		LockSectorAroundShared(pNewSector);
		break;
	case MOVE_DIR_NOMOVE:
		break;
	default:
		break;
	}
}

void DirUnLock(st_CLIENT* pClient, st_SECTOR_AROUND* pNewSector, st_SECTOR_AROUND* pRemoveSector)
{
	//
	// 현재는 뱡향에 따라 구분해서 간단하게 락을 거는데, 데드락 발생 여부는 몰라서 다른데서는 이렇게 하면 안됨
	switch ((MOVE_DIR)pClient->_byMoveDirection)
	{
	case MOVE_DIR_LL:
	case MOVE_DIR_LU:
	case MOVE_DIR_UU:
	case MOVE_DIR_RU:
		UnLockSectorAroundShared(pNewSector);
		UnLockSectorAroundExclusive(pClient->_CurSector);
		UnLockSectorAroundExclusive(pClient->_OldSector);
		UnLockSectorAroundShared(pRemoveSector);
		break;

	case MOVE_DIR_RR:
	case MOVE_DIR_RD:
	case MOVE_DIR_DD:
	case MOVE_DIR_LD:
		UnLockSectorAroundShared(pRemoveSector);
		UnLockSectorAroundExclusive(pClient->_OldSector);
		UnLockSectorAroundExclusive(pClient->_CurSector);
		UnLockSectorAroundShared(pNewSector);
		break;
	case MOVE_DIR_NOMOVE:
		break;
	default:
		break;
	}
}

void LOCK_CharacterSectorUpdatePacket(st_CLIENT* pClient)
{
	//---------------------------------------------------------------------------------------------------------
	// 0. 주변 섹터를 얻어온다.
	// 1. 락을 건다. 
	// 2. 삭제 메시지 보낸다.	-> pClient도 주변 섹터의 삭제 메시지를 받아야함
	// 3. 섹터에서 지운다.		-> 락 걸려 있으니까 나중에 한번에 몰아서 한다. 
	// 4. 새로운 섹터에 생성	
	// 5. 생성 메시지를 보낸다. -> pClient에게도 플레이어 생성 메시지를 받아야 한다.

	st_SECTOR_AROUND newSectorAround;
	st_SECTOR_AROUND removeSectorAround;
	st_CLIENT* pTempClient;
	int iCnt;

	GetNewSector((MOVE_DIR)pClient->_byMoveDirection, &newSectorAround, pClient->_CurSector);
	GetRemoveSector((MOVE_DIR)pClient->_byMoveDirection, &removeSectorAround, pClient->_OldSector);
	
	DirLock(pClient, &newSectorAround, &removeSectorAround);

	g_MMOServer.SendPacketAround(&removeSectorAround, pClient, (char)dfPACKET_SC_DELETE_CHATACTER);
	g_MMOServer.SendPacketAroundToMe(&removeSectorAround, pClient, (char)dfPACKET_SC_DELETE_CHATACTER);

	g_MMOServer.SendPacketAround(&newSectorAround, pClient, (char)dfPACKET_SC_CREATE_OTHER_CHARACTER, false, true);
	g_MMOServer.SendPacketAroundToMe(&newSectorAround, pClient, (char)dfPACKET_SC_CREATE_OTHER_CHARACTER, true);



	DirUnLock(pClient, &newSectorAround, &removeSectorAround);
}




void GetSectorAround(st_SECTOR_POS sector, st_SECTOR_AROUND* pSectorAround)
{
	int iCnt;
	int nSectorX;
	int nSectorY;
	int iCount;

	iCount = 0;

	for (iCnt = 0; iCnt < 9; ++iCnt)
	{
		nSectorX = sector.iX + dx[iCnt];
		nSectorY = sector.iY + dy[iCnt];

		if (InValidPos(nSectorX, nSectorY))
		{
			pSectorAround->Around[iCount].iX = nSectorX;
			pSectorAround->Around[iCount].iY = nSectorY;
			++iCount;
		}
	}

	pSectorAround->iCount = iCount;
}

bool IsSectorUpdateCharacter(st_CLIENT* pClient)
{
	int SectorY;
	int SectorX;

	SectorX = pClient->_shX / dfGRID_X_SIZE;
	SectorY = pClient->_shY / dfGRID_Y_SIZE;

	if (pClient->_CurSector.iX != SectorX || pClient->_CurSector.iY != SectorY)
	{
		pClient->_OldSector.iX = pClient->_CurSector.iX;
		pClient->_OldSector.iY = pClient->_CurSector.iX;

		pClient->_CurSector.iX = SectorX;
		pClient->_CurSector.iY = SectorY;

		return true;
	}

	return false;
}





