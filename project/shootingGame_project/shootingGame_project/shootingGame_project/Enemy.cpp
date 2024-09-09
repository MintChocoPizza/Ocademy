#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "Enemy.h"
#include "Buffer.h"
#include "Player.h"


st_ENEMY g_stEnemy[MAX_ENEMY];
st_ENEMY_BULLET g_stEnemyBullet[MAX_ENEMY_BULLET];

int g_iEnemyCount;		// ���� �������� �ִ� ���� ��
int g_iCurEnemyNum;		// ���� �����ִ� ���� ��
extern float g_ElapsedTime;


//---------------------------------------------------------------------------------
// �� ������ �ʱ�ȭ �Ѵ�.
// 
//---------------------------------------------------------------------------------
void enmy_Initial(void)
{
	memset(g_stEnemy, 0, sizeof(g_stEnemy));
}



//---------------------------------------------------------------------------------
// �� ������ �ҷ��´�.
// �� �Լ��� �ϴ� ���� �ʹ� ����
// ������ �ڼ��� ���� ������ ��� ������ �������� �۾��� ����
// 
//---------------------------------------------------------------------------------
bool enmy_Is_Get_Info(char* cEnemyInfo)
{

	FILE* pFile;
	errno_t err;
	long lFileSize;
	char* pFileMemory;
	char* pEraserMemory;
	char* pTemp;

	int iCnt;
	char cEnemySkin;
	int iHP;
	float fY;
	float fX;
	
	
	char* pMovingFileText;
	int iMovingPatternCnt;
	int iDireY[MAX_MOVING_PATTERN];				// ������ ��� ��ǥ
	int iDireX[MAX_MOVING_PATTERN];				//
	float fMovingCoolTime[MAX_MOVING_PATTERN];		// �����ӿ� ���� ��Ÿ��
	float fMovingSpeed[MAX_MOVING_PATTERN];		// �����̴� �ӵ�


	char* pBulletFileText;
	int iShotOne;
	char cBulletSkin[MAX_SHOT_ONE];
	int iDamage[MAX_SHOT_ONE];
	int iBulletDireY[MAX_SHOT_ONE];
	int iBulletDireX[MAX_SHOT_ONE];
	float fBulletCoolTime[MAX_SHOT_ONE];
	float fBulletSpeed[MAX_SHOT_ONE];



	// ���� ����
	err = fopen_s(&pFile, cEnemyInfo, "r");
	if (err != NULL)
		return false;
	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	// ������ �޸𸮿� ����
	pFileMemory = (char*)malloc(lFileSize);
	pEraserMemory = pFileMemory;
	if (pFileMemory == NULL)
		return false;
	fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
	fclose(pFile);


	// ������� �� ������ ���� ���Ѵ�.
	pTemp = pFileMemory;
	while (*pTemp != 0x0A)
		++pTemp;
	*pTemp = '\0';
	++pTemp;
	g_iEnemyCount = atoi(pFileMemory);
	g_iCurEnemyNum = g_iEnemyCount;

	//---------------------------------------------------------------------------------
	// �� ���ֵ��� ������ ������ �迭�� �����Ѵ�.
	// 
	// ���� ����
	// 
	// ���: �� ������ ��
	// 
	// ��skin		HP		y��ǥ			x��ǥ			������.txt		�Ѿ�.txt
	//---------------------------------------------------------------------------------
	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		// �� ���
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		cEnemySkin = *pFileMemory;

		// �� HP
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iHP = atoi(pFileMemory);

		// y��ǥ
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fY = (float)atof(pFileMemory);


		// x��ǥ
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fX = (float)atof(pFileMemory);

		// moving ���Ͽ��� �������� �����´�.
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		pMovingFileText = pFileMemory;
		if (!enmy_Is_Get_Moving_Info(pMovingFileText, &iMovingPatternCnt, fMovingCoolTime, iDireY, iDireX, fMovingSpeed))
			return false;

		// bullet ���Ͽ��� �Ѿ��� �����´�.
		pFileMemory = pTemp;
		while (*pTemp != 0x0A)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		pBulletFileText = pFileMemory;
		if (!enmy_Is_Get_Bullet_Info(pBulletFileText, &iShotOne, cBulletSkin, iDamage, iBulletDireY, iBulletDireX,  fBulletCoolTime, fBulletSpeed))
			return false;



		// �� �迭�� ������ ���� ������ �����Ѵ�.
		g_stEnemy[iCnt].bVisible = true;
		g_stEnemy[iCnt].cEnemySkin = cEnemySkin;
		g_stEnemy[iCnt].iHp = iHP;
		g_stEnemy[iCnt].fY = fY;
		g_stEnemy[iCnt].fX = fX;

		g_stEnemy[iCnt].iMovingPatternCnt = iMovingPatternCnt;
		memcpy_s(g_stEnemy[iCnt].iDireY, sizeof(g_stEnemy[iCnt].iDireY), iDireY, sizeof(iDireY));
		memcpy_s(g_stEnemy[iCnt].iDireX, sizeof(g_stEnemy[iCnt].iDireX), iDireX, sizeof(iDireX));
		memcpy_s(g_stEnemy[iCnt].fMovingSpeed, sizeof(g_stEnemy[iCnt].fMovingSpeed), fMovingSpeed, sizeof(fMovingSpeed));
		memcpy_s(g_stEnemy[iCnt].fMovingCoolTime, sizeof(g_stEnemy[iCnt].fMovingCoolTime), fMovingCoolTime, sizeof(fMovingCoolTime));



		
		// �Ѿ� ������ �����Ѵ�.
		g_stEnemy[iCnt].iShotOne = iShotOne;


		memcpy_s(g_stEnemy[iCnt].cBulletSkin, sizeof(g_stEnemy[iCnt].cBulletSkin), cBulletSkin, sizeof(cBulletSkin));
		memcpy_s(g_stEnemy[iCnt].iDamage, sizeof(g_stEnemy[iCnt].iDamage), iDamage, sizeof(iDamage));
		memcpy_s(g_stEnemy[iCnt].fBulletCoolTime, sizeof(g_stEnemy[iCnt].fBulletCoolTime), fBulletCoolTime, sizeof(fBulletCoolTime));
		memcpy_s(g_stEnemy[iCnt].fBulletSpeed, sizeof(g_stEnemy[iCnt].fBulletSpeed), fBulletSpeed, sizeof(fBulletSpeed));

		memcpy_s(g_stEnemy[iCnt].iBulletDireY, sizeof(g_stEnemy[iCnt].iBulletDireY), iBulletDireY, sizeof(iBulletDireY));
		memcpy_s(g_stEnemy[iCnt].iBulletDireX, sizeof(g_stEnemy[iCnt].iBulletDireX), iBulletDireX, sizeof(iBulletDireX));


	}

	free(pEraserMemory);
	return true;
}





//---------------------------------------------------------------------------------
// �� �������� �ҷ��´�.
// 
//---------------------------------------------------------------------------------
bool enmy_Is_Get_Moving_Info(char* pMovingFileText, int* iMovingPatternCnt, float* fMovingCoolTime, int* iDireY, int* iDireX, float* fMovingSpeed)
{
	FILE* pFile;
	errno_t err;

	long lFileSize;
	char* pFileMemory;
	char* pEraserMemory;
	char* pTemp;

	int iCnt;

	err = fopen_s(&pFile, pMovingFileText, "r");
	if (err != NULL)
		return false;

	fseek(pFile, 0, SEEK_END);
 	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	pFileMemory = (char*)malloc(lFileSize);
	if (pFileMemory == NULL)
		return false;
	fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
	fclose(pFile);





	//---------------------------------------------------------------------------------
	// ������ ���� ����
	// 
	// 
	// ���: ������ ����
	// 
	// �̵��� ���� ��Ÿ��		������ �ӵ�			y�����ǥ			x�����ǥ
	//---------------------------------------------------------------------------------
	pEraserMemory = pFileMemory;
	pTemp = pFileMemory;

	// ������ ����
	while (*pTemp != 0x0A)
		++pTemp;
	*pTemp = '\0';
	++pTemp;
	*iMovingPatternCnt = atoi(pFileMemory);

	
	for (iCnt = 0; iCnt < *iMovingPatternCnt; ++iCnt)
	{
		// �̵��� ���� ��Ÿ��
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fMovingCoolTime[iCnt] = (float)atof(pFileMemory);


		// ������ �ӵ�
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fMovingSpeed[iCnt] = (float)atof(pFileMemory);


		// y�����ǥ
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iDireY[iCnt] = atoi(pFileMemory);


		// x�����ǥ
		pFileMemory = pTemp;
		while (*pTemp != 0x0A)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iDireX[iCnt] = atoi(pFileMemory);
	}


	free(pEraserMemory);

	return true;
}




//---------------------------------------------------------------------------------
// �Ѿ� ������ �ҷ��´�.
// 
//---------------------------------------------------------------------------------
bool enmy_Is_Get_Bullet_Info(char* pBulletFileText, int* iShotOne, char* cBulletSkin, int* iDamage, int* iDireY, int* iDireX, float* fBulletCoolTime, float* fBulletSpeed)
{
	FILE* pFile;
	errno_t err;
	char* pFileMemory;
	char* pEraserMemory;
	long lFileSize;
	char* pTemp;
	int iCnt;

	err = fopen_s(&pFile, pBulletFileText, "r");
	if (err != NULL)
		return false;

	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	pFileMemory = (char*)malloc(lFileSize);
	if (pFileMemory == NULL)
		return false;
	fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
	fclose(pFile);




	//---------------------------------------------------------------------------------
	// �Ѿ� ���� ����
	// 
	// 
	// ���: �ѹ��� �����ϴ� �Ѿ� ����
	// 
	// �Ѿ� ���		������			y�����ǥ			x�����ǥ			�Ѿ� ��Ÿ��				�Ѿ� �ӵ�
	//---------------------------------------------------------------------------------
	pEraserMemory = pFileMemory;
	pTemp = pFileMemory;

	// �Ѿ� ����
	while (*pTemp != 0x0A)
		++pTemp;
	*pTemp = '\0';
	++pTemp;
	*iShotOne = atoi(pFileMemory);

	for (iCnt = 0; iCnt < *iShotOne; ++iCnt)
	{
		// �Ѿ� ���
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		cBulletSkin[iCnt] = *pFileMemory;

		// ������
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iDamage[iCnt] = atoi(pFileMemory);

		// y�����ǥ
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iDireY[iCnt] = atoi(pFileMemory);

		// x�����ǥ
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iDireX[iCnt] = atoi(pFileMemory);

		// �Ѿ� ��Ÿ��
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fBulletCoolTime[iCnt] = atof(pFileMemory);

		// �Ѿ� �ӵ�
		pFileMemory = pTemp;
		while (*pTemp != 0x0A)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fBulletSpeed[iCnt] = atof(pFileMemory);
	}

	free(pEraserMemory);
	return true;
}


//---------------------------------------------------------------------------------
// �� �̵�
// 
//---------------------------------------------------------------------------------
void enmy_Move_Enemy(void)
{
	int iCnt;


	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		// ���� ���� �Ѿ��.
		if (g_stEnemy[iCnt].bVisible == false)
			continue;

		// ���� �ൿ�� ���� ��Ÿ���� ���� ���� �ʾҴٸ� �Ѿ��.
		if (g_stEnemy[iCnt].fMovingCoolTimeCounter < g_stEnemy[iCnt].fMovingCoolTime[g_stEnemy[iCnt].iNextPattern])
			continue;


		// ��Ÿ�� �ʱ�ȭ
		g_stEnemy[iCnt].fMovingCoolTimeCounter = 0.0f;

		
		// ������ �ٸ��� �Ҽ��� �ݿø����� ���Ͽ� �������� ������ �� ���Ե�
		//// ���� ���� * �����̴� �ӵ� * �帥 �ð�
		//g_stEnemy[iCnt].fY += g_stEnemy[iCnt].iDireY[g_stEnemy[iCnt].iNextPattern] * g_stEnemy[iCnt].fMovingSpeed[g_stEnemy[iCnt].iNextPattern] * g_ElapsedTime;
		//g_stEnemy[iCnt].fX += g_stEnemy[iCnt].iDireX[g_stEnemy[iCnt].iNextPattern] * g_stEnemy[iCnt].fMovingSpeed[g_stEnemy[iCnt].iNextPattern] * g_ElapsedTime;
		g_stEnemy[iCnt].fY += g_stEnemy[iCnt].iDireY[g_stEnemy[iCnt].iNextPattern];
		g_stEnemy[iCnt].fX += g_stEnemy[iCnt].iDireX[g_stEnemy[iCnt].iNextPattern];

		
		//-------------------------------------------------------------
		// �� �̵� �ݰ� ����.
		// ���� ȭ�鿡�� ���� �̵� ������ ������ �����Ѵ�.
		//-------------------------------------------------------------
		g_stEnemy[iCnt].fX = max(g_stEnemy[iCnt].fX, 0.00f);
		g_stEnemy[iCnt].fX = min(g_stEnemy[iCnt].fX, 79.00f);
		g_stEnemy[iCnt].fY = max(g_stEnemy[iCnt].fY, 0.00f);
		g_stEnemy[iCnt].fY = min(g_stEnemy[iCnt].fY, 23.00f);


		// ���� ������ ����Ų��.
		g_stEnemy[iCnt].iNextPattern = (g_stEnemy[iCnt].iNextPattern + 1) % g_stEnemy[iCnt].iMovingPatternCnt;
	}
}




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
	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
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
			if (g_stEnemy[iCnt].fCoolTimeCounter[i] < g_stEnemy[iCnt].fBulletCoolTime[i])
				continue;

			g_stEnemy[iCnt].fCoolTimeCounter[i] = 0.0f;
			

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

				g_stEnemyBullet[j].cBulletSkin		= g_stEnemy[iCnt].cBulletSkin[i];
				g_stEnemyBullet[j].iDamage			= g_stEnemy[iCnt].iDamage[i];
				g_stEnemyBullet[j].fY				= g_stEnemy[iCnt].fY;
				g_stEnemyBullet[j].fX				= g_stEnemy[iCnt].fX;
				g_stEnemyBullet[j].iDireY			= g_stEnemy[iCnt].iBulletDireY[i];
				g_stEnemyBullet[j].iDireX			= g_stEnemy[iCnt].iBulletDireX[i];
				g_stEnemyBullet[j].fBulletSpeed		= g_stEnemy[iCnt].fBulletSpeed[i];
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

		g_stEnemyBullet[iCnt].bVisible = true;
		g_stEnemyBullet[iCnt].fY += g_stEnemyBullet[iCnt].iDireY * g_stEnemyBullet[iCnt].fBulletSpeed * g_ElapsedTime;
		g_stEnemyBullet[iCnt].fX += g_stEnemyBullet[iCnt].iDireX * g_stEnemyBullet[iCnt].fBulletSpeed * g_ElapsedTime;


		if (g_stEnemyBullet[iCnt].fY < 0 || g_stEnemyBullet[iCnt].fX < 0 || g_stEnemyBullet[iCnt].fY >= dfSCREEN_HEIGHT || g_stEnemyBullet[iCnt].fX >= dfSCREEN_WIDTH - 1)
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

		if (play_Is_Hit(g_stEnemyBullet[iCnt].fY, g_stEnemyBullet[iCnt].fX, g_stEnemyBullet[iCnt].iDamage))
		{
			g_stEnemyBullet[iCnt].bVisible = false;
			break;
		}
	}
}


//---------------------------------------------------------------------------------
// ���� �浿����
// 
//---------------------------------------------------------------------------------
bool enmy_Is_Hit(int iY, int iX, int IDamage)
{
	int iCnt;
	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		if ((int)g_stEnemy[iCnt].fY == iY && (int)g_stEnemy[iCnt].fX == iX)
		{
			g_stEnemy[iCnt].iHp -= IDamage;


			// �ǰ� ������ ���� �����.
			if (g_stEnemy[iCnt].iHp == 0)
			{
				g_iCurEnemyNum--;
				g_stEnemy[iCnt].bVisible = false;
			}

			return true;
		}
	}

	return false;
}


//---------------------------------------------------------------------------------
// ���� �� �׾ �̰���� Ȯ���Ѵ�.
// 
//---------------------------------------------------------------------------------
bool enmy_Is_Win(void)
{
	if (g_iCurEnemyNum == 0)
		return true;
	return false;
}




//---------------------------------------------------------------------------------
// ��ũ�� ���ۿ� ���� �׸���.
// 
//---------------------------------------------------------------------------------
void enmy_Draw_Enemy(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		// ���� ���� �Ѿ��.
		if (g_stEnemy[iCnt].bVisible == false)
			continue;

		buff_Sprite_Draw((int)g_stEnemy[iCnt].fY, (int)g_stEnemy[iCnt].fX, g_stEnemy[iCnt].cEnemySkin);
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

		buff_Sprite_Draw((int)g_stEnemyBullet[iCnt].fY, (int)g_stEnemyBullet[iCnt].fX, g_stEnemyBullet[iCnt].cBulletSkin);
	}
}






//---------------------------------------------------------------------------------
// ��Ÿ��
// 
//---------------------------------------------------------------------------------
void enmy_CoolTime(void)
{
	enmy_Moving_CoolTime();
	enmy_Bullet_CoolTime();
}




//---------------------------------------------------------------------------------
// �����ӿ� ���� ��Ÿ��
// 
//---------------------------------------------------------------------------------
void enmy_Moving_CoolTime(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		g_stEnemy[iCnt].fMovingCoolTimeCounter += g_ElapsedTime;
	}
}



//---------------------------------------------------------------------------------
// �Ѿ˿� ���� ��Ÿ��
// 
//---------------------------------------------------------------------------------
void enmy_Bullet_CoolTime(void)
{
	int iCnt;
	int iCnt_two;
	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		for(iCnt_two=0; iCnt_two<g_stEnemy[iCnt].iShotOne; ++iCnt_two)
		{
			g_stEnemy[iCnt].fCoolTimeCounter[iCnt_two] += g_ElapsedTime;
		}
	}
}

