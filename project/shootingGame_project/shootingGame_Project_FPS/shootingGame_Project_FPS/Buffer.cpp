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
// ����,�Ʊ�,�Ѿ� ���� szScreenBuffer �� �־��ְ�, 
// 1 �������� ������ �������� �� �Լ��� ȣ���Ͽ� ���� -> ȭ�� ���� �׸���.
//--------------------------------------------------------------------
void buff_Buffer_Flip(void)
{
	int iCnt;
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
// ������ ���� ���� '\0'���ڿ��� ���
// 
//--------------------------------------------------------------------
void buff_Sprite_Null(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; ++iCnt)
	{
		g_szScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = '\0';
	}
}



//--------------------------------------------------------------------
// ������ Ư�� ��ġ�� ���ϴ� ���ڿ� ���
// 
// �Է� ���� Y��ǥ�� �߾ӿ� ���ڿ��� ����Ѵ�. (���ۿ� �׸�)
//--------------------------------------------------------------------
void buff_Sprite_String(int iY,const char* pSpriteString)
{
	int iCnt;
	size_t szStringLength;



	szStringLength = strlen(pSpriteString);
	for (iCnt = 0; iCnt < szStringLength; ++iCnt)
	{
		buff_Sprite_Draw(iY, (dfSCREEN_WIDTH / 2) - ((int)szStringLength - 1) / 2 + iCnt, pSpriteString[iCnt]);
	}
}



//--------------------------------------------------------------------
// ���ۿ� ���ȭ���� �׸���.
// 
//--------------------------------------------------------------------
void buff_Sprite_Background(const char* cFileName)
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


		pFileMemory = (char *)malloc(lFileSize);
		if (pFileMemory != NULL)
		{
			//---------------------------------------------------------------------
			// window �ؽ�Ʈ ������ 0d0A���� 0d�� ����� �аԵȴ�.
			//---------------------------------------------------------------------
			fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
			fclose(pFile);


			//---------------------------------------------------------------------
			// ��ũ�� ���ۿ� ���ȭ���� memcpy�� ��
			// 
			// 0A�� NULL���ڷ� �ٲ��ش�.
			//---------------------------------------------------------------------
			memcpy_s(g_szScreenBuffer, sizeof(g_szScreenBuffer), pFileMemory, sizeof(g_szScreenBuffer));
			buff_Sprite_Null();



			free(pFileMemory);
		}
	}
}