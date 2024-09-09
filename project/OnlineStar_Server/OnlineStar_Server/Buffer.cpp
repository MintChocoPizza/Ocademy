#include <stdio.h>
#include <Windows.h>

#include "Buffer.h"

//----------------------------------
// ���� ���۸��� ���� ȭ�� ����
// 
//----------------------------------
char g_szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

//--------------------------------------------------------------------
// ������ ������ ȭ������ ����ִ� �Լ�.
//
// 1 �������� ������ �������� �� �Լ��� ȣ���Ͽ� ���� -> ȭ�� ���� �׸���.
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
// ȭ�� ���۸� �����ִ� �Լ�
//
// �� ������ �׸��� �׸��� ������ ���۸� ���� �ش�. 
// �ȱ׷��� ���� �������� �ܻ��� �����ϱ�
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
// ������ Ư�� ��ġ�� ���ϴ� ���ڸ� ���.
//
// �Է� ���� X,Y ��ǥ�� �ƽ�Ű�ڵ� �ϳ��� ����Ѵ�. (���ۿ� �׸�)
//--------------------------------------------------------------------
void buff_Sprite_Draw(int iY, int iX, char chSprite)
{
	if (iX < 0 || iY < 0 || iX >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
		return;

	g_szScreenBuffer[iY][iX] = chSprite;
}

//--------------------------------------------------------------------
// ������ Ư�� ��ġ�� ���ϴ� ���ڿ��� ���.
//
// �Է� ���� X,Y ��ǥ ���� �����Ͽ� ���ڿ��� ����Ѵ�. (���ۿ� �׸�)
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
