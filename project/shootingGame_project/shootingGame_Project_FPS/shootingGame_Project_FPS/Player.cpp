#include <Windows.h>

#include "Player.h"
#include "Buffer.h"
#include "main.h"
#include "Enemy.h"


st_PLAYER g_stPlayer;
st_BULLET g_stBullet[MAX_PLAYER_BULLET];




//////////////////////////////////////////////////////////////////////
//   플레이어
// 
//////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------
// 플레이어 최초 데이터 초기화.
// HP : 3 
// 중앙, 아래서 3칸 위치에서 시작 함.
//-------------------------------------------------------------------
void play_Initial_Player(void)
{
	g_stPlayer.iHP = 3;
	
	g_stPlayer.cPlayerSkin = 'P';
	g_stPlayer.cBulletSkin = 'o';

	g_stPlayer.iDamage = 1;

	g_stPlayer.iY = dfSCREEN_HEIGHT - 3;
	g_stPlayer.iX = (dfSCREEN_WIDTH - 1) / 2;

	g_stPlayer.iPlayerSpeed = 2;
	g_stPlayer.iBulletCoolTime = 5;

	g_stPlayer.bVisible = true;
}

//-------------------------------------------------------------------
// 키보드 입력을 받아서 플레이어를 처리한다.
// 
// 게임중 ESC를 누를 경우 false리턴. (종료 처리)
// 
//-------------------------------------------------------------------
bool play_KeyProcess(void)
{
	// iPlayerSpeed프레임 마다 이동한다.
	if (g_CntFps % g_stPlayer.iPlayerSpeed == 0)
	{
		// 왼쪽 이동 A.
		if (GetAsyncKeyState('A'))
		{
			g_stPlayer.iX += -1;
		}
		// 오른쪽 이동 D.
		if (GetAsyncKeyState('D'))
		{
			g_stPlayer.iX += 1;
		}
		// 위쪽 이동 W.
		if (GetAsyncKeyState('W') & 0x8001)
		{
			g_stPlayer.iY -= 1;
		}
		// 아래쪽 이동 S.
		if (GetAsyncKeyState('S') & 0x8001)
		{
			g_stPlayer.iY += 1;
		}
	}
	//-------------------------------------------------------------
	// 플레이어 이동 반경 제한.
	// 게임 화면에서 플레이어가 이동 가능한 구역을 제한한다.
	//-------------------------------------------------------------

	g_stPlayer.iX = max(g_stPlayer.iX, 0);
	g_stPlayer.iX = min(g_stPlayer.iX, 79);
	g_stPlayer.iY = max(g_stPlayer.iY, 0);
	g_stPlayer.iY = min(g_stPlayer.iY, 23);




	// J 키. (미사일 키)
	if(g_CntFps % g_stPlayer.iBulletCoolTime == 0)
	{
		if (GetAsyncKeyState('J') & 0x8001)
		{
			play_Make_Bullet();
		}
	}
	// ESC 키. (종료)
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001)
	{
		// 종료 방법
		return false;
	}

	return true;
}

//--------------------------------------------------------------------
// 스크린 버퍼에 플레이어 그리기
//--------------------------------------------------------------------
void play_Draw_Player(void)
{
	if (g_stPlayer.bVisible)
		buff_Sprite_Draw(g_stPlayer.iY, g_stPlayer.iX, g_stPlayer.cPlayerSkin);
}

//--------------------------------------------------------------------
// 플레이어와 충돌 판정
// 
//--------------------------------------------------------------------
bool play_Is_Hit(int iY, int iX, int iDamage)
{
	if (iY == g_stPlayer.iY && iX == g_stPlayer.iX)
	{
		g_stPlayer.iHP -= iDamage;
		return true;
	}

	return false;
}

//--------------------------------------------------------------------
// 플레이어와 사망
// 
//--------------------------------------------------------------------
bool play_Is_End(void)
{
	if (g_stPlayer.iHP == 0)
	{
		g_stPlayer.bVisible = false;
		return true;
	}

	return false;
}

//--------------------------------------------------------------------
// 스테이지를 클리어 했는지 실패했는지 확인한다.
// 
//--------------------------------------------------------------------
void play_Is_Stage_Clear(void)
{
	if (enmy_Is_Win())
	{
		g_Scene = LOAD;
		g_Stage++;
	}

	if (play_Is_End())
	{
		g_Scene = OVER;
	}
}









//////////////////////////////////////////////////////////////////////
//    총알
// 
//////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------
// 총알 배열에 총알을 생성한다.
// 
//--------------------------------------------------------------------
void play_Make_Bullet(void)
{
	int iCnt;


	// 비어있는 총알 배열을 찾아 저장
	for (iCnt = 0; iCnt < MAX_PLAYER_BULLET; ++iCnt)
	{
		if (g_stBullet[iCnt].bVisible == true)
			continue;


		g_stBullet[iCnt].bVisible = true;

		g_stBullet[iCnt].cBulletSkin = g_stPlayer.cBulletSkin;

		g_stBullet[iCnt].iY = g_stPlayer.iY;
		g_stBullet[iCnt].iX = g_stPlayer.iX;
		break;
	}
}

//--------------------------------------------------------------------
// 총알의 이동
// 
//--------------------------------------------------------------------
void play_Move_Bullet(void)
{
	int iCnt;
	
	
	for (iCnt = 0; iCnt < MAX_PLAYER_BULLET; ++iCnt)
	{
		if (g_stBullet[iCnt].bVisible == false)
			continue;


		g_stBullet[iCnt].iY -= 1;

		if (g_stBullet[iCnt].iY < 0)
		{
			g_stBullet[iCnt].bVisible = false;
		}
	}
}

//--------------------------------------------------------------------
// 스크린 버퍼에 총알 그리기
// 
//--------------------------------------------------------------------
void play_Draw_Bullet(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < MAX_PLAYER_BULLET; ++iCnt)
	{
		if (g_stBullet[iCnt].bVisible == false)
			continue;

		buff_Sprite_Draw(g_stBullet[iCnt].iY, g_stBullet[iCnt].iX, g_stBullet[iCnt].cBulletSkin);
	}
}

//--------------------------------------------------------------------
// 플레이어 총알과 적 충돌 확인
// 
//--------------------------------------------------------------------
void play_Bullet_Hit_Enemy(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < MAX_PLAYER_BULLET; ++iCnt)
	{
		if (g_stBullet[iCnt].bVisible == false)
			continue;

		if (enmy_Is_Hit(g_stBullet[iCnt].iY, g_stBullet[iCnt].iX, g_stPlayer.iDamage))
		{
			g_stBullet[iCnt].bVisible = false;
		}
	}
}


