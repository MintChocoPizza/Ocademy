
#include <stdlib.h>
#include "MyNew.h"
#include "CBaseObject.h"
#include "CollisionObject.h"

CollisionObject::CollisionObject(int ObjectType, bool Visible)
	:CBaseObject(ObjectType, Visible)
{
	_iY = 0;
	_iX = 0;
	_dy = 0;
	_dx = 0;
}

CollisionObject::~CollisionObject()
{
}

bool CollisionObject::Collision(int iY, int iX)
{
	if (_iY == iY && _iX == iX)
		return true;
	else
		return false;
}

