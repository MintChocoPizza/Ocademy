#include <Windows.h>
#include <stdio.h>

#include "Load.h"
#include "Buffer.h"
#include "main.h"
#include "Enemy.h"

char* g_StageName;


//------------------------------------------------
// 다음게임, 승리, 패배등의 정보를 로드한다.
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
// 로딩하는 동안 보여줄 화면을 버퍼에 저장한다.
// 
//------------------------------------------------
void id_LoadingScreen(void)
{
	buff_Sprite_String(dfSCREEN_HEIGHT / 2, "Game Loading......");
}



//------------------------------------------------
// 모든 스테이지를 클리어 했는지 확인한다.
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
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "스테이지 정보 불러오기 실패");
		return -1;
	}



	fscanf_s(pFile, "%d", &iMaxStage);
	if (g_Stage <= iMaxStage)
		return 0;


	return 1;
}



//------------------------------------------------
// g_Stage에 알맞는 스테이지를 로드한다.
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
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "스테이지 목록 불러오기 실패");
		g_Scene = GAME_ERROR;
		return false;
	}
	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);



	pFileMemory = (char*)malloc(lFileSize);
	if (pFileMemory == NULL)
	{
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "메모리 할당 실패");
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
// 지정한 형식에 따라 파일을 읽는다. 
// 
// pFileMemory: 읽을 파일의 메모리
// 
// iMaxContent: 읽을 정보의 수 
// 
// iLen: format의 수
// 
// format: %c, %d, %s 각각 문자, 정수, 문자열 
//		   %C, %D	 각각 문자열 배열, 정수 배열
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