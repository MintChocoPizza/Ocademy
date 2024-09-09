
#include <Windows.h>
#include <list>
#include <unordered_map>

#include <string>

#include "CMemoryPool.h"
#include "Protocol.h"
#include "C_Ring_Buffer.h"
#include "SerializeBuffer.h"
#include "main.h"
#include "Session.h"
#include "Define.h"
#include "Player.h"
#include "CList.h"
#include "Field.h"

#include "Cdebug.h"


// std::unordered_map< DWORD, st_PLAYER*>	g_CharacterHash;
OreoPizza::CMemoryPool<st_PLAYER> st_PLAYER_MemPool(dfMAX_SESSION, FALSE);
st_PLAYER* g_CharacterArr[dfMAX_SESSION];


#pragma warning( disable : 26495 )
st_PLAYER::st_PLAYER()
{
	_CurSector = new st_SECTOR_POS;
	_OldSector = new st_SECTOR_POS;
}
#pragma warning( default : 26495 )

#pragma warning( disable : 26495 )
st_PLAYER::st_PLAYER(DWORD SessionID, st_SESSION* pSession)
{
	//_pSession = pSession;
	//_SessionID = SessionID;
	//_Disconnect = false;
	//_HP = 100;
	//_dwAction = -1;

	//// rand() % �ִ밪 + �ּڰ�
	//// 0 < _Y < 6401
	//// 0 < _X < 6401
	//_Y = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP + 1) + dfRANGE_MOVE_TOP;
	//_X = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT + 1) + dfRANGE_MOVE_LEFT;

	_CurSector = new st_SECTOR_POS;
	_OldSector = new st_SECTOR_POS;


	//_CurSector->Init_SECTOR_POS(_Y, _X);
	//_OldSector->Init_SECTOR_POS(_Y, _X);

	//_byDirection = (rand() % 8) < 4 ? 0 : 4;
	//_byModeDirection = -1;
}
#pragma warning( default : 26495 )

st_PLAYER::~st_PLAYER()
{
	//--------------------------------------
	// �Ҹ��ڿ��� ���Ϳ� ���� �޸𸮸� ����� �ȴٸ�
	// �޸� Ǯ free���� �Ҹ��ڸ� ȣ���Ͽ� �޸𸮰� �������. 
	// ���⼭ �޸� �������� ������? �׷� �޸� Ǯ������ ��� �޸𸮸� �����ϳ���?
	// �޸� Ǯ�� �Ҹ�Ǹ鼭 ������ �Ҹ��ڸ� ȣ���ϰ� �ȴ�.
	// �׷��Ƿ� �Ҹ��ڿ� �޸𸮿� ���� ������ �־�� �Ѵ�. 

	delete _CurSector;
	delete _OldSector;
}

void Init_CharacterArr(void)
{
	ZeroMemory(g_CharacterArr, dfMAX_SESSION);
}

void ClearCharacterHash(void)
{
	st_PLAYER* st_Delete_Player;
	int iCnt;

	for (iCnt = 0; iCnt < dfMAX_SESSION; ++iCnt)
	{
		st_Delete_Player = g_CharacterArr[iCnt];

		delete st_Delete_Player->_CurSector;
		delete st_Delete_Player->_OldSector;
		//delete st_Delete_Player;

		ZeroMemory(st_Delete_Player, sizeof(st_PLAYER));

		st_PLAYER_MemPool.Free(st_Delete_Player);

		g_CharacterArr[iCnt] = NULL;
	}
}

st_PLAYER* FindCharacter(DWORD dwSessionID)
{	
	return g_CharacterArr[dwSessionID];
}

st_PLAYER* CreateNewPlayer(DWORD SessionID, st_SESSION* st_p_New_Session)
{
	// st_PLAYER* st_Temp_New_Player = new st_PLAYER(SessionID, st_p_New_Session);

	st_PLAYER* st_Temp_New_Player;

	//--------------------------------------------------------------------------------------------------------------------
	// �����ڿ��� �ʱ�ȭ ���� �ʰ�, ����ؼ� ���� �� �ʱ�ȭ �ؼ� �ǳ��ش�. 
	st_Temp_New_Player = st_PLAYER_MemPool.Alloc();
	// new(st_Temp_New_Player) st_PLAYER(SessionID, st_p_New_Session);

	st_Temp_New_Player->_pSession = st_p_New_Session;
	st_Temp_New_Player->_SessionID = SessionID;
	//st_Temp_New_Player->_Disconnect = false;
	st_Temp_New_Player->_HP = 100;
	st_Temp_New_Player->_PreHP = 100;
	st_Temp_New_Player->_dwAction = dfPACKET_CS_MOVE_STOP;

#ifdef _DEBUG
	st_Temp_New_Player->_Y = 50;
	st_Temp_New_Player->_X = 50;
#else
	st_Temp_New_Player->_Y = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP + 1) + dfRANGE_MOVE_TOP;
	st_Temp_New_Player->_PreY = st_Temp_New_Player->_Y;

	st_Temp_New_Player->_X = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT + 1) + dfRANGE_MOVE_LEFT;
	st_Temp_New_Player->_PreX = st_Temp_New_Player->_X;
#endif // _DEBUG



	st_Temp_New_Player->_CurSector->Init_SECTOR_POS(st_Temp_New_Player->_Y, st_Temp_New_Player->_X);
	st_Temp_New_Player->_OldSector->Init_SECTOR_POS(st_Temp_New_Player->_Y, st_Temp_New_Player->_X);

	st_Temp_New_Player->_byDirection = 0;
	st_Temp_New_Player->_byMoveDirection = dfPACKET_CS_MOVE_STOP;


#ifdef SYNC_DEBUG
	st_Temp_New_Player->_PreX;
	st_Temp_New_Player->_PreY = 0;
	st_Temp_New_Player->_PreFPS = 0;
	st_Temp_New_Player->_PreAction = -1;
#endif // SYNC_DEBUG

#ifdef DISCONNECT_SERVER
	st_Temp_New_Player->_PreHP = 100;
#endif // DISCONNECT_SERVER


	g_CharacterArr[SessionID] = st_Temp_New_Player;



	return st_Temp_New_Player;
}

void DeleteCharacter(DWORD dwSessionID)
{
	st_PLAYER* st_p_Player;

	st_p_Player = g_CharacterArr[dwSessionID];



	//delete st_p_Player;
	st_PLAYER_MemPool.Free(st_p_Player);
	g_CharacterArr[dwSessionID] = NULL;
}



