#include <stdio.h>
#include <Windows.h>

#include "Console.h"

HANDLE hConsole;

//-------------------------------------------------------------
// �̷��� ���ϴ�.
//
// #include <stdio.h>
// #include <windows.h>
// #incude "Console.h"
//
// void main(void)
// {
//		cs_Initial();
//
//		cs_MoveCursor(0, 0);	// Ŀ���� 0, 0 ��ġ��
//		printf("abcde");		// 0, 0 ��ġ�� �۾��� ����
//		cs_MoveCursor(20, 10);	// Ŀ���� 20, 10 ��ġ��
//		printf("abcde");		// 20, 10 ��ġ�� �۾��� ����
//
//		// ȭ���� ���� ��
//		cs_ClearScreen();
// }
//-------------------------------------------------------------





//--------------------------------------------
// �ܼ� ��� ���� �غ� �۾�
// 
//--------------------------------------------
void cs_Initial(void)
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

	sprintf_s(cCommand, "mode con:cols=%d lines=%d", dfSCREEN_WIDTH, dfSCREEN_HEIGHT+2);
	system(cCommand);
}


//-----------------------------------------------------
// �ܼ� ȭ���� Ŀ���� Y, X ��ǥ�� �̵���Ų��.
//
//-----------------------------------------------------
void cs_MoveCursor(int iPosY, int iPosX)
{
	COORD stCoord;
	stCoord.Y = iPosY;
	stCoord.X = iPosX;

	//-----------------------------------------------------
	// ���ϴ� ��ġ�� Ŀ���� �̵���Ų��.
	//-----------------------------------------------------
	if (SetConsoleCursorPosition(hConsole, stCoord) == 0)
	{
		printf_s("Ŀ�� �̵� �����߽��ϴ�: %d \n", GetLastError());
		return;
	}
}


//-----------------------------------------------------
// �ܼ� ȭ���� �ʱ�ȭ �Ѵ�.
// 
//-----------------------------------------------------
void cs_ClearScreen(void)
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
			cs_MoveCursor(iCountY, iCountX);
			printf(" ");
		}
	}
}