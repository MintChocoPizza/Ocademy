#include <Windows.h>

#include "Player.h"
#include "Buffer.h"
#include "Enemy.h"

st_PLAYER g_stPlayer;
st_BULLET g_stBullet[30];

extern float g_ElapsedTime;
float g_BulletsCoolTime;


//-------------------------------------------------------------------
// �÷��̾� ���� ������ �ʱ�ȭ.
// HP : 3 
// y:39, x:19 ��ġ���� ���� ��.
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
// Ű���� �Է��� �޾Ƽ� �÷��̾ ó���Ѵ�.
// 
// ������ ESC�� ���� ��� false����. (���� ó��)
// 
//----------------------------------------
bool play_KeyProcess(void)
{
	// ���� �̵� A.
	if (GetAsyncKeyState('A'))
	{
		// g_stPlayer.iX += -1 * g_stPlayer.iSpeed *;
		g_stPlayer.fX -= 1.00f * PLAYER_X_SPEED * g_ElapsedTime;
	}
	// ������ �̵� D.
	if (GetAsyncKeyState('D'))
	{
		g_stPlayer.fX += 1.00f * PLAYER_X_SPEED * g_ElapsedTime;
	}
	// ���� �̵� W.
	if (GetAsyncKeyState('W') & 0x8001)
	{
		g_stPlayer.fY -= 1.00f * PLAYER_Y_SPEED * g_ElapsedTime;
	}
	// �Ʒ��� �̵� S.
	if (GetAsyncKeyState('S') & 0x8001)
	{
		g_stPlayer.fY += 1.00f * PLAYER_Y_SPEED * g_ElapsedTime;
	}

	//-------------------------------------------------------------
	// �÷��̾� �̵� �ݰ� ����.
	// ���� ȭ�鿡�� �÷��̾ �̵� ������ ������ �����Ѵ�.
	//-------------------------------------------------------------

	g_stPlayer.fX = max(g_stPlayer.fX, 0.00f);
	g_stPlayer.fX = min(g_stPlayer.fX, 79.00f);
	g_stPlayer.fY = max(g_stPlayer.fY, 0.00f);
	g_stPlayer.fY = min(g_stPlayer.fY, 23.00f);







	// J Ű. (�̻��� Ű)
	if (GetAsyncKeyState('J') & 0x8001)
	{
		play_Make_Bullet();
	}

	// ESC Ű. (����)
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001)
	{
		// ���� ���
		return false;
	}

	return true;
}


//--------------------------------------------------------------------
// ��ũ�� ���ۿ� �÷��̾� �׸���
//--------------------------------------------------------------------
void play_Draw_Player(void)
{
	if (g_stPlayer.bVisible)
		buff_Sprite_Draw(g_stPlayer.fY, g_stPlayer.fX, g_stPlayer.cPlayerSkin);
}




//--------------------------------------------------------------------
// �Ѿ��� ���� �߽� �ӵ��� �����Ѵ�.
// 
//--------------------------------------------------------------------
void play_Bullet_CoolTime(void)
{
	g_BulletsCoolTime += g_ElapsedTime;
}


//--------------------------------------------------------------------
// �Ѿ� �迭�� �Ѿ��� �����Ѵ�.
// 
// �÷��̾� ��ġ�� �Ѿ��� ���� 
//
//--------------------------------------------------------------------
void play_Make_Bullet(void)
{
	int iCnt;
	

	// ����ִ� �Ѿ� �迭�� ã�´�. 
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
// �Ѿ��� �̵� �� �̵� ��ġ�� �浹 ����
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
// ��ũ�� ���ۿ� �����̴� �Ѿ� �׸���
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
// �÷��̾�� �浹 ����
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
// �÷��̾� �Ѿ˰� �� �浹 Ȯ��
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
// �÷��̾�� ���
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
