
#include <map>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")

#include <stdio.h>

#include "C_Ring_Buffer.h"
#include "CList.h"
#include "SerializeBuffer.h"
#include "Set_SerializeBuffer.h"
#include "NetWork.h"
#include "PacketDefine.h"
#include "Set_Log.h"

//---------------------------------------------------------------
// 헤더와 패킷이 모두 char* 이다. 
// 
// 큰 의미 없다. 그냥 보기 예쁘게 맞춘것이다.
//---------------------------------------------------------------


#ifdef SERIALIZEBUFFER
bool SerializeBuffer_netPacketProc_SC_CREATE_MY_CHARACTER(st_SESSION* p_Session, SerializeBuffer* clpPacket)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = sizeof(st_PACKET_SC_CREATE_MY_CHARACTER);
	stPacketHeader.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	clpPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));

	*clpPacket << p_Session->dwSessionID;
	*clpPacket << p_Session->byDirection;
	*clpPacket << p_Session->shX;
	*clpPacket << p_Session->shY;
	*clpPacket << p_Session->chHP;

	return true;
}
bool SerializeBuffer_netPacketProc_SC_CREATE_OTHER_CHARACTER(st_SESSION* p_Session, SerializeBuffer* clpPacket)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = sizeof(st_PACKET_SC_CREATE_OTHER_CHARACTER);
	stPacketHeader.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	clpPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));

	*clpPacket << p_Session->dwSessionID;
	*clpPacket << p_Session->byDirection;
	*clpPacket << p_Session->shX;
	*clpPacket << p_Session->shY;
	*clpPacket << p_Session->chHP;

	return true;
}

bool SerializeBuffer_netPacketProc_SC_DELETE_CHARACTER(st_SESSION* p_Session, SerializeBuffer* clpPacket)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = sizeof(st_PACKET_SC_DELETE_CHARACTER);
	stPacketHeader.byType = dfPACKET_SC_DELETE_CHARACTER;
	clpPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));

	*clpPacket << p_Session->dwSessionID;
	return true;
}

bool SerializeBuffer_netPacketProc_CS_MOVE_START(st_SESSION* p_Session, SerializeBuffer* clpPacket)
{
	st_PACKET_HEADER Header;
	st_PACKET_SC_MOVE_START SendMsg;
	st_PACKET_CS_MOVE_START* pMoveStart = (st_PACKET_CS_MOVE_START*)(clpPacket->GetBufferPtr());
	SerializeBuffer clPacket;

	// 메시지 수신 로그 확인
	//---------------------------------------------------------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 끊어버림
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음.
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함.
	// 지금은 간단한 구현을 목적으로 하고 있으므로 오차범위 내에서 클라이언트 좌표를 믿도록 한다. 
	//---------------------------------------------------------------------------------------------------

	if (abs(p_Session->shX - pMoveStart->X) > dfERROR_RANGE ||
		abs(p_Session->shY - pMoveStart->Y) > dfERROR_RANGE)
	{
		PushDisconnectList(p_Session);
#ifdef DEFAULT_LOG
		printf_s("# Error Log: CS_MOVE_START - SessionID: %d : dfERROR_RANGE \n", p_Session->dwSessionID);
		printf_s("## Server X: %d, Y: %d, Client X: %d, Y: %d \n", p_Session->shX, p_Session->shY, pMoveStart->X, pMoveStart->Y);
#endif // DEFAULT_LOG
		return false;
	}

	//---------------------------------------------------------------------------------------------------
	// 동작을 변경. 지금 구현에선 동작번호가 방향값이다.
	//---------------------------------------------------------------------------------------------------
	p_Session->dwAction = pMoveStart->Direction;

	//---------------------------------------------------------------------------------------------------
	// 방향을 변경. 아니면 기존의 방향을 유지하는 결정을 내린다.
	//---------------------------------------------------------------------------------------------------
	switch (pMoveStart->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		p_Session->byDirection = dfPACKET_MOVE_DIR_RR;
		break;

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		p_Session->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	default:
		break;
	}

	// 서버의 위치와 실제 클라의 위치가 틀어진경우, 현재 코드에서는 클라의 위치를 믿고 있다. 
	p_Session->shX = pMoveStart->X;
	p_Session->shY = pMoveStart->Y;

	//---------------------------------------------------------------------------------------------------
	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. 당사자 제외
	//---------------------------------------------------------------------------------------------------
	SerializeBuffer_netPacketProc_SC_MOVE_START(&clPacket, pMoveStart->Direction, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	SerializeBuffer_netSendBroadcast(p_Session, &clPacket);

	return true;
}
bool SerializeBuffer_netPacketProc_SC_MOVE_START(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = sizeof(st_PACKET_SC_MOVE_START);
	stPacketHeader.byType = dfPACKET_SC_MOVE_START;
	clpPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));

	*clpPacket << ID;
	*clpPacket << Direction;
	*clpPacket << X;
	*clpPacket << Y;

	return true;
}

bool SerializeBuffer_netPacketProc_CS_MOVE_STOP(st_SESSION* p_Session, SerializeBuffer* clpPacket)
{
	st_PACKET_HEADER Header;
	st_PACKET_SC_MOVE_STOP SendMsg;
	st_PACKET_CS_MOVE_STOP* pMoveStop = (st_PACKET_CS_MOVE_STOP*)clpPacket->GetBufferPtr();
	SerializeBuffer clPacket;

	if (abs(p_Session->shX - pMoveStop->X) > dfERROR_RANGE ||
		abs(p_Session->shY - pMoveStop->Y) > dfERROR_RANGE)
	{
		PushDisconnectList(p_Session);
#ifdef DEFAULT_LOG
		printf_s("# Error Log: CS_MOVE_STOP - SessionID: %d : dfERROR_RANGE \n", p_Session->dwSessionID);
#endif // DEFAULT_LOG
		return false;
	}

	//---------------------------------------------------------------------------------------------------
	// 동작을 변경. 지금 구현에선 동작번호가 방향값이다.
	// 
	// 하지만 정지의 경우 방향값을 그대로 넣어버리면 캐릭터가 정지를 안한다.
	//---------------------------------------------------------------------------------------------------
	p_Session->dwAction = dfPACKET_CS_MOVE_STOP;

	//---------------------------------------------------------------------------------------------------
	// 방향을 변경. 아니면 기존의 방향을 유지하는 결정을 내린다.
	// 
	// 하지만 현재 상황에서는 이동하던 유지하던 방향을 그대로 중지하여 변경할 일이 없다.
	// 
	//---------------------------------------------------------------------------------------------------
	switch (pMoveStop->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		p_Session->byDirection = dfPACKET_MOVE_DIR_RR;
		break;

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		p_Session->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	default:
		break;
	}
	// 서버의 위치와 실제 클라의 위치가 틀어진경우, 현재 코드에서는 클라의 위치를 믿고 있다. 
	p_Session->shX = pMoveStop->X;
	p_Session->shY = pMoveStop->Y;

	//---------------------------------------------------------------------------------------------------
	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. 당사자 제외
	//---------------------------------------------------------------------------------------------------

	SerializeBuffer_netPacketProc_SC_MOVE_STOP(&clPacket, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	SerializeBuffer_netSendBroadcast(p_Session, &clPacket);

	return true;
}

bool SerializeBuffer_netPacketProc_SC_MOVE_STOP(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = sizeof(st_PACKET_SC_MOVE_STOP);
	stPacketHeader.byType = dfPACKET_SC_MOVE_STOP;
	clpPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));

	*clpPacket << ID;
	*clpPacket << Direction;
	*clpPacket << X;
	*clpPacket << Y;
	return true;
}

bool SerializeBuffer_netPacketProc_CS_ATTACK1(st_SESSION* p_Session, SerializeBuffer* clpPacket)
{
	// 공격 하는 순간 판정까지 떨어진다.
	st_PACKET_CS_ATTACK1* pTempPacket;
	st_SESSION* p_Temp_Session;
	st_PACKET_HEADER header_SC_ATTACK1;
	st_PACKET_HEADER header_SC_DAMAGE;
	st_PACKET_SC_ATTACK1 packet_SC_ATTACK1;
	st_PACKET_SC_DAMAGE packet_SC_DAMGE;
	std::map<DWORD, st_SESSION*>::iterator iter;
	SerializeBuffer clPacket;
	SerializeBuffer clPacket2;
	short temp_X;
	short temp_Y;



	// 공격 모션에 대한 패킷은 모두에게 보낸다.
	pTempPacket = (st_PACKET_CS_ATTACK1*)clpPacket->GetBufferPtr();

	SerializeBuffer_netPacketProc_SC_ATTACK1(&clPacket, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	SerializeBuffer_netSendBroadcast(p_Session, &clPacket);

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// 오른쪽 타격 (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK1_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack1 Direction:RR /  SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				SerializeBuffer_netPacketProc_SC_DAMAGE(&clPacket2, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				SerializeBuffer_netSendBroadcast(NULL, &clPacket2);
				break;
			}
		}

		break;
	}

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
	{
		// 왼쪽 타격  (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK1_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack1 Direction:LL / SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				SerializeBuffer_netPacketProc_SC_DAMAGE(&clPacket2, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				SerializeBuffer_netSendBroadcast(NULL, &clPacket2);
				break;
			}
		}
		break;
	}
	default:
		break;
	}

	return true;
}

bool SerializeBuffer_netPacketProc_SC_ATTACK1(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = sizeof(st_PACKET_SC_ATTACK1);
	stPacketHeader.byType = dfPACKET_SC_ATTACK1;
	clpPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));

	*clpPacket << ID;
	*clpPacket << Direction;
	*clpPacket << X;
	*clpPacket << Y;
	return true;
}

bool SerializeBuffer_netPacketProc_CS_ATTACK2(st_SESSION* p_Session, SerializeBuffer* clpPacket)
{
	// 공격 하는 순간 판정까지 떨어진다.
	st_PACKET_CS_ATTACK2* pTempPacket;
	st_SESSION* p_Temp_Session;
	st_PACKET_HEADER header_SC_ATTACK2;
	st_PACKET_HEADER header_SC_DAMAGE;
	st_PACKET_SC_ATTACK2 packet_SC_ATTACK2;
	st_PACKET_SC_DAMAGE packet_SC_DAMGE;
	std::map<DWORD, st_SESSION*>::iterator iter;
	SerializeBuffer clPacket;
	SerializeBuffer clPacket2;
	short temp_X;
	short temp_Y;



	// 공격 모션에 대한 패킷은 모두에게 보낸다.
	pTempPacket = (st_PACKET_CS_ATTACK2*)clpPacket->GetBufferPtr();

	SerializeBuffer_netPacketProc_SC_ATTACK2(&clPacket, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	SerializeBuffer_netSendBroadcast(p_Session, &clPacket);


	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// 오른쪽 타격 (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK2_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack2 Direction:RR /  SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				SerializeBuffer_netPacketProc_SC_DAMAGE(&clPacket2, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				SerializeBuffer_netSendBroadcast(NULL, &clPacket2);
				break;
			}
		}

		break;
	}

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
	{
		// 왼쪽 타격  (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK2_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack2 Direction:LL / SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				SerializeBuffer_netPacketProc_SC_DAMAGE(&clPacket2, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				SerializeBuffer_netSendBroadcast(NULL, &clPacket2);
				break;
			}
		}
		break;
	}
	default:
		break;
	}

	return true;
}

bool SerializeBuffer_netPacketProc_SC_ATTACK2(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = sizeof(st_PACKET_SC_ATTACK2);
	stPacketHeader.byType = dfPACKET_SC_ATTACK2;
	clpPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));

	*clpPacket << ID;
	*clpPacket << Direction;
	*clpPacket << X;
	*clpPacket << Y;
	return true;
}


bool SerializeBuffer_netPacketProc_CS_ATTACK3(st_SESSION* p_Session, SerializeBuffer* clpPacket)
{
	// 공격 하는 순간 판정까지 떨어진다.
	st_PACKET_CS_ATTACK3* pTempPacket;
	st_SESSION* p_Temp_Session;
	st_PACKET_HEADER header_SC_ATTACK3;
	st_PACKET_HEADER header_SC_DAMAGE;
	st_PACKET_SC_ATTACK3 packet_SC_ATTACK3;
	st_PACKET_SC_DAMAGE packet_SC_DAMGE;
	std::map<DWORD, st_SESSION*>::iterator iter;
	SerializeBuffer clPacket;
	SerializeBuffer clPacket2;
	short temp_X;
	short temp_Y;



	// 공격 모션에 대한 패킷은 모두에게 보낸다.
	pTempPacket = (st_PACKET_CS_ATTACK3*)clpPacket->GetBufferPtr();

	SerializeBuffer_netPacketProc_SC_ATTACK3(&clPacket, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	SerializeBuffer_netSendBroadcast(p_Session, &clPacket);

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// 오른쪽 타격 (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK3_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack3 Direction:RR /  SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				SerializeBuffer_netPacketProc_SC_DAMAGE(&clPacket2, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				SerializeBuffer_netSendBroadcast(NULL, &clPacket2);
				break;
			}
		}

		break;
	}

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
	{
		// 왼쪽 타격  (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK3_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack3 Direction:LL / SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				SerializeBuffer_netPacketProc_SC_DAMAGE(&clPacket2, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				SerializeBuffer_netSendBroadcast(NULL, &clPacket2);
				break;
			}
		}
		break;
	}
	default:
		break;
	}

	return true;
}

bool SerializeBuffer_netPacketProc_SC_ATTACK3(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = sizeof(st_PACKET_SC_ATTACK3);
	stPacketHeader.byType = dfPACKET_SC_ATTACK3;
	clpPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));

	*clpPacket << ID;
	*clpPacket << Direction;
	*clpPacket << X;
	*clpPacket << Y;
	return true;
}

bool  SerializeBuffer_netPacketProc_SC_DAMAGE(SerializeBuffer* clpPacket, __int32 AttackID, __int32 DamageID, char HP)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = sizeof(st_PACKET_SC_DAMAGE);
	stPacketHeader.byType = dfPACKET_SC_DAMAGE;
	clpPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));

	*clpPacket << AttackID;
	*clpPacket << DamageID;
	*clpPacket << HP;
	return true;
}



#else
bool netPacketProc_SC_CREATE_MY_CHARACTER(st_SESSION* p_Session, char* pHeader, char* pPacket)
{
	((st_PACKET_HEADER*)pHeader)->byCode = 0x89;
	((st_PACKET_HEADER*)pHeader)->bySize = sizeof(st_PACKET_SC_CREATE_MY_CHARACTER);
	((st_PACKET_HEADER*)pHeader)->byType = dfPACKET_SC_CREATE_MY_CHARACTER;

	((st_PACKET_SC_CREATE_MY_CHARACTER*)pPacket)->Direction = p_Session->byDirection;
	((st_PACKET_SC_CREATE_MY_CHARACTER*)pPacket)->HP = p_Session->chHP;
	((st_PACKET_SC_CREATE_MY_CHARACTER*)pPacket)->ID = p_Session->dwSessionID;
	((st_PACKET_SC_CREATE_MY_CHARACTER*)pPacket)->X = p_Session->shX;
	((st_PACKET_SC_CREATE_MY_CHARACTER*)pPacket)->Y = p_Session->shY;


	return true;
}

bool netPacketProc_SC_CREATE_OTHER_CHARACTER(st_SESSION* p_Session, char* pHeader, char* pPacket)
{
	((st_PACKET_HEADER*)pHeader)->byCode = dfNETWORK_PACKET_CODE;
	((st_PACKET_HEADER*)pHeader)->bySize = sizeof(st_PACKET_SC_CREATE_OTHER_CHARACTER);
	((st_PACKET_HEADER*)pHeader)->byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	((st_PACKET_SC_CREATE_OTHER_CHARACTER*)pPacket)->Direction = p_Session->byDirection;
	((st_PACKET_SC_CREATE_OTHER_CHARACTER*)pPacket)->HP = p_Session->chHP;
	((st_PACKET_SC_CREATE_OTHER_CHARACTER*)pPacket)->ID = p_Session->dwSessionID;
	((st_PACKET_SC_CREATE_OTHER_CHARACTER*)pPacket)->X = p_Session->shX;
	((st_PACKET_SC_CREATE_OTHER_CHARACTER*)pPacket)->Y = p_Session->shY;

	return true;
}

bool netPacketProc_SC_DELETE_CHARACTER(st_SESSION* p_Session, char* pHeader, char* pPacket)
{
	((st_PACKET_HEADER*)pHeader)->byCode = dfNETWORK_PACKET_CODE;
	((st_PACKET_HEADER*)pHeader)->bySize = sizeof(st_PACKET_SC_DELETE_CHARACTER);
	((st_PACKET_HEADER*)pHeader)->byType = dfPACKET_SC_DELETE_CHARACTER;

	((st_PACKET_SC_DELETE_CHARACTER*)pPacket)->ID = p_Session->dwSessionID;
	return true;
}

bool netPacketProc_CS_MOVE_START(st_SESSION* p_Session, char* pPacket)
{
	st_PACKET_HEADER Header;
	st_PACKET_SC_MOVE_START SendMsg;
	st_PACKET_CS_MOVE_START* pMoveStart = (st_PACKET_CS_MOVE_START*)pPacket;

	// 메시지 수신 로그 확인
	//---------------------------------------------------------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 끊어버림
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음.
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함.
	// 지금은 간단한 구현을 목적으로 하고 있으므로 오차범위 내에서 클라이언트 좌표를 믿도록 한다. 
	//---------------------------------------------------------------------------------------------------

	if (abs(p_Session->shX - pMoveStart->X) > dfERROR_RANGE ||
		abs(p_Session->shY - pMoveStart->Y) > dfERROR_RANGE)
	{
		PushDisconnectList(p_Session);
#ifdef DEFAULT_LOG
		printf_s("# Error Log: CS_MOVE_START - SessionID: %d : dfERROR_RANGE \n", p_Session->dwSessionID);
		printf_s("## Server X: %d, Y: %d, Client X: %d, Y: %d \n", p_Session->shX, p_Session->shY, pMoveStart->X, pMoveStart->Y);
#endif // DEFAULT_LOG
		return false;
	}

	//---------------------------------------------------------------------------------------------------
	// 동작을 변경. 지금 구현에선 동작번호가 방향값이다.
	//---------------------------------------------------------------------------------------------------
	p_Session->dwAction = pMoveStart->Direction;

	//---------------------------------------------------------------------------------------------------
	// 방향을 변경. 아니면 기존의 방향을 유지하는 결정을 내린다.
	//---------------------------------------------------------------------------------------------------
	switch (pMoveStart->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		p_Session->byDirection = dfPACKET_MOVE_DIR_RR;
		break;

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		p_Session->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	default:
		break;
	}

	// 서버의 위치와 실제 클라의 위치가 틀어진경우, 현재 코드에서는 클라의 위치를 믿고 있다. 
	p_Session->shX = pMoveStart->X;
	p_Session->shY = pMoveStart->Y;

	//---------------------------------------------------------------------------------------------------
	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. 당사자 제외
	//---------------------------------------------------------------------------------------------------
	netPacketProc_SC_MOVE_START((char*)&Header, (char*)&SendMsg, pMoveStart->Direction, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	netSendBroadcast(p_Session, (char*)&Header, (char*)&SendMsg, sizeof(SendMsg));

	return true;
}
bool netPacketProc_SC_MOVE_START(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y)
{
	((st_PACKET_HEADER*)pHeader)->byCode = dfNETWORK_PACKET_CODE;
	((st_PACKET_HEADER*)pHeader)->bySize = sizeof(st_PACKET_SC_MOVE_START);
	((st_PACKET_HEADER*)pHeader)->byType = dfPACKET_SC_MOVE_START;

	((st_PACKET_SC_MOVE_START*)pPacket)->Direction = Direction;
	((st_PACKET_SC_MOVE_START*)pPacket)->ID = ID;
	((st_PACKET_SC_MOVE_START*)pPacket)->X = X;
	((st_PACKET_SC_MOVE_START*)pPacket)->Y = Y;

	return true;
}

bool netPacketProc_CS_MOVE_STOP(st_SESSION* p_Session, char* pPacket)
{
	st_PACKET_HEADER Header;
	st_PACKET_SC_MOVE_STOP SendMsg;
	st_PACKET_CS_MOVE_STOP* pMoveStop = (st_PACKET_CS_MOVE_STOP*)pPacket;

	if (abs(p_Session->shX - pMoveStop->X) > dfERROR_RANGE ||
		abs(p_Session->shY - pMoveStop->Y) > dfERROR_RANGE)
	{
		PushDisconnectList(p_Session);
#ifdef DEFAULT_LOG
		printf_s("# Error Log: CS_MOVE_STOP - SessionID: %d : dfERROR_RANGE \n", p_Session->dwSessionID);
#endif // DEFAULT_LOG
		return false;
	}

	//---------------------------------------------------------------------------------------------------
	// 동작을 변경. 지금 구현에선 동작번호가 방향값이다.
	// 
	// 하지만 정지의 경우 방향값을 그대로 넣어버리면 캐릭터가 정지를 안한다.
	//---------------------------------------------------------------------------------------------------
	p_Session->dwAction = dfPACKET_CS_MOVE_STOP;

	//---------------------------------------------------------------------------------------------------
	// 방향을 변경. 아니면 기존의 방향을 유지하는 결정을 내린다.
	// 
	// 하지만 현재 상황에서는 이동하던 유지하던 방향을 그대로 중지하여 변경할 일이 없다.
	// 
	//---------------------------------------------------------------------------------------------------
	switch (pMoveStop->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		p_Session->byDirection = dfPACKET_MOVE_DIR_RR;
		break;

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		p_Session->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	default:
		break;
	}
	// 서버의 위치와 실제 클라의 위치가 틀어진경우, 현재 코드에서는 클라의 위치를 믿고 있다. 
	p_Session->shX = pMoveStop->X;
	p_Session->shY = pMoveStop->Y;

	//---------------------------------------------------------------------------------------------------
	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. 당사자 제외
	//---------------------------------------------------------------------------------------------------
	netPacketProc_SC_MOVE_STOP((char*)&Header, (char*)&SendMsg, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	netSendBroadcast(p_Session, (char*)&Header, (char*)&SendMsg, sizeof(SendMsg));

	return true;
}
bool netPacketProc_SC_MOVE_STOP(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y)
{
	((st_PACKET_HEADER*)pHeader)->byCode = 0x89;
	((st_PACKET_HEADER*)pHeader)->bySize = sizeof(st_PACKET_SC_MOVE_STOP);
	((st_PACKET_HEADER*)pHeader)->byType = dfPACKET_SC_MOVE_STOP;

	((st_PACKET_SC_MOVE_STOP*)pPacket)->Direction = Direction;
	((st_PACKET_SC_MOVE_STOP*)pPacket)->ID = ID;
	((st_PACKET_SC_MOVE_STOP*)pPacket)->X = X;
	((st_PACKET_SC_MOVE_STOP*)pPacket)->Y = Y;
	return true;
}

bool netPacketProc_CS_ATTACK1(st_SESSION* p_Session, char* pPacket)
{
	// 공격 하는 순간 판정까지 떨어진다.
	st_PACKET_CS_ATTACK1* pTempPacket;
	st_SESSION* p_Temp_Session;
	st_PACKET_HEADER header_SC_ATTACK1;
	st_PACKET_HEADER header_SC_DAMAGE;
	st_PACKET_SC_ATTACK1 packet_SC_ATTACK1;
	st_PACKET_SC_DAMAGE packet_SC_DAMGE;
	std::map<DWORD, st_SESSION*>::iterator iter;
	short temp_X;
	short temp_Y;



	// 공격 모션에 대한 패킷은 모두에게 보낸다.
	pTempPacket = (st_PACKET_CS_ATTACK1*)pPacket;

	netPacketProc_SC_ATTACK1((char*)&header_SC_ATTACK1, (char*)&packet_SC_ATTACK1, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	netSendBroadcast(p_Session, (char*)&header_SC_ATTACK1, (char*)&packet_SC_ATTACK1, sizeof(packet_SC_ATTACK1));

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// 오른쪽 타격 (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK1_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack1 Direction:RR /  SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				netPacketProc_SC_DAMAGE((char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				netSendBroadcast(NULL, (char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, sizeof(packet_SC_DAMGE));
				break;
			}
		}

		break;
	}

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
	{
		// 왼쪽 타격  (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK1_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack1 Direction:LL / SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				netPacketProc_SC_DAMAGE((char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				netSendBroadcast(NULL, (char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, sizeof(packet_SC_DAMGE));
				break;
			}
		}
		break;
	}
	default:
		break;
	}

	return true;
}
bool netPacketProc_SC_ATTACK1(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y)
{
	((st_PACKET_HEADER*)pHeader)->byCode = dfNETWORK_PACKET_CODE;
	((st_PACKET_HEADER*)pHeader)->bySize = sizeof(st_PACKET_SC_ATTACK1);
	((st_PACKET_HEADER*)pHeader)->byType = dfPACKET_SC_ATTACK1;

	((st_PACKET_SC_ATTACK1*)pPacket)->Direction = Direction;
	((st_PACKET_SC_ATTACK1*)pPacket)->ID = ID;
	((st_PACKET_SC_ATTACK1*)pPacket)->X = X;
	((st_PACKET_SC_ATTACK1*)pPacket)->Y = Y;
	return true;
}

bool netPacketProc_CS_ATTACK2(st_SESSION* p_Session, char* pPacket)
{
	// 공격 하는 순간 판정까지 떨어진다.
	st_PACKET_CS_ATTACK2* pTempPacket;
	st_SESSION* p_Temp_Session;
	st_PACKET_HEADER header_SC_ATTACK2;
	st_PACKET_HEADER header_SC_DAMAGE;
	st_PACKET_SC_ATTACK2 packet_SC_ATTACK2;
	st_PACKET_SC_DAMAGE packet_SC_DAMGE;
	std::map<DWORD, st_SESSION*>::iterator iter;
	short temp_X;
	short temp_Y;



	// 공격 모션에 대한 패킷은 모두에게 보낸다.
	pTempPacket = (st_PACKET_CS_ATTACK2*)pPacket;

	netPacketProc_SC_ATTACK2((char*)&header_SC_ATTACK2, (char*)&packet_SC_ATTACK2, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	netSendBroadcast(p_Session, (char*)&header_SC_ATTACK2, (char*)&packet_SC_ATTACK2, sizeof(packet_SC_ATTACK2));

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// 오른쪽 타격 (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK2_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack2 Direction:RR /  SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				netPacketProc_SC_DAMAGE((char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				netSendBroadcast(NULL, (char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, sizeof(packet_SC_DAMGE));
				break;
			}
		}

		break;
	}

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
	{
		// 왼쪽 타격  (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK2_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack2 Direction:LL / SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				netPacketProc_SC_DAMAGE((char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				netSendBroadcast(NULL, (char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, sizeof(packet_SC_DAMGE));
				break;
			}
		}
		break;
	}
	default:
		break;
	}

	return true;
}
bool netPacketProc_SC_ATTACK2(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y)
{
	((st_PACKET_HEADER*)pHeader)->byCode = dfNETWORK_PACKET_CODE;
	((st_PACKET_HEADER*)pHeader)->bySize = sizeof(st_PACKET_SC_ATTACK2);
	((st_PACKET_HEADER*)pHeader)->byType = dfPACKET_SC_ATTACK2;

	((st_PACKET_SC_ATTACK2*)pPacket)->Direction = Direction;
	((st_PACKET_SC_ATTACK2*)pPacket)->ID = ID;
	((st_PACKET_SC_ATTACK2*)pPacket)->X = X;
	((st_PACKET_SC_ATTACK2*)pPacket)->Y = Y;
	return true;
}

bool netPacketProc_CS_ATTACK3(st_SESSION* p_Session, char* pPacket)
{
	// 공격 하는 순간 판정까지 떨어진다.
	st_PACKET_CS_ATTACK3* pTempPacket;
	st_SESSION* p_Temp_Session;
	st_PACKET_HEADER header_SC_ATTACK3;
	st_PACKET_HEADER header_SC_DAMAGE;
	st_PACKET_SC_ATTACK3 packet_SC_ATTACK3;
	st_PACKET_SC_DAMAGE packet_SC_DAMGE;
	std::map<DWORD, st_SESSION*>::iterator iter;
	short temp_X;
	short temp_Y;



	// 공격 모션에 대한 패킷은 모두에게 보낸다.
	pTempPacket = (st_PACKET_CS_ATTACK3*)pPacket;

	netPacketProc_SC_ATTACK3((char*)&header_SC_ATTACK3, (char*)&packet_SC_ATTACK3, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	netSendBroadcast(p_Session, (char*)&header_SC_ATTACK3, (char*)&packet_SC_ATTACK3, sizeof(packet_SC_ATTACK3));

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// 오른쪽 타격 (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK3_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack3 Direction:RR /  SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				netPacketProc_SC_DAMAGE((char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				netSendBroadcast(NULL, (char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, sizeof(packet_SC_DAMGE));
				break;
			}
		}

		break;
	}

	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
	{
		// 왼쪽 타격  (x 좌표가 나를 기준으로 - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// 기저 사례1: 나를 제외하고 검색한다.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// 나를 기준으로 오른쪽 X좌표가 범위보다 작고 그리고 Y좌표 차의 절댓값이 위아래 범위를 넘지 않을 경우
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP를 깐다.
				// 3. st_PACKET_SC_DAMAGE 메시지를 보낸다.
				p_Temp_Session->chHP -= dfATTACK3_DAMAGE;

#ifdef DEFAULT_LOG
				printf_s("#Attack3 Direction:LL / SessionID:%d -> SessionID:%d \n", p_Session->dwSessionID, p_Temp_Session->dwSessionID);
#endif
				netPacketProc_SC_DAMAGE((char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, p_Session->dwSessionID, p_Temp_Session->dwSessionID, p_Temp_Session->chHP);
				netSendBroadcast(NULL, (char*)&header_SC_DAMAGE, (char*)&packet_SC_DAMGE, sizeof(packet_SC_DAMGE));
				break;
			}
		}
		break;
	}
	default:
		break;
	}

	return true;
}
bool netPacketProc_SC_ATTACK3(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y)
{
	((st_PACKET_HEADER*)pHeader)->byCode = dfNETWORK_PACKET_CODE;
	((st_PACKET_HEADER*)pHeader)->bySize = sizeof(st_PACKET_SC_ATTACK3);
	((st_PACKET_HEADER*)pHeader)->byType = dfPACKET_SC_ATTACK3;

	((st_PACKET_SC_ATTACK3*)pPacket)->Direction = Direction;
	((st_PACKET_SC_ATTACK3*)pPacket)->ID = ID;
	((st_PACKET_SC_ATTACK3*)pPacket)->X = X;
	((st_PACKET_SC_ATTACK3*)pPacket)->Y = Y;
	return true;
}

bool netPacketProc_SC_DAMAGE(char* pHeader, char* pPacket, __int32 AttackID, __int32 DamageID, char HP)
{
	((st_PACKET_HEADER*)pHeader)->byCode = dfNETWORK_PACKET_CODE;
	((st_PACKET_HEADER*)pHeader)->bySize = sizeof(st_PACKET_SC_DAMAGE);
	((st_PACKET_HEADER*)pHeader)->byType = dfPACKET_SC_DAMAGE;

	((st_PACKET_SC_DAMAGE*)pPacket)->AttackID = AttackID;
	((st_PACKET_SC_DAMAGE*)pPacket)->DamageID = DamageID;
	((st_PACKET_SC_DAMAGE*)pPacket)->DamageHP = HP;
	return true;
}


#endif // SERIALIZEBUFFER













