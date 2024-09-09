
#include <stdio.h>
#include <Windows.h>
#include "MyNew.h"
#include "CList.h"
#include "FPSManager.h"

#include "CParsing_ANSI.h"
#include "ConsoleBuffer.h"

#include "CBaseObject.h"
#include "CObjectManager.h"
#include "CollisionObject.h"
#include "CBullet.h"
#include "CEnemy.h"

// CEnemy::CEnemy(int ObjectType, bool Visible, char cSkin, int iHp, int iY, int iX, char *cMoveFile, char *GunFile) : CBaseObject(ObjectType, Visible), _Skin(cSkin), _Hp(iHp), _iY(iY), _iX(iX)
CEnemy::CEnemy(int ObjectType, bool Visible, char cSkin, int iHp, int iY, int iX, char *cMoveFile, char *GunFile) : 
	CollisionObject(ObjectType, Visible), _Skin(cSkin), _Hp(iHp)//, _iY(iY), _iX(iX)
{
	_iY = iY;
	_iX = iX;
	GetMoving(cMoveFile);
	iter = MovingList.begin();

	GetBullet(GunFile);
}

CEnemy::~CEnemy()
{
}

bool CEnemy::Update(void)
{
	Move();
	CreateBullet();

	return true;
}

void CEnemy::Render(void)
{

	ConsoleBuffer::GetInstance()->Sprite_Draw(_iY, _iX, _Skin);
}

bool CEnemy::OnCollision(CBaseObject* ptr)
{
	// 유닛이랑 충돌은 무조건 Bullet이다. 
	CBullet* pBullet = static_cast<CBullet*>(ptr);

	if (pBullet->Collision(_iY, _iX))
	{
		_Hp -= pBullet->GetDamage();

		if (_Hp < 1)
			_Visible = false;

		return true;
	}
	else
	{
		return false;
	}
}

void CEnemy::GetMoving(char* cMoveFile)
{
	int iCnt;
	char cString[256];
	int iMaxMoving=0;

	Moving temp;

	CParsing_ANSI Parsing;

	sprintf_s(cString, 256, "GameFile\\%s", cMoveFile);

	Parsing.LoadFile(cString);
	Parsing.GetValue("MaxMoving", &iMaxMoving);

	for (iCnt = 1; iCnt <= iMaxMoving; ++iCnt)
	{
		sprintf_s(cString, "MovingCool%d", iCnt);
		Parsing.GetValue(cString, &temp._CoolTime);

		sprintf_s(cString, "MovingY%d", iCnt);
		Parsing.GetValue(cString, &temp.dY);

		sprintf_s(cString, "MovingX%d", iCnt);
		Parsing.GetValue(cString, &temp.dX);

		MovingList.push_back(temp);
	}
}

void CEnemy::GetBullet(char* cBulletFile)
{
	int iCnt;
	char cString[256];
	int iMaxBullet;
	Bullet temp;
	
	CParsing_ANSI Parsing;
	sprintf_s(cString, 256, "GameFile\\%s", cBulletFile);

	Parsing.LoadFile(cString);
	Parsing.GetValue("MaxBullet", &iMaxBullet);

	for (iCnt = 1; iCnt <= iMaxBullet; ++iCnt)
	{
		sprintf_s(cString, "Skin%d", iCnt);
		Parsing.GetValue(cString, &temp._Skin);

		sprintf_s(cString, "Damage%d", iCnt);
		Parsing.GetValue(cString, &temp._iDamage);

		sprintf_s(cString, "DY%d", iCnt);
		Parsing.GetValue(cString, &temp._dY);

		sprintf_s(cString, "DX%d", iCnt);
		Parsing.GetValue(cString, &temp._dX);

		sprintf_s(cString, "CoolTime%d", iCnt);
		Parsing.GetValue(cString, &temp._iCoolTime);
		sprintf_s(cString, "Speed%d", iCnt);
		Parsing.GetValue(cString, &temp._iSpeed);

		BulletList.push_back(temp);
	}
}


void CEnemy::Move(void)
{
	// 움직일 프레임이라면 움직인다.
	if (CFpsManager::GetInstance()->_CntFps % (*iter)._CoolTime == 0)
	{
		_iY += (*iter).dY;
		_iX += (*iter).dX;

		_dy = (*iter).dY;
		_dx = (*iter).dX;

		++iter;
		if (iter == MovingList.end())
			iter = MovingList.begin();
	}
}

void CEnemy::CreateBullet(void)
{
	CList<Bullet>::iterator iter;

	for (iter = BulletList.begin(); iter != BulletList.end(); ++iter)
	{
		Bullet tan = *iter;

		// 프레임 단위로 쿨타임을 계산한다.
		if (CFpsManager::GetInstance()->_CntFps % tan._iCoolTime == 0)
		{
			CBullet* pBullet = new CBullet(
				_ObjectType,
				true,
				tan._Skin,
				_iY,
				_iX,
				tan._dY,
				tan._dX,
				tan._iDamage,
				tan._iSpeed);

			CObjectManager::GetInstance()->CreateBullet(pBullet);
		}
	}
}

