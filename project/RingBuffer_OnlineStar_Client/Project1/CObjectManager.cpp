
#include "CObjectBase.h"
#include "CList.h"
#include "CObjectManager.h"


C_ObjectManager C_ObjectManager::_C_ObjectManager;

C_ObjectManager::C_ObjectManager()
{

}

C_ObjectManager::~C_ObjectManager()
{
}

C_ObjectManager* C_ObjectManager::GetInstance(void)
{
	return &_C_ObjectManager;
}

void C_ObjectManager::ObjectUpdate(void)
{
	CList<C_ObjectBase*>::iterator iter;
	for (iter = ObjectList.begin(); iter != ObjectList.end(); ++iter)
	{
	}
}

void C_ObjectManager::ObjectRender(void)
{
}
