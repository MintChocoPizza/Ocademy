
#include <Windows.h>

#include "FPSManager.h"
#include "ConsoleBuffer.h"

#include "CBaseObject.h"

#include "CollisionObject.h"

#include "CBullet.h"


CBullet::CBullet(int ObjectType,
				bool Visible,
				char Skin,
				int iY,
				int iX,
				int dy,
				int dx,
				int iDamage,
				int iSpeed)
	: CollisionObject(ObjectType, Visible), _Skin(Skin), _iDamage(iDamage), _iSpeed(iSpeed)
{
	_iY = iY;
	_iX = iX;
	_dy = dy;
	_dx = dx;
}

CBullet::~CBullet()
{
}

bool CBullet::Update(void)
{
	_iY += _dy;
	_iX += _dx;

	if (_iY < 0 || _iX < 0 || _iX>79 || _iY>23)
	{
		_Visible = false;
	}

	return true;
}

void CBullet::Render(void)
{
	ConsoleBuffer::GetInstance()->Sprite_Draw(_iY, _iX, _Skin);
}

bool CBullet::OnCollision(CBaseObject* ptr)
{
	CollisionObject* pObject = static_cast<CollisionObject *>(ptr);

	// 총알의 경우 충돌했다면 지우기를 예약한다.
	if (pObject->Collision(_iY, _iX))
	{
		_Visible = false;
		return true;
	}
	else
	{
		return false;
	}
}

int CBullet::GetDamage(void)
{
	return _iDamage;
}
