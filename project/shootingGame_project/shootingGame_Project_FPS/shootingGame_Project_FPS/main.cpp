#include <stdio.h>
#include <Windows.h>

#include "main.h"
#include "Console.h"
#include "Buffer.h"
#include "KeyBoard.h"
#include "Title.h"
#include "Load.h"
#include "Player.h"
#include "Enemy.h"

#pragma comment (lib,"winmm.lib")



//---------------------------------
// 시간 측정 변수
//---------------------------------
DWORD g_Start;
DWORD g_End;
DWORD g_OneSecond;


//---------------------------------
// while문 탈출 확인 
//---------------------------------
bool g_IsOut;


//------------------------------------------------
// Scene 변경을 위한 변수 선언 
// 
//------------------------------------------------
enum SCENE g_Scene;



//------------------------------------------------
// 현재 스테이지 변수
// 
//------------------------------------------------
int g_Stage = 1;



//------------------------------------------------
// 프레임에 의존하는 게임 
// 프레임 변수
// 
//------------------------------------------------
unsigned int g_CntFps;
unsigned int g_LogicFps;
unsigned int g_RanderFps;
unsigned int cpLogicFps = 0;
unsigned int cpRanderFps = 0;


//------------------------------------------------
// 강제로 랜더링 시간 추가하기
// 
//------------------------------------------------
DWORD g_AddTime = 0;




//------------------------------------------------
// 50프레임 프레임 스킵
// 
//------------------------------------------------
bool FpsSkip(void);



void GameClear();
void GameOver();



int main()
{

	timeBeginPeriod(1);
	cs_Initial();

	//------------------------------------------------
	// 게임 정보를 불러오기 위하여 디렉토리 변경
	// 
	//------------------------------------------------
	SetCurrentDirectory(L"C:\\Users\\User\\Desktop\\procademy_project\\procademy\\project\\shootingGame_Project_FPS\\shootingGame_Project_FPS\\GameFile");


	//------------------------------------------------
	// 프레임 조절을 위한 시간 측정
	// 
	//------------------------------------------------
	g_Start = timeGetTime();
	g_OneSecond = g_Start;





	while (1)
	{	
		//---------------------------------
		// 로직 시간 추가하기
		// 
		//---------------------------------
		Sleep(g_AddTime);

		//---------------------------------
		// while문 탈출 구문
		//---------------------------------
		if (g_IsOut == true)
			break;
		

		//---------------------------------
		// 로직 프레임 증가
		//---------------------------------
		g_LogicFps++;
		g_CntFps++;

		//---------------------------------
		// 키보드 입력
		// 
		//---------------------------------
		kb_KeyProcess();


		//---------------------------------
		// 로직부
		// 
		// 버퍼에 그리기 전 버퍼 초기화
		//---------------------------------
		buff_Buffer_Clear();
		switch (g_Scene)
		{
		case TITLE:
		{
			tt_SceneTitle();
			break;
		}
		case LOAD:
		{
			if (ld_SceneLoad())
			{
				play_Initial_Player();
				g_Scene = GAME;
			}
			break;
		}
		case GAME:
		{
			play_Move_Bullet();
			enmy_Move_Enemy();
			enmy_Shoot_Bullet();
			enmy_Move_Bullet();

			play_Bullet_Hit_Enemy();
			enmy_Bullet_Hit_Player();

			play_Draw_Player();
			play_Draw_Bullet();
			enmy_Draw_Enemy();
			enmy_Draw_Enemy_Bullet();
			
			play_Is_Stage_Clear();

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
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "게임이 비정상적으로 종료되었습니다.");
			g_IsOut = true;
			break;
		}
		}


		//---------------------------------------------------
		// 랜더부
		// 
		// 게임을 나가기 전에 무조건 오류 메시지 출력
		//---------------------------------------------------
		if(FpsSkip() || g_IsOut)
		{
			buff_Buffer_Flip();
			g_RanderFps++;
		}
		cs_MoveCursor(dfSCREEN_HEIGHT , 0);
		printf_s("                                       ");
		cs_MoveCursor(dfSCREEN_HEIGHT, 0);
		printf_s("로직: %d   -   랜더 :%d   -   추가부하: %d \n", cpLogicFps, cpRanderFps, g_AddTime);

	}


	timeEndPeriod(1);
	return 0;
}



//------------------------------------------------
// 50프레임 프레임 스킵
// 
//------------------------------------------------
bool FpsSkip(void)
{
	bool ret;
	g_End = timeGetTime();
	if (g_End - g_Start < 20)
	{
		g_Start += 20;
		Sleep(g_Start - g_End);
		ret = true;
	}
	else
	{
		g_Start += 20;
		ret = false;
	}
	// 다른 구현
	// 
	// int time = timeGetTime();
	// 
	// 프레임이 한프레임 이상 밀린경우
	// if(time - OldFrameTick >= 20)
	// {
	//	OldFrameTic +=;
	//	++LogicFPS;
	//	continue;
	// }
	// Sleep(20 - (time - OldFrameTick));
	// 
	// 랜더링
	//


	if (g_End - g_OneSecond >= 1000)
	{
		g_OneSecond += 1000;
		cpLogicFps = g_LogicFps;
		cpRanderFps = g_RanderFps;

		g_LogicFps = 0;
		g_RanderFps = 0;
	}

	return ret;
}


void GameClear()
{
	int iCnt;
	char cGameClear[] = "게임 클리어";
	char cGuide[] = "끝내려면 ESC를 누르세요";

	if (!play_KeyProcess())
	{
		g_IsOut = true;
	}

	buff_Buffer_Clear();
	buff_Sprite_Background("Title");


	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 2, "게임 클리어");
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 4, "끝내려면 ESC를 누르세요");

}


void GameOver()
{
	int iCnt;
	char cGameOver[] = "게임 오버";
	char cGuide[] = "끝내려면 ESC를 누르세요";

	if (!play_KeyProcess())
	{
		g_IsOut = true;
	}

	buff_Buffer_Clear();
	buff_Sprite_Background("Title");


	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 2, "게임 오버");
	buff_Sprite_String(dfSCREEN_HEIGHT / 2 + 4, "끝내려면 ESC를 누르세요");

}