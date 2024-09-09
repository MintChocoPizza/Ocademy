
#include <stdio.h>
#include <Windows.h>
#include "MyNew.h"
#include "CParsing_ANSI.h"

#include "CList.h"
#include "CBaseObject.h"
#include "CollisionObject.h"
#include "CObjectManager.h"
#include "CEnemy.h"
#include "CPlayer.h"

#include "CSceneBase.h"
#include "CSceneManager.h"
#include "CSceneLoad.h"





CSceneLoad::CSceneLoad(int iGameStage, int iMaxStage) :_stageName(""), _iGameStage(iGameStage), _iMaxStage(iMaxStage)
{
	if (iGameStage <= iMaxStage)
	{
		sprintf_s(_stageName, "GameFile\\stage%d.txt", iGameStage);

		LoadStage(_stageName);
	}
}

CSceneLoad::~CSceneLoad()
{
}

bool CSceneLoad::Update(void)
{
	if (_iGameStage <= _iMaxStage)
		CSceneManager::GetInstance()->LoadScene(CSceneManager::GAME);
	else
		CSceneManager::GetInstance()->LoadScene(CSceneManager::CLEAR);

	return true;
}

void CSceneLoad::LoadStage(char* cStageName)
{
	int iCnt;
	int iMaxEnemy;

	char cPlayerFile[255] = "";

	char cSkin = NULL;
	int iHp = NULL;
	int iY = NULL;
	int iX = NULL;
	char cMoveFile[255] = "";
	char cGunFile[255] = "";

	char cString[255] = "";


	CParsing_ANSI CParsing;
	CParsing.LoadFile(cStageName);

	CParsing.GetValue("Player", cPlayerFile, 255);
	CPlayer* Player = new CPlayer(1, true, cPlayerFile);
	CObjectManager::GetInstance()->CreateObject(Player);
	

	CParsing.GetValue("MaxEnemy", &iMaxEnemy);
	// stageN.txt 파일을 읽는다.
	for (iCnt = 1; iCnt <= iMaxEnemy; ++iCnt)
	{
		sprintf_s(cString, "EnemySkin%d", iCnt);
		CParsing.GetValue(cString, &cSkin);

		sprintf_s(cString, "EnemyHP%d", iCnt);
		CParsing.GetValue(cString, &iHp);


		sprintf_s(cString, "EnemyY%d", iCnt);
		CParsing.GetValue(cString, &iY);

		sprintf_s(cString, "EnemyX%d", iCnt);
		CParsing.GetValue(cString, &iX);

		sprintf_s(cString, "EnemyMove%d", iCnt);
		CParsing.GetValue(cString, cMoveFile, 255);
		
		sprintf_s(cString, "EnemyGun%d", iCnt);
		CParsing.GetValue(cString, cGunFile, 255);


		// 1번 구현
		CEnemy* Enemy = new CEnemy(2, true, cSkin, iHp, iY, iX, cMoveFile, cGunFile);
		
		// 2번 구현
		CObjectManager::GetInstance()->CreateObject(Enemy);
	}

	
	
}
