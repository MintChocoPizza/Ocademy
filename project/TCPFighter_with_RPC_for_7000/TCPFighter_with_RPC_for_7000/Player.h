
#ifndef __PALYER_H__
#define __PALYER_H__


#include "Cdebug.h"

struct st_SECTOR_POS;

struct st_PLAYER
{
	st_SESSION* _pSession;
	DWORD		_SessionID;

	DWORD		_dwAction;
	BYTE		_byDirection;
	BYTE		_byMoveDirection;	// 아마도 움직이던 방향


	short		_X;
	short		_Y;
	DWORD		_CurFPS;

#ifdef SYNC_DEBUG
	short		_PreX;
	short		_PreY;
	DWORD		_PreFPS;
	DWORD		_PreAction;
#endif // SYNC_DEBUG


	char		_HP;
#ifdef DISCONNECT_SERVER
	char		_PreHP;
#endif // DISCONNECT_SERVER

	// bool		_Disconnect;

	// 구현에 따라 다르다.
	st_SECTOR_POS* _CurSector;
	st_SECTOR_POS* _OldSector;

	st_PLAYER();
	st_PLAYER(DWORD SessionID, st_SESSION* pSession);
	~st_PLAYER();
};

// Key: SessionID, Value: st_Player*
// extern std::unordered_map< DWORD, st_PLAYER*>	g_CharacterHash;

extern st_PLAYER* g_CharacterArr[8001];


void Init_CharacterArr(void);
void ClearCharacterHash(void);
st_PLAYER* FindCharacter(DWORD dwSessionID);
st_PLAYER* CreateNewPlayer(DWORD SessionID, st_SESSION* st_p_New_Session);
void DeleteCharacter(DWORD dwSessionID);



#endif // !__PALYER_H__
