#pragma once

#define dfGRID_X_SIZE	160
#define dfGRID_Y_SIZE	120

constexpr int dfSECTOR_MAX_Y = (dfRANGE_MOVE_BOTTOM / dfGRID_Y_SIZE) + 1;
constexpr int dfSECTOR_MAX_X = (dfRANGE_MOVE_RIGHT / dfGRID_X_SIZE) + 1;


struct st_CLIENT;

struct st_SECTOR_CLIENT_INFO
{
	CList<st_CLIENT*> client_List;
	SRWLOCK srwSector;

	st_SECTOR_CLIENT_INFO()
	{
		InitializeSRWLock(&srwSector);
	}
};

struct st_SECTOR_POS
{
	int iX;
	int iY;

	void Init_SECTOR_POS(int X, int Y);
};

struct st_SECTOR_AROUND
{
	int iCount;
	st_SECTOR_POS Around[10];
};

void LockSectorAroundShared(st_SECTOR_POS sector);
void UnLockSectorAroundShared(st_SECTOR_POS sector);
void LockSectorAroundExclusive(st_SECTOR_POS sector);
void UnLockSectorAroundExclusive(st_SECTOR_POS sector);
void LockSectorAroundShared(st_SECTOR_AROUND* pSectorAround);
void UnLockSectorAroundShared(st_SECTOR_AROUND* pSectorAround);

void GetNewSector(MOVE_DIR dir, st_SECTOR_AROUND* pOutSectorAround, st_SECTOR_POS pos);
void GetRemoveSector(MOVE_DIR dir, st_SECTOR_AROUND* pOutSectorAround, st_SECTOR_POS pos);
void DirLock(st_CLIENT* pClient, st_SECTOR_AROUND* pNewSector, st_SECTOR_AROUND* pRemoveSector);
void DirUnLock(st_CLIENT* pClient, st_SECTOR_AROUND* pNewSector, st_SECTOR_AROUND* pRemoveSector);

void LOCK_CharacterSectorUpdatePacket(st_CLIENT* pClient);


void GetSectorAround(st_SECTOR_POS sector, st_SECTOR_AROUND* pSectorAround);
bool IsSectorUpdateCharacter(st_CLIENT* pClient);


static __forceinline bool InValidPos(st_SECTOR_POS sectorPos)
{

}
static __forceinline bool InValidPos(int sectorX, int sectorY)
{
	bool bValidX;
	bool bValidY;

	bValidX = (dfRANGE_MOVE_LEFT <= sectorX && sectorX < dfRANGE_MOVE_RIGHT);
	bValidY = (dfRANGE_MOVE_TOP <= sectorY && sectorY < dfRANGE_MOVE_BOTTOM);

	return (bValidX && bValidY);
}
