

#include <unordered_map>
#include <Windows.h>

#include <string>

#include "LOG.h"
#include "Contents.h"
#include "Define.h"
#include "SerializeBuffer.h"
#include "Disconnect.h"
#include "main.h"
#include "Protocol.h"
#include "C_Ring_Buffer.h"
#include "Session.h"
#include "CList.h"
#include "Field.h"
#include "Player.h"


   //#define UPDATE_DEBUG

void Update(void)
{
	st_PLAYER* st_p_Player;
	int iSessionID;
	//std::unordered_map<DWORD, st_PLAYER*>::iterator iter;
	//DWORD dwCurrentTick;

	///////////////////////////////////////////////////////////////////////////
	//// 50프레임을 맞춘다. 
	//// 그러기 위해서 
	//// 1초 == 1000ms
	//// 
	//// 고로 20ms마다 1번씩 돌아야 한다.
	//// 메인문으로 뺀다.
	///////////////////////////////////////////////////////////////////////////
	//dwCurrentTick = timeGetTime();
	//if (dwCurrentTick - g_Start_Time < 20)
	//{
	//	return;
	//}
	//g_Start_Time += 20;
	
	for (iSessionID = 0; iSessionID < dfMAX_SESSION; ++iSessionID)
	{
		//------------------------------------------
		// 접속되지 않은 세션 아이디
		if (g_CharacterArr[iSessionID] == NULL)
			continue;

		//------------------------------------------
		// 연결이 끊어져야 하는 플레이어는 건너 뛴다.
		if (DisconnectCheck(iSessionID))
			continue;

		st_p_Player = g_CharacterArr[iSessionID];
		
		//------------------------------------------
		// HP == 0 사망처리
		if (st_p_Player->_HP <= 0)
		{
			//_LOG(dfLOG_LEVEL_DEBUG, L"Player HP Zero # SessionID:%d", st_p_Player->_SessionID);
			//LOG(L"Disconnect", LEVEL_SYSTEM, false, true, L"Player HP Zero # SessionID:%d / HP:%d / PreHP:%d", st_p_Player->_SessionID, st_p_Player->_HP, st_p_Player->_PreHP);
#ifdef DISCONNECT_SERVER
			LOG(L"Disconnect", DEBUG, TEXTFILE, L"Player HP Zero # SessionID:%d / HP:%d / PreHP:%d", st_p_Player->_SessionID, st_p_Player->_HP, st_p_Player->_PreHP);
#endif // 

			enqueueForDeletion(iSessionID);
			continue;
		}


		//--------------------------------------------------------------------------------------
		// 현재 동작에 따른 처리
		//--------------------------------------------------------------------------------------
		switch (st_p_Player->_dwAction)
		{
		case dfPACKET_MOVE_DIR_LL:
			if (CharacterMoveCheck(st_p_Player->_X - dfSPEED_PLAYER_X, st_p_Player->_Y))
			{
				st_p_Player->_X -= dfSPEED_PLAYER_X;
			}
			break;
		case dfPACKET_MOVE_DIR_LU:
			if (CharacterMoveCheck(st_p_Player->_X - dfSPEED_PLAYER_X, st_p_Player->_Y - dfSPEED_PLAYER_Y))
			{
				st_p_Player->_X -= dfSPEED_PLAYER_X;
				st_p_Player->_Y -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_UU:
			if (CharacterMoveCheck(st_p_Player->_X, st_p_Player->_Y - dfSPEED_PLAYER_Y))
			{
				st_p_Player->_Y -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_RU:
			if (CharacterMoveCheck(st_p_Player->_X + dfSPEED_PLAYER_X, st_p_Player->_Y - dfSPEED_PLAYER_Y))
			{
				st_p_Player->_X += dfSPEED_PLAYER_X;
				st_p_Player->_Y -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_RR:
			if (CharacterMoveCheck(st_p_Player->_X + dfSPEED_PLAYER_X, st_p_Player->_Y))
			{
				st_p_Player->_X += dfSPEED_PLAYER_X;
			}
			break;
		case dfPACKET_MOVE_DIR_RD:
			if (CharacterMoveCheck(st_p_Player->_X + dfSPEED_PLAYER_X, st_p_Player->_Y + dfSPEED_PLAYER_Y))
			{
				st_p_Player->_X += dfSPEED_PLAYER_X;
				st_p_Player->_Y += dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_DD:
			if (CharacterMoveCheck(st_p_Player->_X, st_p_Player->_Y + dfSPEED_PLAYER_Y))
			{
				st_p_Player->_Y += dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_LD:
			if (CharacterMoveCheck(st_p_Player->_X - dfSPEED_PLAYER_X, st_p_Player->_Y + dfSPEED_PLAYER_Y))
			{
				st_p_Player->_X -= dfSPEED_PLAYER_X;
				st_p_Player->_Y += dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_CS_MOVE_STOP:
			break;

		default:
			{
			// 이상함 유저의 연결을 끊는다.
			//_LOG(dfLOG_LEVEL_SYSTEM, L"#Update error!!! Delete - SessionID:%d\n", iSessionID);
			LOG(L"Disconnect", LEVEL_SYSTEM, false, true, L"#Update error!!! Delete - SessionID:%d\n", iSessionID);
			enqueueForDeletion(iSessionID);
			return;
			}
		}
	
		//--------------------------------------------------------------------------------------
		// 이동 액션인 경우, 섹터를 체크하여 변경해준다. 
		//--------------------------------------------------------------------------------------
		// 이동인 경우 섹터 업데이트를 함.
		if (C_Field::GetInstance()->Sector_UpdateCharacter(st_p_Player))
		{
			// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다. 
			C_Field::GetInstance()->CharacterSectorUpdatePacket(st_p_Player);
		}
	}
}

bool CharacterMoveCheck(int SectorX, int SectorY)
{
	if (SectorX < dfRANGE_MOVE_LEFT || dfRANGE_MOVE_RIGHT < SectorX || SectorY < dfRANGE_MOVE_TOP || dfRANGE_MOVE_BOTTOM < SectorY)
		return false;

	return true;
}
