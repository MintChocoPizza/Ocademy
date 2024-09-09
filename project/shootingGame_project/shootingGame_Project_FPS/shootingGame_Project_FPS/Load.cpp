#include <Windows.h>
#include <stdio.h>

#include "Load.h"
#include "Buffer.h"
#include "main.h"
#include "Enemy.h"

char* g_StageName;


//------------------------------------------------
// ��������, �¸�, �й���� ������ �ε��Ѵ�.
// 
//------------------------------------------------
bool ld_SceneLoad(void)
{
	id_LoadingScreen();

	switch (ld_IsGameClear())
	{
	case 1:
		g_Scene = CLEAR;
		return false;
		break;
	case 0:
		if (!ld_LodingGame())
			return false;
		if (!enmy_Is_GetStage(g_StageName))
			return false;
		break;
	case -1:
		g_Scene = GAME_ERROR;
		return false;
		break;
	}
	return true;
}



//------------------------------------------------
// �ε��ϴ� ���� ������ ȭ���� ���ۿ� �����Ѵ�.
// 
//------------------------------------------------
void id_LoadingScreen(void)
{
	buff_Sprite_String(dfSCREEN_HEIGHT / 2, "Game Loading......");
}



//------------------------------------------------
// ��� ���������� Ŭ���� �ߴ��� Ȯ���Ѵ�.
// 
//------------------------------------------------
int ld_IsGameClear(void)
{
	errno_t err;
	FILE* pFile;
	int iMaxStage;


	err = fopen_s(&pFile, "MaxStage", "r");
	if (err != NULL)
	{
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�������� ���� �ҷ����� ����");
		return -1;
	}



	fscanf_s(pFile, "%d", &iMaxStage);
	if (g_Stage <= iMaxStage)
		return 0;


	return 1;
}



//------------------------------------------------
// g_Stage�� �˸´� ���������� �ε��Ѵ�.
// 
//------------------------------------------------
bool ld_LodingGame(void)
{
	errno_t err;
	FILE* pFile;
	long lFileSize;
	char* pFileMemory;

	err = fopen_s(&pFile, "StageInfo", "r");
	if (err != NULL)
	{
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�������� ��� �ҷ����� ����");
		g_Scene = GAME_ERROR;
		return false;
	}
	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);



	pFileMemory = (char*)malloc(lFileSize);
	if (pFileMemory == NULL)
	{
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�޸� �Ҵ� ����");
		g_Scene = GAME_ERROR;
		return false;
	}
	fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
	fclose(pFile);


	g_StageName = ld_GetStage(pFileMemory, g_Stage);
	
	free(pFileMemory);
}


char* ld_GetStage(void* p, int size)
{
	char* pFileMemory = (char*)p;
	char* pTemp = pFileMemory;

	int iCnt;

	for (iCnt = 0; iCnt < size; ++iCnt)
	{
		pFileMemory = pTemp;
		while (*pTemp != 0x0A)
			++pTemp;

		*pTemp = '\0';
		++pTemp;
	}

	return pFileMemory;
}


//------------------------------------------------
// ������ ���Ŀ� ���� ������ �д´�. 
// 
// pFileMemory: ���� ������ �޸�
// 
// iMaxContent: ���� ������ �� 
// 
// iLen: format�� ��
// 
// format: %c, %d, %s ���� ����, ����, ���ڿ� 
//		   %C, %D	 ���� ���ڿ� �迭, ���� �迭
//------------------------------------------------
void ld_ReadContent(char* pFileMemory, int iMaxContent, int iLen, const char* format, ...)
{
	int iContent;
	int iCnt;
	bool check = true;


	char* pTemp = pFileMemory;;
	const char* pCopyFormat = format;

	void* pVar;
	va_list ap;

	va_start(ap, format);


	for (iContent = 0; iContent < iMaxContent; ++iContent)
	{
		format = pCopyFormat;
		va_start(ap, format);
		for (iCnt = 0; iCnt < iLen; ++iCnt)
		{
			pFileMemory = pTemp;
			while ((*pTemp != 0x20) && (*pTemp != 0x0A))
				++pTemp;
			*pTemp = '\0';
			++pTemp;

			check = true;
			while (*format && check)
			{
				if (*format == '%')
				{
					format++;
					check = false;

					switch (*format)
					{
					case 'c':
						pVar = va_arg(ap, char*);
						*(char*)pVar = *pFileMemory;
						break;
					case 'd':
						pVar = va_arg(ap, int*);
						*(int*)pVar = atoi(pFileMemory);
						break;
					case 's':
						pVar = va_arg(ap, char**);
						*(char**)pVar = pFileMemory;
						break;
					case 'C':
						pVar = va_arg(ap, char*);
						((char*)pVar)[iContent] = *pFileMemory;
						break;
					case 'D':
						pVar = va_arg(ap, int*);
						((int*)pVar)[iContent] = atoi(pFileMemory);
						break;
					}
				}
				format++;
			}
		}
	}
	va_end(ap);
}