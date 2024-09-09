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
// ���� ���� �� ó�� Title ȭ���� �����ش�.
// 
//---------------------------------------------------------------------------------
void SceneTitle(void);


//---------------------------------------------------------------------------------
// ��� �������� Ŭ��� Ȯ���Ѵ�.
// 
//---------------------------------------------------------------------------------
RESULT IsGameClear(void);


//---------------------------------------------------------------------------------
// stageInfo ������ �ҷ��� �޸𸮿� �����ϰ�,
// ���� ���������� �´� �� ������ �ҷ��´�.
// 
//---------------------------------------------------------------------------------
bool GameLoading(void);


//---------------------------------------------------------------------------------
// ���� ���� ��ü ������ �ʱ�ȭ
//---------------------------------------------------------------------------------
void Initial(void);

//---------------------------------------------------------------------------------
// ���� Ŭ����
// 
//---------------------------------------------------------------------------------
void GameClear();

//---------------------------------------------------------------------------------
// ���� ����
// 
//---------------------------------------------------------------------------------
void GameOver();

























int main()
{
	int iWaiting = 0;

	//-----------------------------------------------
	// ��Ȯ�� �ð��� ���� Ÿ�� ���͹��� �����Ѵ�.
	//-----------------------------------------------
	timeBeginPeriod(1);
	cs_Initial();
	

	//-----------------------------------------------
	// ���� ������ �ҷ����� ���Ͽ� ���丮 ����
	//-----------------------------------------------  
	SetCurrentDirectory(L"C:\\Users\\user\\Desktop\\procademy_project\\procademy\\project\\shootingGame_Project\\shootingGame_Project\\GameFile");


	//-----------------------------------------------
	// �پ��� ����� �ӵ� ������ ���� Ÿ�̸� ����
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

		// while�� Ż�� ����
		if (iIsOut == true)
			break;

		switch (g_Scene)
		{
		case TITLE:
		{
			// Ű���� �Է�
			if (GetAsyncKeyState(VK_RETURN) & 0x8001)
			{
				// ������
				g_Scene = LOAD;
			}

			// ������
			SceneTitle();

			// ������
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
				// ������ Ŭ�������� ���ߴٸ� ���� ���������� �ε��Ѵ�.
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


			// Ű���� �Է�
			if (!play_KeyProcess())
			{
				g_Scene = OVER;
				break;
			}



			// ������
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







			// ������
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
			printf_s("������ ������������ ����Ǿ����ϴ�.  \n");
			iIsOut = true;
			break;
		}

		Sleep(1000 / FPS);
		}
	}


	



	//-----------------------------------------------
	// Ÿ�� ���͹� �ʱ�ȭ
	//-----------------------------------------------
	timeEndPeriod(1);

	return 0;
}









//----------------------------------------
// ���� ���� �� ó�� Title ȭ���� �����ش�.
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

	// ���ۿ� �׸��� �� ���� �ʱ�ȭ
	buff_Buffer_Clear();

	//-----------------------------------------------------------------------------
	// Ÿ��Ʋ ȭ���� ��׶��� ����� �����´�.
	// 
	//-----------------------------------------------------------------------------
	buff_Sprite_Background("Title");





	//-----------------------------------------------------------------------------
	// ���ȭ�� ���� ������ Ÿ��Ʋ�� ������ ������ �׸���.
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
			// ���ۿ� ���� Ÿ��Ʋ ���
			for (iCnt = 0; iCnt < szStrLen; ++iCnt)
			{
				buff_Sprite_Draw((dfSCREEN_HEIGHT / 2 - 2), (dfSCREEN_WIDTH / 2) - ((int)szStrLen - 1)/2 + iCnt, pGameInfo->cGameName[iCnt]);
			}



			szStrLen = strnlen_s(pGameInfo->cGameVersion, sizeof(pGameInfo->cGameVersion));
			// ���� �ϴܿ� ������ ������ ����Ѵ�. 
			for (iCnt = 0; iCnt < szStrLen; ++iCnt)
			{
				buff_Sprite_Draw(dfSCREEN_HEIGHT - 2, 1 + iCnt, pGameInfo->cGameVersion[iCnt]);
			}

			free(pGameInfo);
		}
	}




	//-----------------------------------------------------------------------------
	// ���ۿ� ���� ������ ���ۿ� �׸���.
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
// ��� �������� Ŭ��� Ȯ���Ѵ�.
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
		printf_s("�������� ���� �ҷ����� ���� \n");
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
// stageInfo ������ �ҷ��� �޸𸮿� �����ϰ�,
// ���� ���������� �´� �� ������ �ҷ��´�.
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
		printf_s("�������� ��� �ҷ����� ���� \n");
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
		printf_s("�޸� �Ҵ� ���� \n");
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
		printf_s("�� ���� �ҷ����� ���� \n");
		free(pFileMemory);
		return false;
	}


	free(pEraserMemory);
	return true;
}








//---------------------------------------------------------------------------------
// ���� ���� ��ü ������ �ʱ�ȭ
//---------------------------------------------------------------------------------
void Initial(void)
{
	play_Initial_Player();
	enmy_Initial();
}




void GameClear()
{
	int iCnt;
	char cGameClear[] = "���� Ŭ����";
	char cGuide[] = "�������� ESC�� ��������";

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
	char cGameOver[] = "���� ����";
	char cGuide[] = "�������� ESC�� ��������";

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