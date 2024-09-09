

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
	// �� ���ְ� �浹�ϴ� ���� ������ �Ѿ��̴�. 

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

	// _iPlayerSpeed������ ���� �̵��Ѵ�.
	if (CFpsManager::GetInstance()->_CntFps % _iPlayerSpeed == 0)
	{
		// ���� �̵� A.
		if (GetAsyncKeyState('A'))
		{
			_iX += -1;
		}
		// ������ �̵� D.
		if (GetAsyncKeyState('D'))
		{
			_iX += 1;
		}
		// ���� �̵� W.
		if (GetAsyncKeyState('W') & 0x8001)
		{
			_iY -= 1;
		}
		// �Ʒ��� �̵� S.
		if (GetAsyncKeyState('S') & 0x8001)
		{
			_iY += 1;
		}
	}
	//-------------------------------------------------------------
	// �÷��̾� �̵� �ݰ� ����.
	// ���� ȭ�鿡�� �÷��̾ �̵� ������ ������ �����Ѵ�.
	//-------------------------------------------------------------

	_iX = max(_iX, 0);
	_iX = min(_iX, 79);
	_iY = max(_iY, 0);
	_iY = min(_iY, 23);


	// J Ű, (�̻��� Ű)
	if (CFpsManager::GetInstance()->_CntFps % _iBulletCoolTime == 0)
	{
		if (GetAsyncKeyState('J') & 0x8001)
		{
			CBullet* Bullet = new CBullet(_ObjectType, true, _cBulletSkin, _iY - 1, _iX, -1, 0, _iDamage, _iBulletSpeed);
			
			CObjectManager::GetInstance()->CreateBullet(Bullet);
		}
	}
}
