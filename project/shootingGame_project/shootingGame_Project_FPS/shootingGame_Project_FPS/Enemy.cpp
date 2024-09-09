#include <stdio.h>
#include <Windows.h>

#include <stdarg.h>
#include <math.h>

#include "Enemy.h"
#include "main.h"
#include "Buffer.h"
#include "Load.h"
#include "Player.h"

st_ENEMY g_stEnemy[MAX_ENEMY];
st_ENEMY_BULLET g_stEnemyBullet[MAX_ENEMY_BULLET];

int g_MaxEnemy;				// ���� �������� �ִ� ���� ��
int g_EnemyCount;			// ���� �����ִ� ���� ��




///////////////////////////////////////////////////////
// �� 
// 
///////////////////////////////////////////////////////
// ----------------------------------------------------
// �� ������ �ҷ��´�.
// 
// ----------------------------------------------------
bool enmy_Is_GetStage(const char* cEnemyInfo)
{
	FILE* pFile;
	errno_t err;
	long lFileSize;
	char* pFileMemory = NULL;
	char* pEraserMemory = NULL;
	char* pTemp;


	int iCnt;
	char cEnemySkin;
	int iHP;
	int iY;
	int iX;


	char* pMovingFileText = NULL;
	int iMaxMoving;
	int iDireY[MAX_MOVING_PATTERN];				// ������ ��� ��ǥ
	int iDireX[MAX_MOVING_PATTERN];				//
	int iMovingCoolTime[MAX_MOVING_PATTERN];		// �����ӿ� ���� ��Ÿ��


	char* pBulletFileText = NULL;
	int iShotOne;
	char cBulletSkin[MAX_SHOT_ONE];
	int iDamage[MAX_SHOT_ONE];
	int iBulletDireY[MAX_SHOT_ONE];
	int iBulletDireX[MAX_SHOT_ONE];
	int iBulletCoolTime[MAX_SHOT_ONE];
	int iBulletSpeed[MAX_SHOT_ONE];



	// �������� ���� ����
	err = fopen_s(&pFile, cEnemyInfo, "r");
	if (err != NULL)
	{
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�������� ���� ���� ����");
		g_Scene = GAME_ERROR;
		return false;
	}
	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	// �������� ������ �޸𸮿� ����
	pFileMemory = (char*)malloc(lFileSize);
	if (pFileMemory == NULL)
	{
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�޸� �Ҵ� ����");
		g_Scene = GAME_ERROR;
		return false;
	}
	fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
	fclose(pFile);


	// ������� �� ������ ���� ���Ѵ�. 
	pTemp = pFileMemory;
	pEraserMemory = pFileMemory;
	while (*pTemp != 0x0A)
		++pTemp;
	*pTemp = '\0';
	++pTemp;
	g_MaxEnemy = atoi(pFileMemory);
	g_EnemyCount = g_MaxEnemy;
	pFileMemory = pTemp;




	for (iCnt = 0; iCnt < g_MaxEnemy; ++iCnt)
	{
		FILE* l_pFile;
		errno_t l_err;
		long l_lFileSize;
		char* l_pFileMemory;
		char* l_pEraserMemory;
		char* l_pTemp;


		//---------------------------------------------------------------------------------
		// �� ���ֵ��� ������ ������ �迭�� �����Ѵ�.
		// 
		// ���� ����
		// 
		// ���: �� ������ ��
		// 
		// ��skin		HP		y��ǥ			x��ǥ			������.txt		�Ѿ�.txt
		//---------------------------------------------------------------------------------
		ld_ReadContent(pFileMemory, 1, 6, "%c %d %d %d %s %s", &cEnemySkin, &iHP, &iY, &iX, &pMovingFileText, &pBulletFileText);

		// �� �迭�� ������ ���� ������ �����Ѵ�.
		g_stEnemy[iCnt].bVisible = true;
		g_stEnemy[iCnt].cEnemySkin = cEnemySkin;
		g_stEnemy[iCnt].iHp = iHP;
		g_stEnemy[iCnt].iY = iY;
		g_stEnemy[iCnt].iX = iX;





		//---------------------------------------------------------------------------------
		// moving���Ͽ��� �� �������� �ҷ��´�.
		// 
		//---------------------------------------------------------------------------------
		if (pMovingFileText == NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�� ������ ���� ���� ����1");
			g_Scene = GAME_ERROR;
			return false;
		}
		l_err = fopen_s(&l_pFile, pMovingFileText, "r");
		if(l_err != NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�� ������ ���� ���� ����2");
			g_Scene = GAME_ERROR;
			return false;
		}
		fseek(l_pFile, 0, SEEK_END);
		l_lFileSize = ftell(l_pFile);
		fseek(l_pFile, 0, SEEK_SET);
		l_pFileMemory = (char*)malloc(l_lFileSize);
		if (pFileMemory == NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�޸� �Ҵ� ����");
			g_Scene = GAME_ERROR;
			return false;
		}
		fread_s(l_pFileMemory, l_lFileSize, 1, l_lFileSize, l_pFile);
		fclose(l_pFile);

		// ������� �� ������ ���� ���Ѵ�. 
		l_pTemp = l_pFileMemory;
		l_pEraserMemory = l_pFileMemory;
		while (*l_pTemp != 0x0A)
			++l_pTemp;
		*l_pTemp = '\0';
		++l_pTemp;
		iMaxMoving = atoi(l_pFileMemory);
		l_pFileMemory = l_pTemp;

		//---------------------------------------------------------------------------------
		// ������ ���� ����
		// 
		// 
		// ���: ������ ����
		// 
		// �̵��� ���� ��Ÿ��			y�����ǥ			x�����ǥ
		//---------------------------------------------------------------------------------
		ld_ReadContent(l_pFileMemory, iMaxMoving, 3, "%D %D %D", iMovingCoolTime, iDireY, iDireX);
		free(l_pEraserMemory);

		//---------------------------------------------------------------------------------
		// �� �������� �����Ѵ�.
		// 
		//---------------------------------------------------------------------------------
		g_stEnemy[iCnt].iMaxMovingPattern = iMaxMoving;
		memcpy_s(g_stEnemy[iCnt].iMovingCoolTime, sizeof(g_stEnemy[iCnt].iMovingCoolTime), iMovingCoolTime, sizeof(iMovingCoolTime));
		memcpy_s(g_stEnemy[iCnt].iDireY, sizeof(g_stEnemy[iCnt].iDireY), iDireY, sizeof(iDireY));
		memcpy_s(g_stEnemy[iCnt].iDireX, sizeof(g_stEnemy[iCnt].iDireX), iDireX, sizeof(iDireX));

		






		//---------------------------------------------------------------------------------
		// bullet ���Ͽ��� �Ѿ��� �ҷ��´�.
		// 
		//---------------------------------------------------------------------------------
		if (pBulletFileText == NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�Ѿ� ���� ���� ����1");
			g_Scene = GAME_ERROR;
			return false;
		}
		l_err = fopen_s(&l_pFile, pBulletFileText, "r");
		if(l_err != NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�Ѿ� ���� ���� ����2");
			g_Scene = GAME_ERROR;
			return false;
		}
		fseek(l_pFile, 0, SEEK_END);
		l_lFileSize = ftell(l_pFile);
		fseek(l_pFile, 0, SEEK_SET);
		l_pFileMemory = (char*)malloc(l_lFileSize);
		if (l_pFileMemory == NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "�޸� �Ҵ� ����");
			g_Scene = GAME_ERROR;
			return false;
		}
		fread_s(l_pFileMemory, l_lFileSize, 1, l_lFileSize, l_pFile);
		fclose(l_pFile);

		// ������� �ѹ��� �߽��ϴ� �Ѿ��� ����
		l_pTemp = l_pFileMemory;
		l_pEraserMemory = l_pFileMemory;
		while (*l_pTemp != 0x0A)
			++l_pTemp;
		*l_pTemp = '\0';
		++l_pTemp;
		iShotOne = atoi(l_pFileMemory);
		l_pFileMemory = l_pTemp;


		//----------------------------------------------------------------------------------------------------------------------
		// �Ѿ� ���� ����
		// 
		// 
		// ���: �ѹ��� �����ϴ� �Ѿ� ����
		// 
		// �Ѿ� ���		������			y�����ǥ			x�����ǥ			�Ѿ� ��Ÿ��		�Ѿ� �ӵ�(�� �����ӿ� 1ĭ)		
		//----------------------------------------------------------------------------------------------------------------------
		ld_ReadContent(l_pFileMemory, iShotOne, 6, "%C %D %D %D %D %D", cBulletSkin, iDamage, iBulletDireY, iBulletDireX, iBulletCoolTime, iBulletSpeed);
		free(l_pEraserMemory);

		//---------------------------------------------------------------------------------
		// �� �������� �����Ѵ�.
		// 
		//---------------------------------------------------------------------------------
		g_stEnemy[iCnt].iShotOne = iShotOne;
		memcpy_s(g_stEnemy[iCnt].cBulletSkin, sizeof(g_stEnemy[iCnt].cBulletSkin), cBulletSkin, sizeof(cBulletSkin));
		memcpy_s(g_stEnemy[iCnt].iDamage, sizeof(g_stEnemy[iCnt].iDamage), iDamage, sizeof(iDamage));
		memcpy_s(g_stEnemy[iCnt].iBulletDireY, sizeof(g_stEnemy[iCnt].iBulletDireY), iBulletDireY, sizeof(iBulletDireY));
		memcpy_s(g_stEnemy[iCnt].iBulletDireX, sizeof(g_stEnemy[iCnt].iBulletDireX), iBulletDireX, sizeof(iBulletDireX));
		memcpy_s(g_stEnemy[iCnt].iBulletCoolTime, sizeof(g_stEnemy[iCnt].iBulletCoolTime), iBulletCoolTime, sizeof(iBulletCoolTime));
		memcpy_s(g_stEnemy[iCnt].iBulletSpeed, sizeof(g_stEnemy[iCnt].iBulletSpeed), iBulletSpeed, sizeof(iBulletSpeed));
	}
	free(pEraserMemory);
}


// ----------------------------------------------------
// ���� �̵�
// 
// ----------------------------------------------------
void enmy_Move_Enemy(void)
{
	int iCnt;

	for (iCnt = 0; iCnt < g_MaxEnemy; ++iCnt)
	{
		// ���� ���� �Ѿ��.
		if (g_stEnemy[iCnt].bVisible == false)
			continue;

		// ������ �������� �ƴ϶�� �Ѿ��.
		if (g_CntFps % g_stEnemy[iCnt].iMovingCoolTime[g_stEnemy[iCnt].iNextPattern] != 0)
			continue;

		// �����̱� ���� ���� ��ġ �����Ѵ�.
		g_stEnemy[iCnt].iBeforeY = g_stEnemy[iCnt].iY;
		g_stEnemy[iCnt].iBeforeX = g_stEnemy[iCnt].iX;

		// �� ������
		g_stEnemy[iCnt].iY += g_stEnemy[iCnt].iDireY[g_stEnemy[iCnt].iNextPattern];
		g_stEnemy[iCnt].iX += g_stEnemy[iCnt].iDireX[g_stEnemy[iCnt].iNextPattern];



		//-------------------------------------------------------------
		// �� �̵� �ݰ� ����.
		// ���� ȭ�鿡�� ���� �̵� ������ ������ �����Ѵ�.
		//-------------------------------------------------------------
		g_stEnemy[iCnt].iX = max(g_stEnemy[iCnt].iX, 0);
		g_stEnemy[iCnt].iX = min(g_stEnemy[iCnt].iX, 79);
		g_stEnemy[iCnt].iY = max(g_stEnemy[iCnt].iY, 0);
		g_stEnemy[iCnt].iY = min(g_stEnemy[iCnt].iY, 23);


		// ���� ������ ����Ų��.
		g_stEnemy[iCnt].iNextPattern = (g_stEnemy[iCnt].iNextPattern + 1) % g_stEnemy[iCnt].iMaxMovingPattern;
	}
}

//---------------------------------------------------------------------------------
// ���� �浿����
// 
//---------------------------------------------------------------------------------
bool enmy_Is_Hit(int iY, int iX, int iDamage)
{
	int iCnt;
	double dM;
	
	for (iCnt = 0; iCnt < g_MaxEnemy; ++iCnt)
	{
		if (g_stEnemy[iCnt].bVisible == false)
			continue;

		// �Ѿ��� ��ǥ�� ���� �̵��ϱ� ���� �� ��ǥ ���̿� �ִ��� Ȯ���Ѵ�. 
		if (g_stEnemy[iCnt].iBeforeY <= g_stEnemy[iCnt].iY)
		{
			if (iY < g_stEnemy[iCnt].iBeforeY || g_stEnemy[iCnt].iY < iY)
				continue;
		}
		else
		{
			if (iY < g_stEnemy[iCnt].iY || g_stEnemy[iCnt].iBeforeY < iY)
				continue;
		}
		if (g_stEnemy[iCnt].iBeforeX <= g_stEnemy[iCnt].iX)
		{
			if (iX < g_stEnemy[iCnt].iBeforeX || g_stEnemy[iCnt].iX < iX)
				continue;
		}
		else
		{
			if (iX < g_stEnemy[iCnt].iX || g_stEnemy[iCnt].iBeforeX < iX)
				continue;
		}


		// ���� �̵��� ��ǥ�� ���� 2�� ������ 
		// �����Ŀ� �Ѿ��� ��ǥ�� �־� �浹�� Ȯ���Ѵ�.
		dM = fabs((double)(g_stEnemy[iCnt].iY - g_stEnemy[iCnt].iBeforeY)) / fabs((double)(g_stEnemy[iCnt].iX - g_stEnemy[iCnt].iBeforeX));
		// 0�ΰ�� �浹�ߴ�
		if ((dM * iX + (g_stEnemy[iCnt].iY - (dM * g_stEnemy[iCnt].iX)) - iY) == 0)
		{
			g_stEnemy[iCnt].iHp -= iDamage;

			// �ǰ� ������ ���� �����.
			if (g_stEnemy[iCnt].iHp == 0)
			{
				g_EnemyCount--;
				g_stEnemy[iCnt].bVisible = false;
			}
			return true;
		}

	}

	return false;
}

//---------------------------------------------------------------------------------
// ��ũ�� ���ۿ� ���� �׸���.
// 
//---------------------------------------------------------------------------------
void enmy_Draw_Enemy(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < g_MaxEnemy; ++iCnt)
	{
		// ���� ���� �Ѿ��.
		if (g_stEnemy[iCnt].bVisible == false)
			continue;

		buff_Sprite_Draw(g_stEnemy[iCnt].iY, g_stEnemy[iCnt].iX, g_stEnemy[iCnt].cEnemySkin);
	}
}

bool enmy_Is_Win(void)
{
	if (g_EnemyCount == 0)
		return true;
	return false;
}











///////////////////////////////////////////////////////
// �Ѿ�
// 
///////////////////////////////////////////////////////
//---------------------------------------------------------------------------------
// �Ѿ� �߽�
// 
//---------------------------------------------------------------------------------
void enmy_Shoot_Bullet(void)
{
	int iCnt;
	int i;
	int j;

	//-----------------------------------------------------------
	// �� ������ ���� �߽� �� �� �ִ��� Ž���Ѵ�
	// 
	//-----------------------------------------------------------
	for (iCnt = 0; iCnt < g_MaxEnemy; ++iCnt)
	{
		// ���� ���� �Ѿ��.
		if (g_stEnemy[iCnt].bVisible == false)
			continue;

		//-----------------------------------------------------------
		// iShotOne �ִ� 5
		// �ѹ��� ������ �Ѿ��� ������
		// 
		//-----------------------------------------------------------
		for (i = 0; i < g_stEnemy[iCnt].iShotOne; ++i)
		{
			// �Ѿ��� ��Ÿ���� ���� ���� �ʾҴٸ� �Ѿ��.
			if (g_CntFps % g_stEnemy[iCnt].iBulletCoolTime[i] != 0)
				continue;

			// ����ִ� �Ѿ� �迭�� ã�´�.
			for (j = 0; j < MAX_ENEMY_BULLET; ++j)
			{

				// �Ѿ��� �Ҵ�Ǿ� �ִٸ� �Ѿ��.
				if (g_stEnemyBullet[j].bVisible == true)
					continue;


				//------------------------------------------
				// �Ѿ� �迭�� �Ѿ��� �Ҵ��Ѵ�.
				//------------------------------------------
				g_stEnemyBullet[j].bVisible = true;

				g_stEnemyBullet[j].cBulletSkin = g_stEnemy[iCnt].cBulletSkin[i];
				g_stEnemyBullet[j].iDamage = g_stEnemy[iCnt].iDamage[i];
				g_stEnemyBullet[j].iY = g_stEnemy[iCnt].iY;
				g_stEnemyBullet[j].iX = g_stEnemy[iCnt].iX;
				g_stEnemyBullet[j].iDireY = g_stEnemy[iCnt].iBulletDireY[i];
				g_stEnemyBullet[j].iDireX = g_stEnemy[iCnt].iBulletDireX[i];
				g_stEnemyBullet[j].iSpeed = g_stEnemy[iCnt].iBulletSpeed[i];
				break;
			}

		}
	}

}

//---------------------------------------------------------------------------------
// �Ѿ� �̵�
// 
//---------------------------------------------------------------------------------
void enmy_Move_Bullet(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < MAX_ENEMY_BULLET; ++iCnt)
	{
		// �Ҵ� �ȵǾ� ������ �Ѿ��.
		if (g_stEnemyBullet[iCnt].bVisible == false)
			continue;

		// �̵��� �������� �ƴϸ� �Ѿ��.
		if (g_CntFps % g_stEnemyBullet[iCnt].iSpeed != 0)
			continue;

		g_stEnemyBullet[iCnt].iY += g_stEnemyBullet[iCnt].iDireY;
		g_stEnemyBullet[iCnt].iX += g_stEnemyBullet[iCnt].iDireX;


		if (g_stEnemyBullet[iCnt].iY < 0 || g_stEnemyBullet[iCnt].iX < 0 || g_stEnemyBullet[iCnt].iY >= dfSCREEN_HEIGHT || g_stEnemyBullet[iCnt].iX >= dfSCREEN_WIDTH - 1)
		{
			g_stEnemyBullet[iCnt].bVisible = false;
		}
	}
}

//---------------------------------------------------------------------------------
// �� �Ѿ˰� �÷��̾�� �浹 Ȯ��
// 
//---------------------------------------------------------------------------------
void enmy_Bullet_Hit_Player(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < MAX_ENEMY_BULLET; ++iCnt)
	{
		if (g_stEnemyBullet[iCnt].bVisible == false)
			continue;

		if (play_Is_Hit(g_stEnemyBullet[iCnt].iY, g_stEnemyBullet[iCnt].iX, g_stEnemyBullet[iCnt].iDamage))
		{
			g_stEnemyBullet[iCnt].bVisible = false;
			break;
		}
	}
}

//---------------------------------------------------------------------------------
// ��ũ�� ���ۿ� �Ѿ��� �׸���.
// 
//---------------------------------------------------------------------------------
void enmy_Draw_Enemy_Bullet(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < MAX_ENEMY_BULLET; ++iCnt)
	{
		// ��� ���ϴ� �Ѿ��� �Ѿ��.
		if (g_stEnemyBullet[iCnt].bVisible == false)
			continue;

		buff_Sprite_Draw(g_stEnemyBullet[iCnt].iY, g_stEnemyBullet[iCnt].iX, g_stEnemyBullet[iCnt].cBulletSkin);
	}
}

