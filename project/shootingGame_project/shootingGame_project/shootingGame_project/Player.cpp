#include <Windows.h>

#include "Player.h"
#include "Buffer.h"
#include "Enemy.h"

st_PLAYER g_stPlayer;
st_BULLET g_stBullet[30];

extern float g_ElapsedTime;
float g_BulletsCoolTime;


//-------------------------------------------------------------------
// 플레이어 최초 데이터 초기화.
// HP : 3 
// y:39, x:19 위치에서 시작 함.
//-------------------------------------------------------------------
void play_Initial_Player(void)
{
	g_stPlayer.iHP = 3;
	g_stPlayer.iDamage = 1;

	g_stPlayer.cPlayerSkin = 'p';
	g_stPlayer.cBulletSkin = 'o';

	g_stPlayer.bVisible = true;

	g_stPlayer.fY = 39.00f;
	g_stPlayer.fX = 19.00f;

	g_BulletsCoolTime = BULLET_COOL_TIME;
}


//----------------------------------------
// 키보드 입력을 받아서 플레이어를 처리한다.
// 
// 게임중 ESC를 누를 경우 false리턴. (종료 처리)
// 
//----------------------------------------
bool play_KeyProcess(void)
{
	// 왼쪽 이동 A.
	if (GetAsyncKeyState('A'))
	{
		// g_stPlayer.iX += -1 * g_stPlayer.iSpeed *;
		g_stPlayer.fX -= 1.00f * PLAYER_X_SPEED * g_ElapsedTime;
	}
	// 오른쪽 이동 D.
	if (GetAsyncKeyState('D'))
	{
		g_stPlayer.fX += 1.00f * PLAYER_X_SPEED * g_ElapsedTime;
	}
	// 위쪽 이동 W.
	if (GetAsyncKeyState('W') & 0x8001)
	{
		g_stPlayer.fY -= 1.00f * PLAYER_Y_SPEED * g_ElapsedTime;
	}
	// 아래쪽 이동 S.
	if (GetAsyncKeyState('S') & 0x8001)
	{
		g_stPlayer.fY += 1.00f * PLAYER_Y_SPEED * g_ElapsedTime;
	}

	//-------------------------------------------------------------
	// 플레이어 이동 반경 제한.
	// 게임 화면에서 플레이어가 이동 가능한 구역을 제한한다.
	//-------------------------------------------------------------

	g_stPlayer.fX = max(g_stPlayer.fX, 0.00f);
	g_stPlayer.fX = min(g_stPlayer.fX, 79.00f);
	g_stPlayer.fY = max(g_stPlayer.fY, 0.00f);
	g_stPlayer.fY = min(g_stPlayer.fY, 23.00f);







	// J 키. (미사일 키)
	if (GetAsyncKeyState('J') & 0x8001)
	{
		play_Make_Bullet();
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
		buff_Sprite_Draw(g_stPlayer.fY, g_stPlayer.fX, g_stPlayer.cPlayerSkin);
}




//--------------------------------------------------------------------
// 총알의 연속 발싸 속도를 조절한다.
// 
//--------------------------------------------------------------------
void play_Bullet_CoolTime(void)
{
	g_BulletsCoolTime += g_ElapsedTime;
}


//--------------------------------------------------------------------
// 총알 배열에 총알을 생성한다.
// 
// 플레이어 위치에 총알을 생성 
//
//--------------------------------------------------------------------
void play_Make_Bullet(void)
{
	int iCnt;
	

	// 비어있는 총알 배열을 찾는다. 
	for (iCnt = 0; iCnt < MAX_PLAYER_BULLET; ++iCnt)
	{
		if (g_BulletsCoolTime < BULLET_COOL_TIME)
			continue;
		if (g_stBullet[iCnt].bVisible == true)
			continue;

		g_stBullet[iCnt].bVisible = true;


		g_stBullet[iCnt].cBulletSkin = g_stPlayer.cBulletSkin;

		g_stBullet[iCnt].fY = g_stPlayer.fY;
		g_stBullet[iCnt].fX = g_stPlayer.fX;

		g_BulletsCoolTime = 0;

		break;
	}
}


//--------------------------------------------------------------------
// 총알의 이동 및 이동 위치에 충돌 판정
// 
//--------------------------------------------------------------------
void play_Move_Bullet(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < MAX_PLAYER_BULLET; ++iCnt)
	{
		if (g_stBullet[iCnt].bVisible == false)
			continue;

		g_stBullet[iCnt].fY += -1 * BULLET_SPEED * g_ElapsedTime;

		if (g_stBullet[iCnt].fY < 0)
		{
			g_stBullet[iCnt].bVisible = false;
		}


	}
}



//--------------------------------------------------------------------
// 스크린 버퍼에 움직이는 총알 그리기
// 
//--------------------------------------------------------------------
void play_Draw_Bullet(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < MAX_PLAYER_BULLET; ++iCnt)
	{
		if (g_stBullet[iCnt].bVisible == false)
			continue;

		buff_Sprite_Draw(g_stBullet[iCnt].fY, g_stBullet[iCnt].fX, g_stBullet[iCnt].cBulletSkin);
	}
}



//--------------------------------------------------------------------
// 플레이어와 충돌 판정
// 
//--------------------------------------------------------------------
bool play_Is_Hit(int iY, int iX, int iDamage)
{
	if (iY == (int)g_stPlayer.fY && iX == (int)g_stPlayer.fX)
	{
		g_stPlayer.iHP -= iDamage;
		return true;
	}

	return false;
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

		if (enmy_Is_Hit((int)g_stBullet[iCnt].fY, (int)g_stBullet[iCnt].fX, g_stPlayer.iDamage))
		{
			g_stBullet[iCnt].bVisible = false;
		}
	}
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
