#include <stdio.h>
#include <memory.h>
#include <Windows.h>

#include "Console.h"
#include "Buffer.h"

//----------------------------------
// ���� ���۸��� ���� ȭ�� ����
//----------------------------------
char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];




//--------------------------------------------------------------------
// ������ ������ ȭ������ ����ִ� �Լ�.
//
// ����,�Ʊ�,�Ѿ� ���� szScreenBuffer �� �־��ְ�, 
// 1 �������� ������ �������� �� �Լ��� ȣ���Ͽ� ���� -> ȭ�� ���� �׸���.
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
		memset(szScreenBuffer[iCnt], ' ', dfSCREEN_WIDTH);
		szScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = (char)NULL;
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

	szScreenBuffer[iY][iX] = chSprite;
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
		szScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = '\0';
	}
}




//--------------------------------------------------------------------
// ���ۿ� ���ȭ���� �׸���.
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
			// window �ؽ�Ʈ ������ 0D0A ���� 0D�� ����� ����
			fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
			fclose(pFile);


			memcpy_s(szScreenBuffer, sizeof(szScreenBuffer), pFileMemory, sizeof(szScreenBuffer));
			buff_Sprite_Null();

			free(pFileMemory);
		}
	}
}