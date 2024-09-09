#include <stdio.h>
#include <Windows.h>

#include "Buffer.h"

//----------------------------------
// 더블 버퍼링을 위한 화면 버퍼
// 
//----------------------------------
char g_szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

//--------------------------------------------------------------------
// 버퍼의 내용을 화면으로 찍어주는 함수.
//
// 1 프레임이 끝나는 마지막에 본 함수를 호출하여 버퍼 -> 화면 으로 그린다.
//--------------------------------------------------------------------
void buff_Buffer_Flip(void)
{
	int iCnt;
	//for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; ++iCnt)
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; ++iCnt)
	{
		cs_MoveCursor(iCnt, 0);
		printf_s(g_szScreenBuffer[iCnt]);
	}
}

//--------------------------------------------------------------------
// 화면 버퍼를 지워주는 함수
//
// 매 프레임 그림을 그리기 직전에 버퍼를 지워 준다. 
// 안그러면 이전 프레임의 잔상이 남으니까
//--------------------------------------------------------------------
void buff_Buffer_Clear(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
	{
		memset(g_szScreenBuffer[iCnt], ' ', dfSCREEN_WIDTH);
		g_szScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = (char)NULL;
	}
}

//--------------------------------------------------------------------
// 버퍼의 특정 위치에 원하는 문자를 출력.
//
// 입력 받은 X,Y 좌표에 아스키코드 하나를 출력한다. (버퍼에 그림)
//--------------------------------------------------------------------
void buff_Sprite_Draw(int iY, int iX, char chSprite)
{
	if (iX < 0 || iY < 0 || iX >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
		return;

	g_szScreenBuffer[iY][iX] = chSprite;
}

//--------------------------------------------------------------------
// 버퍼의 특정 위치에 원하는 문자열을 출력.
//
// 입력 받은 X,Y 좌표 부터 시작하여 문자열을 출력한다. (버퍼에 그림)
//--------------------------------------------------------------------
void buff_Sprite_Draw_String(int iY, int iX, const char* cpSprite, int Str_Len)
{
	int i_Cnt;

	if (iX < 0 || iY < 0 || iX >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
		return;

	for (i_Cnt = 0; i_Cnt < Str_Len; ++i_Cnt)
	{
		if (iX + i_Cnt < 0 || iY < 0 || iX + i_Cnt >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
			continue;

		g_szScreenBuffer[iY][iX + i_Cnt] = cpSprite[i_Cnt];
	}
	
}
