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

int g_MaxEnemy;				// 현재 스테이지 최대 적의 수
int g_EnemyCount;			// 현재 남아있는 적의 수




///////////////////////////////////////////////////////
// 적 
// 
///////////////////////////////////////////////////////
// ----------------------------------------------------
// 적 정보를 불러온다.
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
	int iDireY[MAX_MOVING_PATTERN];				// 움직임 상대 좌표
	int iDireX[MAX_MOVING_PATTERN];				//
	int iMovingCoolTime[MAX_MOVING_PATTERN];		// 움직임에 대한 쿨타임


	char* pBulletFileText = NULL;
	int iShotOne;
	char cBulletSkin[MAX_SHOT_ONE];
	int iDamage[MAX_SHOT_ONE];
	int iBulletDireY[MAX_SHOT_ONE];
	int iBulletDireX[MAX_SHOT_ONE];
	int iBulletCoolTime[MAX_SHOT_ONE];
	int iBulletSpeed[MAX_SHOT_ONE];



	// 스테이지 파일 오픈
	err = fopen_s(&pFile, cEnemyInfo, "r");
	if (err != NULL)
	{
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "스테이지 파일 열기 실패");
		g_Scene = GAME_ERROR;
		return false;
	}
	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	// 스테이지 데이터 메모리에 저장
	pFileMemory = (char*)malloc(lFileSize);
	if (pFileMemory == NULL)
	{
		buff_Sprite_String(dfSCREEN_HEIGHT / 2, "메모리 할당 실패");
		g_Scene = GAME_ERROR;
		return false;
	}
	fread_s(pFileMemory, lFileSize, 1, lFileSize, pFile);
	fclose(pFile);


	// 헤더에서 적 유닛의 수를 구한다. 
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
		// 적 유닛들의 정보를 가져와 배열에 저장한다.
		// 
		// 파일 구조
		// 
		// 헤더: 적 유닛의 수
		// 
		// 적skin		HP		y좌표			x좌표			움직임.txt		총알.txt
		//---------------------------------------------------------------------------------
		ld_ReadContent(pFileMemory, 1, 6, "%c %d %d %d %s %s", &cEnemySkin, &iHP, &iY, &iX, &pMovingFileText, &pBulletFileText);

		// 적 배열에 정보를 읽은 정보를 저장한다.
		g_stEnemy[iCnt].bVisible = true;
		g_stEnemy[iCnt].cEnemySkin = cEnemySkin;
		g_stEnemy[iCnt].iHp = iHP;
		g_stEnemy[iCnt].iY = iY;
		g_stEnemy[iCnt].iX = iX;





		//---------------------------------------------------------------------------------
		// moving파일에서 적 움직임을 불러온다.
		// 
		//---------------------------------------------------------------------------------
		if (pMovingFileText == NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "적 움직임 파일 열기 실패1");
			g_Scene = GAME_ERROR;
			return false;
		}
		l_err = fopen_s(&l_pFile, pMovingFileText, "r");
		if(l_err != NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "적 움직임 파일 열기 실패2");
			g_Scene = GAME_ERROR;
			return false;
		}
		fseek(l_pFile, 0, SEEK_END);
		l_lFileSize = ftell(l_pFile);
		fseek(l_pFile, 0, SEEK_SET);
		l_pFileMemory = (char*)malloc(l_lFileSize);
		if (pFileMemory == NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "메모리 할당 실패");
			g_Scene = GAME_ERROR;
			return false;
		}
		fread_s(l_pFileMemory, l_lFileSize, 1, l_lFileSize, l_pFile);
		fclose(l_pFile);

		// 헤더에서 적 유닛의 수를 구한다. 
		l_pTemp = l_pFileMemory;
		l_pEraserMemory = l_pFileMemory;
		while (*l_pTemp != 0x0A)
			++l_pTemp;
		*l_pTemp = '\0';
		++l_pTemp;
		iMaxMoving = atoi(l_pFileMemory);
		l_pFileMemory = l_pTemp;

		//---------------------------------------------------------------------------------
		// 움직임 파일 구조
		// 
		// 
		// 헤더: 움직임 갯수
		// 
		// 이동에 대한 쿨타임			y상대좌표			x상대좌표
		//---------------------------------------------------------------------------------
		ld_ReadContent(l_pFileMemory, iMaxMoving, 3, "%D %D %D", iMovingCoolTime, iDireY, iDireX);
		free(l_pEraserMemory);

		//---------------------------------------------------------------------------------
		// 적 움직임을 저장한다.
		// 
		//---------------------------------------------------------------------------------
		g_stEnemy[iCnt].iMaxMovingPattern = iMaxMoving;
		memcpy_s(g_stEnemy[iCnt].iMovingCoolTime, sizeof(g_stEnemy[iCnt].iMovingCoolTime), iMovingCoolTime, sizeof(iMovingCoolTime));
		memcpy_s(g_stEnemy[iCnt].iDireY, sizeof(g_stEnemy[iCnt].iDireY), iDireY, sizeof(iDireY));
		memcpy_s(g_stEnemy[iCnt].iDireX, sizeof(g_stEnemy[iCnt].iDireX), iDireX, sizeof(iDireX));

		






		//---------------------------------------------------------------------------------
		// bullet 파일에서 총알을 불러온다.
		// 
		//---------------------------------------------------------------------------------
		if (pBulletFileText == NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "총알 파일 열기 실패1");
			g_Scene = GAME_ERROR;
			return false;
		}
		l_err = fopen_s(&l_pFile, pBulletFileText, "r");
		if(l_err != NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "총알 파일 열기 실패2");
			g_Scene = GAME_ERROR;
			return false;
		}
		fseek(l_pFile, 0, SEEK_END);
		l_lFileSize = ftell(l_pFile);
		fseek(l_pFile, 0, SEEK_SET);
		l_pFileMemory = (char*)malloc(l_lFileSize);
		if (l_pFileMemory == NULL)
		{
			buff_Sprite_String(dfSCREEN_HEIGHT / 2, "메모리 할당 실패");
			g_Scene = GAME_ERROR;
			return false;
		}
		fread_s(l_pFileMemory, l_lFileSize, 1, l_lFileSize, l_pFile);
		fclose(l_pFile);

		// 헤더에서 한번에 발싸하는 총알의 갯수
		l_pTemp = l_pFileMemory;
		l_pEraserMemory = l_pFileMemory;
		while (*l_pTemp != 0x0A)
			++l_pTemp;
		*l_pTemp = '\0';
		++l_pTemp;
		iShotOne = atoi(l_pFileMemory);
		l_pFileMemory = l_pTemp;


		//----------------------------------------------------------------------------------------------------------------------
		// 총알 파일 구조
		// 
		// 
		// 헤더: 한번에 생성하는 총알 갯수
		// 
		// 총알 모양		데미지			y상대좌표			x상대좌표			총알 쿨타임		총알 속도(몇 프레임에 1칸)		
		//----------------------------------------------------------------------------------------------------------------------
		ld_ReadContent(l_pFileMemory, iShotOne, 6, "%C %D %D %D %D %D", cBulletSkin, iDamage, iBulletDireY, iBulletDireX, iBulletCoolTime, iBulletSpeed);
		free(l_pEraserMemory);

		//---------------------------------------------------------------------------------
		// 적 움직임을 저장한다.
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
// 적의 이동
// 
// ----------------------------------------------------
void enmy_Move_Enemy(void)
{
	int iCnt;

	for (iCnt = 0; iCnt < g_MaxEnemy; ++iCnt)
	{
		// 죽은 적은 넘어간다.
		if (g_stEnemy[iCnt].bVisible == false)
			continue;

		// 움직일 프레임이 아니라면 넘어간다.
		if (g_CntFps % g_stEnemy[iCnt].iMovingCoolTime[g_stEnemy[iCnt].iNextPattern] != 0)
			continue;

		// 움직이기 전에 기존 위치 저장한다.
		g_stEnemy[iCnt].iBeforeY = g_stEnemy[iCnt].iY;
		g_stEnemy[iCnt].iBeforeX = g_stEnemy[iCnt].iX;

		// 적 움직임
		g_stEnemy[iCnt].iY += g_stEnemy[iCnt].iDireY[g_stEnemy[iCnt].iNextPattern];
		g_stEnemy[iCnt].iX += g_stEnemy[iCnt].iDireX[g_stEnemy[iCnt].iNextPattern];



		//-------------------------------------------------------------
		// 적 이동 반경 제한.
		// 게임 화면에서 적이 이동 가능한 구역을 제한한다.
		//-------------------------------------------------------------
		g_stEnemy[iCnt].iX = max(g_stEnemy[iCnt].iX, 0);
		g_stEnemy[iCnt].iX = min(g_stEnemy[iCnt].iX, 79);
		g_stEnemy[iCnt].iY = max(g_stEnemy[iCnt].iY, 0);
		g_stEnemy[iCnt].iY = min(g_stEnemy[iCnt].iY, 23);


		// 다음 패턴을 가르킨다.
		g_stEnemy[iCnt].iNextPattern = (g_stEnemy[iCnt].iNextPattern + 1) % g_stEnemy[iCnt].iMaxMovingPattern;
	}
}

//---------------------------------------------------------------------------------
// 적과 충동판정
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

		// 총알의 좌표가 적이 이동하기 전과 후 좌표 사이에 있는지 확인한다. 
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


		// 적이 이동한 좌표에 대한 2차 방정식 
		// 방정식에 총알의 좌표를 넣어 충돌을 확인한다.
		dM = fabs((double)(g_stEnemy[iCnt].iY - g_stEnemy[iCnt].iBeforeY)) / fabs((double)(g_stEnemy[iCnt].iX - g_stEnemy[iCnt].iBeforeX));
		// 0인경우 충돌했다
		if ((dM * iX + (g_stEnemy[iCnt].iY - (dM * g_stEnemy[iCnt].iX)) - iY) == 0)
		{
			g_stEnemy[iCnt].iHp -= iDamage;

			// 피가 없어진 적은 지운다.
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
// 스크린 버퍼에 적을 그린다.
// 
//---------------------------------------------------------------------------------
void enmy_Draw_Enemy(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < g_MaxEnemy; ++iCnt)
	{
		// 죽은 적은 넘어간다.
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
// 총알
// 
///////////////////////////////////////////////////////
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
	for (iCnt = 0; iCnt < g_MaxEnemy; ++iCnt)
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
			if (g_CntFps % g_stEnemy[iCnt].iBulletCoolTime[i] != 0)
				continue;

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

		// 이동할 프레임이 아니면 넘어간다.
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

		if (play_Is_Hit(g_stEnemyBullet[iCnt].iY, g_stEnemyBullet[iCnt].iX, g_stEnemyBullet[iCnt].iDamage))
		{
			g_stEnemyBullet[iCnt].bVisible = false;
			break;
		}
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

		buff_Sprite_Draw(g_stEnemyBullet[iCnt].iY, g_stEnemyBullet[iCnt].iX, g_stEnemyBullet[iCnt].cBulletSkin);
	}
}

