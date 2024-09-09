#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "Enemy.h"
#include "Buffer.h"
#include "Player.h"


st_ENEMY g_stEnemy[MAX_ENEMY];
st_ENEMY_BULLET g_stEnemyBullet[MAX_ENEMY_BULLET];

int g_iEnemyCount;		// 현재 스테이지 최대 적의 수
int g_iCurEnemyNum;		// 현재 남아있는 적의 수
extern float g_ElapsedTime;


//---------------------------------------------------------------------------------
// 적 정보를 초기화 한다.
// 
//---------------------------------------------------------------------------------
void enmy_Initial(void)
{
	memset(g_stEnemy, 0, sizeof(g_stEnemy));
}



//---------------------------------------------------------------------------------
// 적 정보를 불러온다.
// 이 함수의 하는 일이 너무 많음
// 하지만 자세히 보면 파일을 열어서 정보를 꺼내오는 작업만 있음
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
	int iDireY[MAX_MOVING_PATTERN];				// 움직임 상대 좌표
	int iDireX[MAX_MOVING_PATTERN];				//
	float fMovingCoolTime[MAX_MOVING_PATTERN];		// 움직임에 대한 쿨타임
	float fMovingSpeed[MAX_MOVING_PATTERN];		// 움직이는 속도


	char* pBulletFileText;
	int iShotOne;
	char cBulletSkin[MAX_SHOT_ONE];
	int iDamage[MAX_SHOT_ONE];
	int iBulletDireY[MAX_SHOT_ONE];
	int iBulletDireX[MAX_SHOT_ONE];
	float fBulletCoolTime[MAX_SHOT_ONE];
	float fBulletSpeed[MAX_SHOT_ONE];



	// 파일 오픈
	err = fopen_s(&pFile, cEnemyInfo, "r");
	if (err != NULL)
		return false;
	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	// 데이터 메모리에 저장
	pFileMemory = (char*)malloc(lFileSize);
	pEraserMemory = pFileMemory;
	if (pFileMemory == NULL)
		return false;
	fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
	fclose(pFile);


	// 헤더에서 적 유닛의 수를 구한다.
	pTemp = pFileMemory;
	while (*pTemp != 0x0A)
		++pTemp;
	*pTemp = '\0';
	++pTemp;
	g_iEnemyCount = atoi(pFileMemory);
	g_iCurEnemyNum = g_iEnemyCount;

	//---------------------------------------------------------------------------------
	// 적 유닛들의 정보를 가져와 배열에 저장한다.
	// 
	// 파일 구조
	// 
	// 헤더: 적 유닛의 수
	// 
	// 적skin		HP		y좌표			x좌표			움직임.txt		총알.txt
	//---------------------------------------------------------------------------------
	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		// 적 모양
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		cEnemySkin = *pFileMemory;

		// 적 HP
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iHP = atoi(pFileMemory);

		// y좌표
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fY = (float)atof(pFileMemory);


		// x좌표
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fX = (float)atof(pFileMemory);

		// moving 파일에서 움직임을 가져온다.
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		pMovingFileText = pFileMemory;
		if (!enmy_Is_Get_Moving_Info(pMovingFileText, &iMovingPatternCnt, fMovingCoolTime, iDireY, iDireX, fMovingSpeed))
			return false;

		// bullet 파일에서 총알을 가져온다.
		pFileMemory = pTemp;
		while (*pTemp != 0x0A)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		pBulletFileText = pFileMemory;
		if (!enmy_Is_Get_Bullet_Info(pBulletFileText, &iShotOne, cBulletSkin, iDamage, iBulletDireY, iBulletDireX,  fBulletCoolTime, fBulletSpeed))
			return false;



		// 적 배열에 정보를 읽은 정보를 저장한다.
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



		
		// 총알 정보를 저장한다.
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
// 적 움직임을 불러온다.
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
	// 움직임 파일 구조
	// 
	// 
	// 헤더: 움직임 갯수
	// 
	// 이동에 대한 쿨타임		움직임 속도			y상대좌표			x상대좌표
	//---------------------------------------------------------------------------------
	pEraserMemory = pFileMemory;
	pTemp = pFileMemory;

	// 움직임 갯수
	while (*pTemp != 0x0A)
		++pTemp;
	*pTemp = '\0';
	++pTemp;
	*iMovingPatternCnt = atoi(pFileMemory);

	
	for (iCnt = 0; iCnt < *iMovingPatternCnt; ++iCnt)
	{
		// 이동에 대한 쿨타임
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fMovingCoolTime[iCnt] = (float)atof(pFileMemory);


		// 움직임 속도
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fMovingSpeed[iCnt] = (float)atof(pFileMemory);


		// y상대좌표
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iDireY[iCnt] = atoi(pFileMemory);


		// x상대좌표
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
// 총알 정보를 불러온다.
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
	// 총알 파일 구조
	// 
	// 
	// 헤더: 한번에 생성하는 총알 갯수
	// 
	// 총알 모양		데미지			y상대좌표			x상대좌표			총알 쿨타임				총알 속도
	//---------------------------------------------------------------------------------
	pEraserMemory = pFileMemory;
	pTemp = pFileMemory;

	// 총알 갯수
	while (*pTemp != 0x0A)
		++pTemp;
	*pTemp = '\0';
	++pTemp;
	*iShotOne = atoi(pFileMemory);

	for (iCnt = 0; iCnt < *iShotOne; ++iCnt)
	{
		// 총알 모양
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		cBulletSkin[iCnt] = *pFileMemory;

		// 데미지
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iDamage[iCnt] = atoi(pFileMemory);

		// y상대좌표
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iDireY[iCnt] = atoi(pFileMemory);

		// x상대좌표
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		iDireX[iCnt] = atoi(pFileMemory);

		// 총알 쿨타임
		pFileMemory = pTemp;
		while (*pTemp != 0x20)
			++pTemp;
		*pTemp = '\0';
		++pTemp;
		fBulletCoolTime[iCnt] = atof(pFileMemory);

		// 총알 속도
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
// 적 이동
// 
//---------------------------------------------------------------------------------
void enmy_Move_Enemy(void)
{
	int iCnt;


	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		// 죽은 적은 넘어간다.
		if (g_stEnemy[iCnt].bVisible == false)
			continue;

		// 다음 행동에 대한 쿨타임이 아직 돌지 않았다면 넘어간다.
		if (g_stEnemy[iCnt].fMovingCoolTimeCounter < g_stEnemy[iCnt].fMovingCoolTime[g_stEnemy[iCnt].iNextPattern])
			continue;


		// 쿨타임 초기화
		g_stEnemy[iCnt].fMovingCoolTimeCounter = 0.0f;

		
		// 생각과 다르게 소수점 반올림으로 인하여 움직임을 예측할 수 없게됨
		//// 다음 패턴 * 움직이는 속도 * 흐른 시간
		//g_stEnemy[iCnt].fY += g_stEnemy[iCnt].iDireY[g_stEnemy[iCnt].iNextPattern] * g_stEnemy[iCnt].fMovingSpeed[g_stEnemy[iCnt].iNextPattern] * g_ElapsedTime;
		//g_stEnemy[iCnt].fX += g_stEnemy[iCnt].iDireX[g_stEnemy[iCnt].iNextPattern] * g_stEnemy[iCnt].fMovingSpeed[g_stEnemy[iCnt].iNextPattern] * g_ElapsedTime;
		g_stEnemy[iCnt].fY += g_stEnemy[iCnt].iDireY[g_stEnemy[iCnt].iNextPattern];
		g_stEnemy[iCnt].fX += g_stEnemy[iCnt].iDireX[g_stEnemy[iCnt].iNextPattern];

		
		//-------------------------------------------------------------
		// 적 이동 반경 제한.
		// 게임 화면에서 적이 이동 가능한 구역을 제한한다.
		//-------------------------------------------------------------
		g_stEnemy[iCnt].fX = max(g_stEnemy[iCnt].fX, 0.00f);
		g_stEnemy[iCnt].fX = min(g_stEnemy[iCnt].fX, 79.00f);
		g_stEnemy[iCnt].fY = max(g_stEnemy[iCnt].fY, 0.00f);
		g_stEnemy[iCnt].fY = min(g_stEnemy[iCnt].fY, 23.00f);


		// 다음 패턴을 가르킨다.
		g_stEnemy[iCnt].iNextPattern = (g_stEnemy[iCnt].iNextPattern + 1) % g_stEnemy[iCnt].iMovingPatternCnt;
	}
}




//---------------------------------------------------------------------------------
// 총알 발싸
// 
//---------------------------------------------------------------------------------
void enmy_Shoot_Bullet(void)
{
	int iCnt;
	int i;
	int j;

	//-----------------------------------------------------------
	// 적 유닛이 총을 발싸 할 수 있는지 탐색한다
	// 
	//-----------------------------------------------------------
	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		// 죽은 적은 넘어간다.
		if (g_stEnemy[iCnt].bVisible == false)
			continue;


		
		//-----------------------------------------------------------
		// iShotOne 최대 5
		// 한번에 나가는 총알을 저장함
		// 
		//-----------------------------------------------------------
		for (i = 0; i < g_stEnemy[iCnt].iShotOne; ++i)
		{
			// 총알의 쿨타임이 아직 돌지 않았다면 넘어간다.
			if (g_stEnemy[iCnt].fCoolTimeCounter[i] < g_stEnemy[iCnt].fBulletCoolTime[i])
				continue;

			g_stEnemy[iCnt].fCoolTimeCounter[i] = 0.0f;
			

			// 비어있는 총알 배열을 찾는다.
			for (j = 0; j < MAX_ENEMY_BULLET; ++j)
			{

				// 총알이 할당되어 있다면 넘어간다.
				if (g_stEnemyBullet[j].bVisible == true)
					continue;


				//------------------------------------------
				// 총알 배열에 총알을 할당한다.
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
// 총알 이동
// 
//---------------------------------------------------------------------------------
void enmy_Move_Bullet(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < MAX_ENEMY_BULLET; ++iCnt)
	{
		// 할당 안되어 있으면 넘어간다.
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
// 적 총알과 플레이어와 충돌 확인
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
// 적과 충동판정
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


			// 피가 없어진 적은 지운다.
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
// 적이 다 죽어서 이겼는지 확인한다.
// 
//---------------------------------------------------------------------------------
bool enmy_Is_Win(void)
{
	if (g_iCurEnemyNum == 0)
		return true;
	return false;
}




//---------------------------------------------------------------------------------
// 스크린 버퍼에 적을 그린다.
// 
//---------------------------------------------------------------------------------
void enmy_Draw_Enemy(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < g_iEnemyCount; ++iCnt)
	{
		// 죽은 적은 넘어간다.
		if (g_stEnemy[iCnt].bVisible == false)
			continue;

		buff_Sprite_Draw((int)g_stEnemy[iCnt].fY, (int)g_stEnemy[iCnt].fX, g_stEnemy[iCnt].cEnemySkin);
	}
}

//---------------------------------------------------------------------------------
// 스크린 버퍼에 총알을 그린다.
// 
//---------------------------------------------------------------------------------
void enmy_Draw_Enemy_Bullet(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < MAX_ENEMY_BULLET; ++iCnt)
	{
		// 사용 안하는 총알은 넘어간다.
		if (g_stEnemyBullet[iCnt].bVisible == false)
			continue;

		buff_Sprite_Draw((int)g_stEnemyBullet[iCnt].fY, (int)g_stEnemyBullet[iCnt].fX, g_stEnemyBullet[iCnt].cBulletSkin);
	}
}






//---------------------------------------------------------------------------------
// 쿨타임
// 
//---------------------------------------------------------------------------------
void enmy_CoolTime(void)
{
	enmy_Moving_CoolTime();
	enmy_Bullet_CoolTime();
}




//---------------------------------------------------------------------------------
// 움직임에 대한 쿨타임
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
// 총알에 대한 쿨타임
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

