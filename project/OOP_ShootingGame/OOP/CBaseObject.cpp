#include "MyNew.h"
#include "CBaseObject.h"

CBaseObject::CBaseObject(int ObjectType, bool Visible) :
	_ObjectType(ObjectType), _Visible(Visible)
{
}

CBaseObject::~CBaseObject()
{
}

int CBaseObject::GetVisible(void)
{
	return _Visible;
}

int CBaseObject::GetObjectType(void)
{
	return _ObjectType;
}
