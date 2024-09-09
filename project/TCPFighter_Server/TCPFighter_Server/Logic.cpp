
#include <stdio.h>
#include <map>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")

#include "C_Ring_Buffer.h"
#include "CList.h"
#include "main.h"
#include "Logic.h"
#include "SerializeBuffer.h"
#include "NetWork.h"
#include "PacketDefine.h"
#include "Set_Log.h"


void Update()
{
	/////////////////////////////////////////////////////////////////////////
	// 50�������� �����. 
	// �׷��� ���ؼ� 
	// 1�� == 1000ms
	// 
	// ��� 20ms���� 1���� ���ƾ� �Ѵ�.
	/////////////////////////////////////////////////////////////////////////
	g_End_Time = timeGetTime();
	if (g_End_Time - g_Start_Time < 20)
	{
		// 20ms ���� ���� �ð��� ������ ������ ���Դٸ�
		// ������ ������ �ʰ� ��ȯ�Ѵ�. 
		return;
	}
	g_Start_Time += 20;


	std::map<DWORD, st_SESSION*>::iterator iter;
	st_SESSION* pSession;
	bool isNoX;
	bool isNoY;

	for (iter = g_Session_List.begin(); iter != g_Session_List.end(); ++iter)
	{
		pSession = (*iter).second;
		isNoX = false;
		isNoY = false;

		if (pSession->Disconnect) continue;

		if (0 >= pSession->chHP)
		{
#ifdef DETAILS_LOG
			printf_s("HP 0 # SessionID: %d \n", pSession->dwSessionID);
#endif // DETAILS_LOG
			PushDisconnectList(pSession);
			continue;
		}

		// ���ó���� �ؾ���
		// ����� ��� �밢�� �̵��� ���ϰ� �ؾ��Ѵ�. 
		//if (pSession->shX = dfRANGE_MOVE_LEFT)
		if (pSession->shX - FRAME_X_MOVEMENT < dfRANGE_MOVE_LEFT || pSession->shX + FRAME_X_MOVEMENT > dfRANGE_MOVE_RIGHT)
		{
			isNoX = true;
		}
		if (pSession->shY - FRAME_Y_MOVEMENT < dfRANGE_MOVE_TOP || pSession->shY + FRAME_Y_MOVEMENT > dfRANGE_MOVE_BOTTOM)
		{
			isNoY = true;
		}

		// ���� ���ۿ� ���� ó��
		switch (pSession->dwAction)
		{
		case dfPACKET_MOVE_DIR_LL:
			if (!isNoX)
			{
#ifdef DEFAULT_LOG
				printf_s("# gameRun: LL # SessionID: %d / X:%d / Y:%d \n", pSession->dwSessionID, pSession->shX, pSession->shY);
#endif // DEFAULT_LOG
				pSession->shX -= FRAME_X_MOVEMENT;
			}
			break;
		case dfPACKET_MOVE_DIR_LU:
			if (!isNoX && !isNoY)
			{
#ifdef DEFAULT_LOG
				printf_s("# gameRun: LU # SessionID: %d / X:%d / Y:%d \n", pSession->dwSessionID, pSession->shX, pSession->shY);
#endif // DEFAULT_LOG
				pSession->shX -= FRAME_X_MOVEMENT;
				pSession->shY -= FRAME_Y_MOVEMENT;
			}
			break;
		case dfPACKET_MOVE_DIR_UU:
			if (!isNoY)
			{
#ifdef DEFAULT_LOG
				printf_s("# gameRun: UU # SessionID: %d / X:%d / Y:%d \n", pSession->dwSessionID, pSession->shX, pSession->shY);
#endif // DEFAULT_LOG
				pSession->shY -= FRAME_Y_MOVEMENT;
			}
			break;
		case dfPACKET_MOVE_DIR_RU:
			if (!isNoX && !isNoY)
			{
#ifdef DEFAULT_LOG
				printf_s("# gameRun: RU # SessionID: %d / X:%d / Y:%d \n", pSession->dwSessionID, pSession->shX, pSession->shY);
#endif // DEFAULT_LOG
				pSession->shX += FRAME_X_MOVEMENT;
				pSession->shY -= FRAME_Y_MOVEMENT;
			}
			break;
		case dfPACKET_MOVE_DIR_RR:
			if (!isNoX)
			{
#ifdef DEFAULT_LOG
				printf_s("# gameRun: RR # SessionID: %d / X:%d / Y:%d \n", pSession->dwSessionID, pSession->shX, pSession->shY);
#endif // DEFAULT_LOG
				pSession->shX += FRAME_X_MOVEMENT;
			}
			break;
		case dfPACKET_MOVE_DIR_RD:
			if (!isNoX && !isNoY)
			{
#ifdef DEFAULT_LOG
				printf_s("# gameRun: RD # SessionID: %d / X:%d / Y:%d \n", pSession->dwSessionID, pSession->shX, pSession->shY);
#endif // DEFAULT_LOG
				pSession->shX += FRAME_X_MOVEMENT;
				pSession->shY += FRAME_Y_MOVEMENT;
			}
			break;
		case dfPACKET_MOVE_DIR_DD:
			if (!isNoY)
			{
#ifdef DEFAULT_LOG
				printf_s("# gameRun: DD # SessionID: %d / X:%d / Y:%d \n", pSession->dwSessionID, pSession->shX, pSession->shY);
#endif // DEFAULT_LOG
				pSession->shY += FRAME_Y_MOVEMENT;
			}
			break;
		case dfPACKET_MOVE_DIR_LD:
			if (!isNoX && !isNoY)
			{
#ifdef DEFAULT_LOG
				printf_s("# gameRun: LD # SessionID: %d / X:%d / Y:%d \n", pSession->dwSessionID, pSession->shX, pSession->shY);
#endif // DEFAULT_LOG
				pSession->shX -= FRAME_X_MOVEMENT;
				pSession->shY += FRAME_Y_MOVEMENT;
			}
			break;
		case dfPACKET_CS_MOVE_STOP:
		{
			break;
		}
		default:
		{
			// �̻��� ������ �´ٸ� �׳� ������ ���������. 
			// ��� ���� ���������� �׳� �ƹ��͵� ���ϸ� �ȴ�. 
#ifdef DEFAULT_LOG
			printf_s("dwSessionID Error # SessionID: %d \n", pSession->dwSessionID);
#endif // DEFAULT_LOG
			PushDisconnectList(pSession);
			__debugbreak();
			break;
		}
		}

		//-----------------------------------------------------------------
		// ȭ�� �̵����� 
		// ���� �˻�
		//-----------------------------------------------------------------
		if (pSession->shX < dfRANGE_MOVE_LEFT) pSession->shX = dfRANGE_MOVE_LEFT;
		else if (pSession->shX > dfRANGE_MOVE_RIGHT) pSession->shX = dfRANGE_MOVE_RIGHT;

		if (pSession->shY < dfRANGE_MOVE_TOP) pSession->shY = dfRANGE_MOVE_TOP;
		else if (pSession->shY > dfRANGE_MOVE_BOTTOM) pSession->shY = dfRANGE_MOVE_BOTTOM;



	}

}
