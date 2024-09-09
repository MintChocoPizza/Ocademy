#include <stdio.h>
#include <Windows.h>

#include "Console.h"
#include "Buffer.h"
#include "Player.h"
#include "Enemy.h"

#define FPS 60

enum SCENE
{
	TITLE,
	LOAD,
	GAME,
	CLEAR,
	OVER,
	GAME_ERROR
};

typedef enum
{
	FILE_ERROR = -1,
	RESULT_FALSE = 0,
	RESULT_TRUE = 1
} RESULT;

int g_Scene;
int g_Stage = 1;

int iIsOut = 0;

LARGE_INTEGER g_Timer;
LARGE_INTEGER g_Start;
LARGE_INTEGER g_End;
float g_ElapsedTime;


//---------------------------------------------------------------------------------
// 게임 시작 시 처음 Title 화면을 보여준다.
// 
//---------------------------------------------------------------------------------
void SceneTitle(void);


//---------------------------------------------------------------------------------
// 모든 스테이지 클리어를 확인한다.
// 
//---------------------------------------------------------------------------------
RESULT IsGameClear(void);


//---------------------------------------------------------------------------------
// stageInfo 파일을 불러와 메모리에 저장하고,
// 현재 스테이지에 맞는 맵 정보를 불러온다.
// 
//---------------------------------------------------------------------------------
bool GameLoading(void);


//---------------------------------------------------------------------------------
// 게임 시작 전체 데이터 초기화
//---------------------------------------------------------------------------------
void Initial(void);

//---------------------------------------------------------------------------------
// 게임 클리어
// 
//---------------------------------------------------------------------------------
void GameClear();

//---------------------------------------------------------------------------------
// 게임 오버
// 
//---------------------------------------------------------------------------------
void GameOver();

























int main()
{
	int iWaiting = 0;

	//-----------------------------------------------
	// 정확한 시간을 위해 타임 인터벌을 변경한다.
	//-----------------------------------------------
	timeBeginPeriod(1);
	cs_Initial();
	

	//-----------------------------------------------
	// 게임 정보를 불러오기 위하여 디렉토리 변경
	//-----------------------------------------------  
	SetCurrentDirectory(L"C:\\Users\\user\\Desktop\\procademy_project\\procademy\\project\\shootingGame_Project\\shootingGame_Project\\GameFile");


	//-----------------------------------------------
	// 다양한 요소의 속도 조절을 위한 타이머 세팅
	// 
	//-----------------------------------------------
	QueryPerformanceFrequency(&g_Timer);
	QueryPerformanceCounter(&g_Start);
	QueryPerformanceCounter(&g_End);

	
	while (1)
	{
		QueryPerformanceCounter(&g_End);
		g_ElapsedTime = (float)(g_End.QuadPart - g_Start.QuadPart) / g_Timer.QuadPart;
		g_Start = g_End;

		// while문 탈출 구문
		if (iIsOut == true)
			break;

		switch (g_Scene)
		{
		case TITLE:
		{
			// 키보드 입력
			if (GetAsyncKeyState(VK_RETURN) & 0x8001)
			{
				// 로직부
				g_Scene = LOAD;
			}

			// 로직부
			SceneTitle();

			// 랜더부
			buff_Buffer_Flip();
			break;
		}
		case LOAD:
		{
			RESULT isClear = IsGameClear();
			switch (isClear)
			{
			case RESULT_TRUE:
				g_Scene = CLEAR;
				break;
			case RESULT_FALSE:
			{
				//---------------------------------------------------------
				// 게임을 클리어하지 못했다면 다음 스테이지를 로딩한다.
				// 
				//---------------------------------------------------------
				Initial();
				if (GameLoading())
					g_Scene = GAME;
				else
					g_Scene = GAME_ERROR;
				break;
			}
			case FILE_ERROR:
				g_Scene = GAME_ERROR;
				break;
			}
			break;
		}
		case GAME:
		{


			// 키보드 입력
			if (!play_KeyProcess())
			{
				g_Scene = OVER;
				break;
			}



			// 로직부
			buff_Buffer_Clear();


			enmy_CoolTime();
			play_Bullet_CoolTime();

			play_Move_Bullet();
			enmy_Move_Enemy();
			enmy_Shoot_Bullet();
			enmy_Move_Bullet();



			enmy_Bullet_Hit_Player();
			play_Bullet_Hit_Enemy();
			if (play_Is_End())
			{
				g_Scene = OVER;
				break;
			}
			if (enmy_Is_Win())
			{
				g_Scene = LOAD;
				g_Stage++;
				break;
			}


			enmy_Draw_Enemy();
			enmy_Draw_Enemy_Bullet();
			play_Draw_Player();
			play_Draw_Bullet();







			// 랜더부
			buff_Buffer_Flip();

			break;
		}
		case CLEAR:
		{
			GameClear();

			break;
		}
		case OVER:
		{
			GameOver();

			break;
		}
		case GAME_ERROR:
		{
			printf_s("게임이 비정상적으로 종료되었습니다.  \n");
			iIsOut = true;
			break;
		}

		Sleep(1000 / FPS);
		}
	}


	



	//-----------------------------------------------
	// 타임 인터벌 초기화
	//-----------------------------------------------
	timeEndPeriod(1);

	return 0;
}









//----------------------------------------
// 게임 시작 시 처음 Title 화면을 보여준다.
// 
//----------------------------------------
void SceneTitle(void)
{
	struct st_GAMEINFO
	{
		char cGameName[30];
		char cGameVersion[10];
	} *pGameInfo;

	errno_t err;
	FILE* pFile;
	size_t szStrLen;

	char cControls[] = "Move: WASD,  Weapon: J";
	char cGuide[] = "Press Enter.....";


	int iCnt;

	// 버퍼에 그리기 전 버퍼 초기화
	buff_Buffer_Clear();

	//-----------------------------------------------------------------------------
	// 타이틀 화면의 백그라운드 배경을 가져온다.
	// 
	//-----------------------------------------------------------------------------
	buff_Sprite_Background("Title");





	//-----------------------------------------------------------------------------
	// 배경화면 위에 게임의 타이틀과 게임의 버전을 그린다.
	// 
	//-----------------------------------------------------------------------------
	err = fopen_s(&pFile, "GameInfo", "rb");
	if (err == NULL)
	{
		pGameInfo = (st_GAMEINFO*)malloc(sizeof(st_GAMEINFO));
		if (pGameInfo != NULL)
		{
			fread_s(pGameInfo, sizeof(st_GAMEINFO), 1, sizeof(st_GAMEINFO), pFile);
			fclose(pFile);

			szStrLen = strnlen_s(pGameInfo->cGameName, sizeof(pGameInfo->cGameName));
			// 버퍼에 게임 타이틀 출력
			for (iCnt = 0; iCnt < szStrLen; ++iCnt)
			{
				buff_Sprite_Draw((dfSCREEN_HEIGHT / 2 - 2), (dfSCREEN_WIDTH / 2) - ((int)szStrLen - 1)/2 + iCnt, pGameInfo->cGameName[iCnt]);
			}



			szStrLen = strnlen_s(pGameInfo->cGameVersion, sizeof(pGameInfo->cGameVersion));
			// 좌측 하단에 게임의 버전을 출력한다. 
			for (iCnt = 0; iCnt < szStrLen; ++iCnt)
			{
				buff_Sprite_Draw(dfSCREEN_HEIGHT - 2, 1 + iCnt, pGameInfo->cGameVersion[iCnt]);
			}

			free(pGameInfo);
		}
	}




	//-----------------------------------------------------------------------------
	// 조작에 대한 설명을 버퍼에 그린다.
	// 
	//-----------------------------------------------------------------------------
	for (iCnt = 0; iCnt < sizeof(cControls) - 1; ++iCnt)
	{
		buff_Sprite_Draw((dfSCREEN_HEIGHT / 2 + 2), (dfSCREEN_WIDTH / 2) - (sizeof(cControls)/2 - 1) + iCnt, cControls[iCnt]);
	}
	for (iCnt = 0; iCnt < sizeof(cGuide) - 1; ++iCnt)
	{
		buff_Sprite_Draw((dfSCREEN_HEIGHT / 2 + 4), (dfSCREEN_WIDTH / 2) - (sizeof(cGuide) / 2 - 1) + iCnt, cGuide[iCnt]);
	}

	
	
}



//----------------------------------------
// 모든 스테이지 클리어를 확인한다.
// 
//----------------------------------------
RESULT IsGameClear(void)
{
	FILE* pFile;
	errno_t err;
	int iMaxStage;

	err = fopen_s(&pFile, "MaxStage", "r");
	if (err != NULL)
	{
		system("cls");
		printf_s("스테이지 정보 불러오기 실패 \n");
		return FILE_ERROR;
	}

	fscanf_s(pFile, "%d", &iMaxStage);

	if (g_Stage <= iMaxStage)
	{
		return RESULT_FALSE;
	}

	return RESULT_TRUE;
}

//----------------------------------------
// stageInfo 파일을 불러와 메모리에 저장하고,
// 현재 스테이지에 맞는 맵 정보를 불러온다.
// 
//----------------------------------------
bool GameLoading(void)
{
	FILE* pFile;
	errno_t err;

	char* pFileMemory;
	char* pEraserMemory;
	char* pTemp;
	long lFileSize;
	int iCnt;
	char cStageName[50];

	err = fopen_s(&pFile, "StageInfo", "r");
	if (err != NULL)
	{
		system("cls");
		printf_s("스테이지 목록 불러오기 실패 \n");
		return false;
	}
	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	pFileMemory = (char*)malloc(lFileSize);
	pEraserMemory = pFileMemory;
	if (pFileMemory == NULL)
	{
		system("cls");
		printf_s("메모리 할당 실패 \n");
		return false;
	}

	fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
	fclose(pFile);

	pTemp = pFileMemory;
	for (iCnt = 0; iCnt < g_Stage; ++iCnt)
	{
		// fgets(cStageName, sizeof(cStageName), pFile);
		pFileMemory = pTemp;
		while (*pTemp != 0x0A)
			++pTemp;

		*pTemp = '\0';
		++pTemp;
	}


	if (!enmy_Is_Get_Info(pFileMemory))
	{
		system("cls");
		printf_s("적 정보 불러오기 실패 \n");
		free(pFileMemory);
		return false;
	}


	free(pEraserMemory);
	return true;
}








//---------------------------------------------------------------------------------
// 게임 시작 전체 데이터 초기화
//---------------------------------------------------------------------------------
void Initial(void)
{
	play_Initial_Player();
	enmy_Initial();
}




void GameClear()
{
	int iCnt;
	char cGameClear[] = "게임 클리어";
	char cGuide[] = "끝내려면 ESC를 누르세요";

	if (!play_KeyProcess())
	{
		iIsOut = true;
	}

	buff_Buffer_Clear();
	buff_Sprite_Background("Title");

	for (iCnt = 0; iCnt < sizeof(cGameClear) - 1; ++iCnt)
	{
		buff_Sprite_Draw((dfSCREEN_HEIGHT / 2 + 2), (dfSCREEN_WIDTH / 2) - (sizeof(cGameClear) / 2 - 1) + iCnt, cGameClear[iCnt]);
	}
	for (iCnt = 0; iCnt < sizeof(cGuide) - 1; ++iCnt)
	{
		buff_Sprite_Draw((dfSCREEN_HEIGHT / 2 + 4), (dfSCREEN_WIDTH / 2) - (sizeof(cGuide) / 2 - 1) + iCnt, cGuide[iCnt]);
	}


	buff_Buffer_Flip();
}




void GameOver()
{
	int iCnt;
	char cGameOver[] = "게임 오버";
	char cGuide[] = "끝내려면 ESC를 누르세요";

	if (!play_KeyProcess())
	{
		iIsOut = true;
	}

	buff_Buffer_Clear();
	buff_Sprite_Background("Title");

	for (iCnt = 0; iCnt < sizeof(cGameOver) - 1; ++iCnt)
	{
		buff_Sprite_Draw((dfSCREEN_HEIGHT / 2 + 2), (dfSCREEN_WIDTH / 2) - (sizeof(cGameOver) / 2 - 1) + iCnt, cGameOver[iCnt]);
	}
	for (iCnt = 0; iCnt < sizeof(cGuide) - 1; ++iCnt)
	{
		buff_Sprite_Draw((dfSCREEN_HEIGHT / 2 + 4), (dfSCREEN_WIDTH / 2) - (sizeof(cGuide) / 2 - 1) + iCnt, cGuide[iCnt]);
	}


	buff_Buffer_Flip();
}