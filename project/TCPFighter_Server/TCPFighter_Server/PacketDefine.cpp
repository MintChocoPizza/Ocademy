
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
// ����� ��Ŷ�� ��� char* �̴�. 
// 
// ū �ǹ� ����. �׳� ���� ���ڰ� ������̴�.
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

	// �޽��� ���� �α� Ȯ��
	//---------------------------------------------------------------------------------------------------
	// ������ ��ġ�� ���� ��Ŷ�� ��ġ���� �ʹ� ū ���̰� ���ٸ� �������
	// �� ������ ��ǥ ����ȭ ������ �ܼ��� Ű���� ���� (Ŭ���̾�Ʈ�� ��ó��, ������ �� �ݿ�) �������
	// Ŭ���̾�Ʈ�� ��ǥ�� �״�� �ϴ� ����� ���ϰ� ����.
	// ���� �¶��� �����̶�� Ŭ���̾�Ʈ���� �������� �����ϴ� ����� ���ؾ� ��.
	// ������ ������ ������ �������� �ϰ� �����Ƿ� �������� ������ Ŭ���̾�Ʈ ��ǥ�� �ϵ��� �Ѵ�. 
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
	// ������ ����. ���� �������� ���۹�ȣ�� ���Ⱚ�̴�.
	//---------------------------------------------------------------------------------------------------
	p_Session->dwAction = pMoveStart->Direction;

	//---------------------------------------------------------------------------------------------------
	// ������ ����. �ƴϸ� ������ ������ �����ϴ� ������ ������.
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

	// ������ ��ġ�� ���� Ŭ���� ��ġ�� Ʋ�������, ���� �ڵ忡���� Ŭ���� ��ġ�� �ϰ� �ִ�. 
	p_Session->shX = pMoveStart->X;
	p_Session->shY = pMoveStart->Y;

	//---------------------------------------------------------------------------------------------------
	// ���� �������� ����ڿ��� ��� ��Ŷ�� �Ѹ���. ����� ����
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
	// ������ ����. ���� �������� ���۹�ȣ�� ���Ⱚ�̴�.
	// 
	// ������ ������ ��� ���Ⱚ�� �״�� �־������ ĳ���Ͱ� ������ ���Ѵ�.
	//---------------------------------------------------------------------------------------------------
	p_Session->dwAction = dfPACKET_CS_MOVE_STOP;

	//---------------------------------------------------------------------------------------------------
	// ������ ����. �ƴϸ� ������ ������ �����ϴ� ������ ������.
	// 
	// ������ ���� ��Ȳ������ �̵��ϴ� �����ϴ� ������ �״�� �����Ͽ� ������ ���� ����.
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
	// ������ ��ġ�� ���� Ŭ���� ��ġ�� Ʋ�������, ���� �ڵ忡���� Ŭ���� ��ġ�� �ϰ� �ִ�. 
	p_Session->shX = pMoveStop->X;
	p_Session->shY = pMoveStop->Y;

	//---------------------------------------------------------------------------------------------------
	// ���� �������� ����ڿ��� ��� ��Ŷ�� �Ѹ���. ����� ����
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
	// ���� �ϴ� ���� �������� ��������.
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



	// ���� ��ǿ� ���� ��Ŷ�� ��ο��� ������.
	pTempPacket = (st_PACKET_CS_ATTACK1*)clpPacket->GetBufferPtr();

	SerializeBuffer_netPacketProc_SC_ATTACK1(&clPacket, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	SerializeBuffer_netSendBroadcast(p_Session, &clPacket);

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// ������ Ÿ�� (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
		// ���� Ÿ��  (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
	// ���� �ϴ� ���� �������� ��������.
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



	// ���� ��ǿ� ���� ��Ŷ�� ��ο��� ������.
	pTempPacket = (st_PACKET_CS_ATTACK2*)clpPacket->GetBufferPtr();

	SerializeBuffer_netPacketProc_SC_ATTACK2(&clPacket, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	SerializeBuffer_netSendBroadcast(p_Session, &clPacket);


	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// ������ Ÿ�� (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
		// ���� Ÿ��  (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
	// ���� �ϴ� ���� �������� ��������.
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



	// ���� ��ǿ� ���� ��Ŷ�� ��ο��� ������.
	pTempPacket = (st_PACKET_CS_ATTACK3*)clpPacket->GetBufferPtr();

	SerializeBuffer_netPacketProc_SC_ATTACK3(&clPacket, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	SerializeBuffer_netSendBroadcast(p_Session, &clPacket);

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// ������ Ÿ�� (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
		// ���� Ÿ��  (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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

	// �޽��� ���� �α� Ȯ��
	//---------------------------------------------------------------------------------------------------
	// ������ ��ġ�� ���� ��Ŷ�� ��ġ���� �ʹ� ū ���̰� ���ٸ� �������
	// �� ������ ��ǥ ����ȭ ������ �ܼ��� Ű���� ���� (Ŭ���̾�Ʈ�� ��ó��, ������ �� �ݿ�) �������
	// Ŭ���̾�Ʈ�� ��ǥ�� �״�� �ϴ� ����� ���ϰ� ����.
	// ���� �¶��� �����̶�� Ŭ���̾�Ʈ���� �������� �����ϴ� ����� ���ؾ� ��.
	// ������ ������ ������ �������� �ϰ� �����Ƿ� �������� ������ Ŭ���̾�Ʈ ��ǥ�� �ϵ��� �Ѵ�. 
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
	// ������ ����. ���� �������� ���۹�ȣ�� ���Ⱚ�̴�.
	//---------------------------------------------------------------------------------------------------
	p_Session->dwAction = pMoveStart->Direction;

	//---------------------------------------------------------------------------------------------------
	// ������ ����. �ƴϸ� ������ ������ �����ϴ� ������ ������.
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

	// ������ ��ġ�� ���� Ŭ���� ��ġ�� Ʋ�������, ���� �ڵ忡���� Ŭ���� ��ġ�� �ϰ� �ִ�. 
	p_Session->shX = pMoveStart->X;
	p_Session->shY = pMoveStart->Y;

	//---------------------------------------------------------------------------------------------------
	// ���� �������� ����ڿ��� ��� ��Ŷ�� �Ѹ���. ����� ����
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
	// ������ ����. ���� �������� ���۹�ȣ�� ���Ⱚ�̴�.
	// 
	// ������ ������ ��� ���Ⱚ�� �״�� �־������ ĳ���Ͱ� ������ ���Ѵ�.
	//---------------------------------------------------------------------------------------------------
	p_Session->dwAction = dfPACKET_CS_MOVE_STOP;

	//---------------------------------------------------------------------------------------------------
	// ������ ����. �ƴϸ� ������ ������ �����ϴ� ������ ������.
	// 
	// ������ ���� ��Ȳ������ �̵��ϴ� �����ϴ� ������ �״�� �����Ͽ� ������ ���� ����.
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
	// ������ ��ġ�� ���� Ŭ���� ��ġ�� Ʋ�������, ���� �ڵ忡���� Ŭ���� ��ġ�� �ϰ� �ִ�. 
	p_Session->shX = pMoveStop->X;
	p_Session->shY = pMoveStop->Y;

	//---------------------------------------------------------------------------------------------------
	// ���� �������� ����ڿ��� ��� ��Ŷ�� �Ѹ���. ����� ����
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
	// ���� �ϴ� ���� �������� ��������.
	st_PACKET_CS_ATTACK1* pTempPacket;
	st_SESSION* p_Temp_Session;
	st_PACKET_HEADER header_SC_ATTACK1;
	st_PACKET_HEADER header_SC_DAMAGE;
	st_PACKET_SC_ATTACK1 packet_SC_ATTACK1;
	st_PACKET_SC_DAMAGE packet_SC_DAMGE;
	std::map<DWORD, st_SESSION*>::iterator iter;
	short temp_X;
	short temp_Y;



	// ���� ��ǿ� ���� ��Ŷ�� ��ο��� ������.
	pTempPacket = (st_PACKET_CS_ATTACK1*)pPacket;

	netPacketProc_SC_ATTACK1((char*)&header_SC_ATTACK1, (char*)&packet_SC_ATTACK1, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	netSendBroadcast(p_Session, (char*)&header_SC_ATTACK1, (char*)&packet_SC_ATTACK1, sizeof(packet_SC_ATTACK1));

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// ������ Ÿ�� (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
		// ���� Ÿ��  (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
	// ���� �ϴ� ���� �������� ��������.
	st_PACKET_CS_ATTACK2* pTempPacket;
	st_SESSION* p_Temp_Session;
	st_PACKET_HEADER header_SC_ATTACK2;
	st_PACKET_HEADER header_SC_DAMAGE;
	st_PACKET_SC_ATTACK2 packet_SC_ATTACK2;
	st_PACKET_SC_DAMAGE packet_SC_DAMGE;
	std::map<DWORD, st_SESSION*>::iterator iter;
	short temp_X;
	short temp_Y;



	// ���� ��ǿ� ���� ��Ŷ�� ��ο��� ������.
	pTempPacket = (st_PACKET_CS_ATTACK2*)pPacket;

	netPacketProc_SC_ATTACK2((char*)&header_SC_ATTACK2, (char*)&packet_SC_ATTACK2, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	netSendBroadcast(p_Session, (char*)&header_SC_ATTACK2, (char*)&packet_SC_ATTACK2, sizeof(packet_SC_ATTACK2));

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// ������ Ÿ�� (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
		// ���� Ÿ��  (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
	// ���� �ϴ� ���� �������� ��������.
	st_PACKET_CS_ATTACK3* pTempPacket;
	st_SESSION* p_Temp_Session;
	st_PACKET_HEADER header_SC_ATTACK3;
	st_PACKET_HEADER header_SC_DAMAGE;
	st_PACKET_SC_ATTACK3 packet_SC_ATTACK3;
	st_PACKET_SC_DAMAGE packet_SC_DAMGE;
	std::map<DWORD, st_SESSION*>::iterator iter;
	short temp_X;
	short temp_Y;



	// ���� ��ǿ� ���� ��Ŷ�� ��ο��� ������.
	pTempPacket = (st_PACKET_CS_ATTACK3*)pPacket;

	netPacketProc_SC_ATTACK3((char*)&header_SC_ATTACK3, (char*)&packet_SC_ATTACK3, p_Session->byDirection, p_Session->dwSessionID, p_Session->shX, p_Session->shY);
	netSendBroadcast(p_Session, (char*)&header_SC_ATTACK3, (char*)&packet_SC_ATTACK3, sizeof(packet_SC_ATTACK3));

	switch (pTempPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
	{
		// ������ Ÿ�� (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((temp_X - p_Session->shX > 0) && (temp_X - p_Session->shX < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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
		// ���� Ÿ��  (x ��ǥ�� ���� �������� - )
		for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
		{

			// ���� ���1: ���� �����ϰ� �˻��Ѵ�.
			p_Temp_Session = (*iter).second;
			if (p_Session == p_Temp_Session) continue;
			temp_X = p_Temp_Session->shX;
			temp_Y = p_Temp_Session->shY;


			// ���� �������� ������ X��ǥ�� �������� �۰� �׸��� Y��ǥ ���� ������ ���Ʒ� ������ ���� ���� ���
			if ((p_Session->shX - temp_X > 0) && (p_Session->shX - temp_X < dfATTACK1_RANGE_X) && (abs(temp_Y - p_Session->shY) < dfATTACK1_RANGE_Y / 2))
			{
				// 1. HP�� ���.
				// 3. st_PACKET_SC_DAMAGE �޽����� ������.
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













