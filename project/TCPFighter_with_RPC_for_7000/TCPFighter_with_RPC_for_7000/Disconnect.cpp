
#include <Windows.h>
#include <list>
#include <unordered_map>
#include <string>
#include "SerializeBuffer.h"

#include "Disconnect.h"
#include "C_Ring_Buffer.h"
#include "Session.h"
#include "Player.h"
#include "Define.h"
#include "CList.h"
#include "Field.h"
#include "Protocol.h"
#include "main.h"


// std::list<DWORD> g_Disconnect_List;
CList<DWORD> g_Disconnect_List;
OreoPizza::CMemoryPool<CList<DWORD>::Node> g_Disconnect_List_MemPool(8001, false);
bool g_bCheckDisconnectSessionIds[8001];

bool DisconnectCheck(DWORD dwSessionID)
{
	return g_bCheckDisconnectSessionIds[dwSessionID];
}

void InitDisconnectSessionIds(void)
{
	ZeroMemory(g_bCheckDisconnectSessionIds, 8001);
	g_Disconnect_List.InitCList(g_Disconnect_List_MemPool);
}

void enqueueForDeletion(DWORD dwSessionID)
{
	if (g_bCheckDisconnectSessionIds[dwSessionID] == false)
	{
		g_Disconnect_List.push_back(dwSessionID);
		g_bCheckDisconnectSessionIds[dwSessionID] = true;
	}
}

void Disconnect(void)
{
	//std::list<DWORD>::iterator iter;
	CList<DWORD>::iterator iter;
	st_SESSION* pSession;
	DWORD dwSessionID;
	st_PLAYER* st_p_Player;
	st_SECTOR_AROUND st_Sector_Around;

	// 3�� ���
	// �׳� �ִ°͸� �����ϰ�, ���� �ִ� ���� ������ �����Ѵ�.
	for (iter = g_Disconnect_List.begin(); iter != g_Disconnect_List.end(); )
	{
		// g_CharacterHash ���� ����
		// g_Session_Hash ���� ����
		// g_Sector ���� ����
		// �ֺ� 9ĭ�� ���� �޽��� �Ѹ���
		// 
		
		dwSessionID = *iter;
		pSession = FindSession(dwSessionID);
		st_p_Player = FindCharacter(dwSessionID);

		mpDelete(&g_Packet, dwSessionID);
		C_Field::GetInstance()->GetSectorAround(st_p_Player->_CurSector->iX, st_p_Player->_CurSector->iY, &st_Sector_Around);
		C_Field::GetInstance()->SendPacket_Around(pSession, &g_Packet, &st_Sector_Around);
		g_Packet.Clear();

		// st_p_Player�� ������ �����̱� ������ delete�� �ϱ� ���� ����ؾ� �Ѵ�.
		C_Field::GetInstance()->removeUserFromSector(st_p_Player);
		// ���� �Ҵ�� ������ �Ժη� '=' ������ �ϸ� ������ �����.
		DeleteCharacter(dwSessionID);
		DeleteSession(dwSessionID);

		iter = g_Disconnect_List.erase(iter);
		g_bCheckDisconnectSessionIds[dwSessionID] = false;
	}

}

void mpDelete(SerializeBuffer* pPacket, DWORD dwSessionID)
{
	st_PACKET_HEADER New_Header;

	New_Header.byCode = (char)dfPACKET_CODE;
	New_Header.bySize = 4;
	New_Header.byType = (char)dfPACKET_SC_DELETE_CHATACTER;

	(*pPacket).PutData((char*)&New_Header, sizeof(New_Header));
	(*pPacket) << dwSessionID;
}


