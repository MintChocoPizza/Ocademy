#include <stdio.h>
#include <Windows.h>

#include "Title.h"
#include "Buffer.h"

//------------------------------------------------
// 게임 시작시 처음 Title 화면을 보여준다. 
// 
//------------------------------------------------
void tt_SceneTitle(void)
{
	//----------------------------------------------------
	// 버퍼에 타이틀 화면의 백그라운드 배경을 가져온다.
	// 
	//----------------------------------------------------
	buff_Sprite_Background("Title");



	//------------------------------------------------
	// 버퍼에 게임의 타이틀과 게임의 버전을 그린다.
	// 
	//------------------------------------------------
	tt_LoadGameInfo();



	//------------------------------------------------
	// 버퍼에 조작에 대한 설멍을 그린다.
	// 
	//------------------------------------------------
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 2, "Move: WASD,  Weapon: J");
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 8, "Press Enter.....");
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 4, "로직 시간 늘리기: N");
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 6, "로직 시간 줄이기: M");
}



//------------------------------------------------
// 버퍼에 게임의 타이틀과 게임의 버전을 그린다.
// 
//------------------------------------------------
void tt_LoadGameInfo(void)
{
	struct st_GAMEINFO
	{
		char cGameName[30];
		char cGameVersion[10];
	} *st_pGameInfo;
	errno_t err;
	FILE* pFile;
	size_t szStringLength;
	int iCnt;
	
	


	err = fopen_s(&pFile, "GameInfo", "rb");
	if(err == NULL)
	{
		st_pGameInfo = (st_GAMEINFO*)malloc(sizeof(st_GAMEINFO));
		if (st_pGameInfo != NULL)
		{
			fread_s(st_pGameInfo, sizeof(st_GAMEINFO), 1, sizeof(st_GAMEINFO), pFile);
			fclose(pFile);



			//-------------------------------
			// 버퍼에 타이틀 출력
			//-------------------------------
			buff_Sprite_String(dfSCREEN_HEIGHT / 2 - 2, st_pGameInfo->cGameName);



			//-------------------------------
			// 버퍼에 게임 버전 출력
			//-------------------------------  
			szStringLength = strlen(st_pGameInfo->cGameVersion);
			for (iCnt = 0; iCnt < szStringLength; ++iCnt)
			{
				buff_Sprite_Draw(dfSCREEN_HEIGHT - 2, 1 + iCnt, st_pGameInfo->cGameVersion[iCnt]);
			}
		}
	}
}