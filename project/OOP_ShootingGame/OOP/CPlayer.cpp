

#include <stdio.h>
#include <Windows.h>
#include "MyNew.h"
#include "CList.h"
#include "ConsoleBuffer.h"
#include "CParsing_ANSI.h"
#include "FPSManager.h"

#include "CSceneBase.h"
#include "CSceneManager.h"

#include "CBaseObject.h"
#include "CollisionObject.h"
#include "CObjectManager.h"
#include "CBullet.h"
#include "CPlayer.h"

CPlayer::CPlayer(int ObjectType, bool Visible, char* PlayerFile) 
	: CollisionObject(ObjectType, Visible)
{
	
	_iY = dfSCREEN_HEIGHT - 3;
	_iX = (dfSCREEN_WIDTH - 1) / 2;
	_dy = _iY;
	_dx = _iX;


	char cString[255] = "";
	sprintf_s(cString, "GameFile\\%s", PlayerFile);

	CParsing_ANSI CParsing;
	CParsing.LoadFile(cString);
	CParsing.GetValue("HP", &_iHP);
	CParsing.GetValue("Skin", &_cSkin);
	CParsing.GetValue("Bullet", &_cBulletSkin);
	CParsing.GetValue("Damage", &_iDamage);
	CParsing.GetValue("Speed", &_iPlayerSpeed);
	CParsing.GetValue("CoolTime", &_iBulletCoolTime);
	CParsing.GetValue("BulletSpeed", &_iBulletSpeed);
}

CPlayer::~CPlayer()
{

}

bool CPlayer::Update(void)
{
	KeyboardInput();


	return true;
}

void CPlayer::Render(void)
{
	ConsoleBuffer::GetInstance()->Sprite_Draw(_iY, _iX, _cSkin);
}

bool CPlayer::OnCollision(CBaseObject* ptr)
{
	// 이 유닛과 충돌하는 것은 무조건 총알이다. 

	CBullet* pBullet = static_cast<CBullet*>(ptr);

	if (pBullet->Collision(_iY, _iX))
	{
		_iHP -= pBullet->GetDamage();
		if (_iHP < 1)
			_Visible = false;

		return true;
	}
	else
	{
		return false;
	}
}

void CPlayer::KeyboardInput(void)
{
	_dy = _iY;
	_dx = _iX;

	// _iPlayerSpeed프레임 마다 이동한다.
	if (CFpsManager::GetInstance()->_CntFps % _iPlayerSpeed == 0)
	{
		// 왼쪽 이동 A.
		if (GetAsyncKeyState('A'))
		{
			_iX += -1;
		}
		// 오른쪽 이동 D.
		if (GetAsyncKeyState('D'))
		{
			_iX += 1;
		}
		// 위쪽 이동 W.
		if (GetAsyncKeyState('W') & 0x8001)
		{
			_iY -= 1;
		}
		// 아래쪽 이동 S.
		if (GetAsyncKeyState('S') & 0x8001)
		{
			_iY += 1;
		}
	}
	//-------------------------------------------------------------
	// 플레이어 이동 반경 제한.
	// 게임 화면에서 플레이어가 이동 가능한 구역을 제한한다.
	//-------------------------------------------------------------

	_iX = max(_iX, 0);
	_iX = min(_iX, 79);
	_iY = max(_iY, 0);
	_iY = min(_iY, 23);


	// J 키, (미사일 키)
	if (CFpsManager::GetInstance()->_CntFps % _iBulletCoolTime == 0)
	{
		if (GetAsyncKeyState('J') & 0x8001)
		{
			CBullet* Bullet = new CBullet(_ObjectType, true, _cBulletSkin, _iY - 1, _iX, -1, 0, _iDamage, _iBulletSpeed);
			
			CObjectManager::GetInstance()->CreateBullet(Bullet);
		}
	}
}
