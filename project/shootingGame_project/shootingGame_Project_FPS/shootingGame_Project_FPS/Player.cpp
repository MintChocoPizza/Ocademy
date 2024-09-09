#include <Windows.h>

#include "Player.h"
#include "Buffer.h"
#include "main.h"
#include "Enemy.h"


st_PLAYER g_stPlayer;
st_BULLET g_stBullet[MAX_PLAYER_BULLET];




//////////////////////////////////////////////////////////////////////
//   �÷��̾�
// 
//////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------
// �÷��̾� ���� ������ �ʱ�ȭ.
// HP : 3 
// �߾�, �Ʒ��� 3ĭ ��ġ���� ���� ��.
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
// Ű���� �Է��� �޾Ƽ� �÷��̾ ó���Ѵ�.
// 
// ������ ESC�� ���� ��� false����. (���� ó��)
// 
//-------------------------------------------------------------------
bool play_KeyProcess(void)
{
	// iPlayerSpeed������ ���� �̵��Ѵ�.
	if (g_CntFps % g_stPlayer.iPlayerSpeed == 0)
	{
		// ���� �̵� A.
		if (GetAsyncKeyState('A'))
		{
			g_stPlayer.iX += -1;
		}
		// ������ �̵� D.
		if (GetAsyncKeyState('D'))
		{
			g_stPlayer.iX += 1;
		}
		// ���� �̵� W.
		if (GetAsyncKeyState('W') & 0x8001)
		{
			g_stPlayer.iY -= 1;
		}
		// �Ʒ��� �̵� S.
		if (GetAsyncKeyState('S') & 0x8001)
		{
			g_stPlayer.iY += 1;
		}
	}
	//-------------------------------------------------------------
	// �÷��̾� �̵� �ݰ� ����.
	// ���� ȭ�鿡�� �÷��̾ �̵� ������ ������ �����Ѵ�.
	//-------------------------------------------------------------

	g_stPlayer.iX = max(g_stPlayer.iX, 0);
	g_stPlayer.iX = min(g_stPlayer.iX, 79);
	g_stPlayer.iY = max(g_stPlayer.iY, 0);
	g_stPlayer.iY = min(g_stPlayer.iY, 23);




	// J Ű. (�̻��� Ű)
	if(g_CntFps % g_stPlayer.iBulletCoolTime == 0)
	{
		if (GetAsyncKeyState('J') & 0x8001)
		{
			play_Make_Bullet();
		}
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
		buff_Sprite_Draw(g_stPlayer.iY, g_stPlayer.iX, g_stPlayer.cPlayerSkin);
}

//--------------------------------------------------------------------
// �÷��̾�� �浹 ����
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

//--------------------------------------------------------------------
// ���������� Ŭ���� �ߴ��� �����ߴ��� Ȯ���Ѵ�.
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
//    �Ѿ�
// 
//////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------
// �Ѿ� �迭�� �Ѿ��� �����Ѵ�.
// 
//--------------------------------------------------------------------
void play_Make_Bullet(void)
{
	int iCnt;


	// ����ִ� �Ѿ� �迭�� ã�� ����
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
// �Ѿ��� �̵�
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
// ��ũ�� ���ۿ� �Ѿ� �׸���
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

		if (enmy_Is_Hit(g_stBullet[iCnt].iY, g_stBullet[iCnt].iX, g_stPlayer.iDamage))
		{
			g_stBullet[iCnt].bVisible = false;
		}
	}
}


