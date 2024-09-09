#include <Windows.h>

#include "KeyBoard.h"
#include "main.h"
#include "Player.h"




//---------------------------------
// 키보드 입력
//---------------------------------
void kb_KeyProcess(void)
{
	if (GetAsyncKeyState('N') & 0x8000)
	{
		g_AddTime++;
	}

	if (GetAsyncKeyState('M') & 0x8000)
	{
		if (g_AddTime > 0)
			g_AddTime--;
	}

	switch (g_Scene)
	{
	case TITLE:
	{
		kb_Title();
		break;
	}
	case GAME:
	{
		if (!play_KeyProcess())
			g_Scene = OVER;
		break;
	}
	case CLEAR:
	{
		break;
	}
	case OVER:
	{
		break;
	}
	case GAME_ERROR:
	{
		break;
	}
	}
}


void kb_Title(void)
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8001)
	{
		g_Scene = LOAD;
	}
}
