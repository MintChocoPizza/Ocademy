
#include <stdio.h>
#include <Windows.h>
#include "MyNew.h"

#include "FPSManager.h"
#include "ConsoleBuffer.h"

ConsoleBuffer ConsoleBuffer::_ConsoleBuffer;

ConsoleBuffer::ConsoleBuffer() : hConsole(NULL)
{
	initial();
}

ConsoleBuffer::~ConsoleBuffer()
{
}

ConsoleBuffer* ConsoleBuffer::GetInstance(void)
{
	return &_ConsoleBuffer;
}

void ConsoleBuffer::Buffer_Flip(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; ++iCnt)
	{
		MoveCursor(iCnt, 0);
		printf_s(ScreenBuffer[iCnt]);
	}
}

void ConsoleBuffer::Buffer_Clear(void)
{
	memset(ScreenBuffer, ' ', sizeof(ScreenBuffer));
	Sprite_Null();
}

void ConsoleBuffer::Sprite_Null(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; ++iCnt)
	{
		ScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = (char)NULL;
	}
}

void ConsoleBuffer::Sprite_Background(unsigned char* pFileMemory)
{
	memcpy_s(ScreenBuffer, sizeof(ScreenBuffer), pFileMemory, sizeof(ScreenBuffer));
	Sprite_Null();
}

void ConsoleBuffer::Sprite_Center_String(int iY, const char* pSpriteString)
{
	int iCnt;
	int szStringLength;



	szStringLength = (int)strlen(pSpriteString);
	for (iCnt = 0; iCnt < szStringLength; ++iCnt)
	{
		Sprite_Draw(iY, (dfSCREEN_WIDTH / 2) - (szStringLength - 1) / 2 + iCnt, pSpriteString[iCnt]);
	}
}

void ConsoleBuffer::Sprite_Draw(int iY, int iX, char chSprite)
{
	if (iX < 0 || iY < 0 || iX >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
		return;

	ScreenBuffer[iY][iX] = chSprite;
}

void ConsoleBuffer::print_FPS(void)
{
	MoveCursor(dfSCREEN_HEIGHT, 0);
	printf_s("                                       ");
	MoveCursor(dfSCREEN_HEIGHT, 0);
	CFpsManager::GetInstance()->print_FPS();
}
















void ConsoleBuffer::initial(void)
{
	CONSOLE_CURSOR_INFO stConsoleCursor;
	char cCommand[28];

	//-----------------------------------------------------
	// ȭ���� Ŀ���� �Ⱥ��̰Բ� �����Ѵ�.
	//-----------------------------------------------------
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1;				// Ŀ�� ũ��
	// �̻��ϰԵ� 0�̸� ���´�. 1�� �ϸ� �ȳ��´�.

	//-----------------------------------------------------
	// �ܼ�ȭ�� (���ٴٵ� �ƿ�ǲ) �ڵ��� ���Ѵ�. 
	// 
	//-----------------------------------------------------
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);


	//-----------------------------------------------------
	// �ܼ�ȭ�� ũ�⸦ �����Ѵ�.
	//col = ����, lines = ����
	// 
	//-----------------------------------------------------

	sprintf_s(cCommand, "mode con:cols=%d lines=%d", dfSCREEN_WIDTH, dfSCREEN_HEIGHT + 2);
	system(cCommand);
}

void ConsoleBuffer::MoveCursor(int iPosY, int iPosX)
{
	COORD stCoord;
	stCoord.Y = iPosY;
	stCoord.X = iPosX;

	if (SetConsoleCursorPosition(hConsole, stCoord) == 0)
	{
		printf_s("Ŀ�� �̵� �����߽��ϴ�: %d \n", GetLastError());
		return;
	}
}

void ConsoleBuffer::ClearScreen(void)
{
	int iCountX, iCountY;

	//-------------------------------------------------------------
	// ȭ�� ũ�⸸ŭ ����, ���� ���� for ���� ����Ͽ�
	// ������ ��ǥ���� printf(" ");  ������ ���� ��� ���ش�.
	//-------------------------------------------------------------
	for (iCountY = 0; iCountY < dfSCREEN_HEIGHT; iCountY++)
	{
		for (iCountX = 0; iCountX < dfSCREEN_WIDTH; iCountX++)
		{
			MoveCursor(iCountY, iCountX);
			printf(" ");
		}
	}
}
