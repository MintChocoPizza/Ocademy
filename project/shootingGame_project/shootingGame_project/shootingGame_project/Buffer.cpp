#include <stdio.h>
#include <memory.h>
#include <Windows.h>

#include "Console.h"
#include "Buffer.h"

//----------------------------------
// 더블 버퍼링을 위한 화면 버퍼
//----------------------------------
char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];




//--------------------------------------------------------------------
// 버퍼의 내용을 화면으로 찍어주는 함수.
//
// 적군,아군,총알 등을 szScreenBuffer 에 넣어주고, 
// 1 프레임이 끝나는 마지막에 본 함수를 호출하여 버퍼 -> 화면 으로 그린다.
//--------------------------------------------------------------------
void buff_Buffer_Flip(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
	{
		cs_MoveCursor(iCnt, 0);
		printf_s(szScreenBuffer[iCnt]);
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
		memset(szScreenBuffer[iCnt], ' ', dfSCREEN_WIDTH);
		szScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = (char)NULL;
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

	szScreenBuffer[iY][iX] = chSprite;
}

//--------------------------------------------------------------------
// 버퍼의 행의 끝에 '\0'문자열을 출력
// 
//--------------------------------------------------------------------
void buff_Sprite_Null(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; ++iCnt)
	{
		szScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = '\0';
	}
}




//--------------------------------------------------------------------
// 버퍼에 배경화면을 그린다.
// 
//--------------------------------------------------------------------
void buff_Sprite_Background(const char * cFileName)
{
	errno_t err;
	FILE* pFile;
	long lFileSize;
	char* pFileMemory;

	err = fopen_s(&pFile, cFileName, "r");
	if (err == NULL)
	{
		fseek(pFile, 0, SEEK_END);
		lFileSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		pFileMemory = (char*)malloc(lFileSize);
		if (pFileMemory != NULL)
		{
			// window 텍스트 파일의 0D0A 에서 0D를 지우고 읽음
			fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
			fclose(pFile);


			memcpy_s(szScreenBuffer, sizeof(szScreenBuffer), pFileMemory, sizeof(szScreenBuffer));
			buff_Sprite_Null();

			free(pFileMemory);
		}
	}
}