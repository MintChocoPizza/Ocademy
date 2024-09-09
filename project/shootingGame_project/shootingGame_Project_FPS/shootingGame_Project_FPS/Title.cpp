#include <stdio.h>
#include <Windows.h>

#include "Title.h"
#include "Buffer.h"

//------------------------------------------------
// ���� ���۽� ó�� Title ȭ���� �����ش�. 
// 
//------------------------------------------------
void tt_SceneTitle(void)
{
	//----------------------------------------------------
	// ���ۿ� Ÿ��Ʋ ȭ���� ��׶��� ����� �����´�.
	// 
	//----------------------------------------------------
	buff_Sprite_Background("Title");



	//------------------------------------------------
	// ���ۿ� ������ Ÿ��Ʋ�� ������ ������ �׸���.
	// 
	//------------------------------------------------
	tt_LoadGameInfo();



	//------------------------------------------------
	// ���ۿ� ���ۿ� ���� ������ �׸���.
	// 
	//------------------------------------------------
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 2, "Move: WASD,  Weapon: J");
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 8, "Press Enter.....");
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 4, "���� �ð� �ø���: N");
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 6, "���� �ð� ���̱�: M");
}



//------------------------------------------------
// ���ۿ� ������ Ÿ��Ʋ�� ������ ������ �׸���.
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
			// ���ۿ� Ÿ��Ʋ ���
			//-------------------------------
			buff_Sprite_String(dfSCREEN_HEIGHT / 2 - 2, st_pGameInfo->cGameName);



			//-------------------------------
			// ���ۿ� ���� ���� ���
			//-------------------------------  
			szStringLength = strlen(st_pGameInfo->cGameVersion);
			for (iCnt = 0; iCnt < szStringLength; ++iCnt)
			{
				buff_Sprite_Draw(dfSCREEN_HEIGHT - 2, 1 + iCnt, st_pGameInfo->cGameVersion[iCnt]);
			}
		}
	}
}