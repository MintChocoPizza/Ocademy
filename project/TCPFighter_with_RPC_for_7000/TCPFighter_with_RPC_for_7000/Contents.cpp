

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
	//// 50�������� �����. 
	//// �׷��� ���ؼ� 
	//// 1�� == 1000ms
	//// 
	//// ��� 20ms���� 1���� ���ƾ� �Ѵ�.
	//// ���ι����� ����.
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
		// ���ӵ��� ���� ���� ���̵�
		if (g_CharacterArr[iSessionID] == NULL)
			continue;

		//------------------------------------------
		// ������ �������� �ϴ� �÷��̾�� �ǳ� �ڴ�.
		if (DisconnectCheck(iSessionID))
			continue;

		st_p_Player = g_CharacterArr[iSessionID];
		
		//------------------------------------------
		// HP == 0 ���ó��
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
		// ���� ���ۿ� ���� ó��
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
			// �̻��� ������ ������ ���´�.
			//_LOG(dfLOG_LEVEL_SYSTEM, L"#Update error!!! Delete - SessionID:%d\n", iSessionID);
			LOG(L"Disconnect", LEVEL_SYSTEM, false, true, L"#Update error!!! Delete - SessionID:%d\n", iSessionID);
			enqueueForDeletion(iSessionID);
			return;
			}
		}
	
		//--------------------------------------------------------------------------------------
		// �̵� �׼��� ���, ���͸� üũ�Ͽ� �������ش�. 
		//--------------------------------------------------------------------------------------
		// �̵��� ��� ���� ������Ʈ�� ��.
		if (C_Field::GetInstance()->Sector_UpdateCharacter(st_p_Player))
		{
			// ���Ͱ� ����� ���� Ŭ�󿡰� ���� ������ ���. 
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
