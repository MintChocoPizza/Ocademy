#include "MyNew.h"
#include "CList.h"
#include "CBaseObject.h"
#include "CSceneBase.h"
#include "CSceneManager.h"

#include "CObjectManager.h"


CObjectManager CObjectManager::_CObjectManager;

CObjectManager::CObjectManager()
{
	PlayerDie = true;
	EnemyAllDie = true;
}

CObjectManager::~CObjectManager()
{
}

CObjectManager* CObjectManager::GetInstance(void)
{
	return &_CObjectManager;
}

bool CObjectManager::GetPlayerDie(void)
{
	return PlayerDie;
}

bool CObjectManager::GetEnemyAllDie(void)
{
	return EnemyAllDie;
}

void CObjectManager::CreateObject(void* Object)
{
	ObjectList.push_back(static_cast<CBaseObject*> (Object));
	
}

void CObjectManager::CreateBullet(void* Object)
{
	BulletList.push_back(static_cast<CBaseObject*>(Object));
}







void CObjectManager::ObjectClear(void)
{
	CList<CBaseObject*> ::iterator removeIter;

	for (removeIter = ObjectList.begin(); removeIter != ObjectList.end();)
	{
		CBaseObject* removeObject = *removeIter;
		setLog(__FILE__, __LINE__);
		delete removeObject;

		removeIter = ObjectList.erase(removeIter);
	}
}

void CObjectManager::ObjectUpdate(void)
{
	CList<CBaseObject*>::iterator iter;

	for (iter = ObjectList.begin(); iter != ObjectList.end(); ++iter)
	{
		(*iter)->Update();
	}
}

void CObjectManager::ObjectRender(void)
{
	CList<CBaseObject*>::iterator iter;

	for (iter = ObjectList.begin(); iter != ObjectList.end(); ++iter)
	{
		(*iter)->Render();
	}
}

void CObjectManager::ObjectVisibleCheck(void)
{
	// �Ҹ��ڸ� virtual�� �����Ͽ� 
	// CBaseObject�� �Ҹ���ѵ� �� �Ҹ�ȴ�.
	CList<CBaseObject*>::iterator iter;

	PlayerDie = true;
	EnemyAllDie = true;

	for (iter = ObjectList.begin(); iter != ObjectList.end();)
	{
		CBaseObject* Object = *iter;
		if (Object->GetVisible() == false)
		{
			setLog(__FILE__, __LINE__);
			delete Object;

			iter = ObjectList.erase(iter);

		}
		else
		{	
			if (Object->GetObjectType() == 1)
				PlayerDie = false;

			else if (Object->GetObjectType() == 2)
				EnemyAllDie = false;

			++iter;
		}
	}
}





void CObjectManager::BulletClear(void)
{
	CList<CBaseObject*> ::iterator removeIter;

	for (removeIter = BulletList.begin(); removeIter != BulletList.end();)
	{
		CBaseObject* removeObject = *removeIter;

		setLog(__FILE__, __LINE__);
		delete removeObject;

		removeIter = BulletList.erase(removeIter);
	}
}

void CObjectManager::BulletUpdate(void)
{
	CList<CBaseObject*>::iterator iter;

	for (iter = BulletList.begin(); iter != BulletList.end(); ++iter)
	{
		CBaseObject* pBullet = *iter;
		pBullet->Update();

		
		// �浹ó��
		// Type 0���� �浹���� �ʴ� ������Ʈ�̴�.
		if (pBullet->GetObjectType() == 0)
			continue;

		CList<CBaseObject*>::iterator target_iter;
		for (target_iter = ObjectList.begin(); target_iter != ObjectList.end(); ++target_iter)
		{
			CBaseObject* pObject = *target_iter;

			// �Ѿ��� ������ ������Ʈ�� �Ѿ��� Ÿ���� ����
			if (pBullet->GetObjectType() == pObject->GetObjectType())
				continue;

			// �Ѿ˰� ������ ����Ʈ�� �и��Ͽ���. 
			// �ش� �Ѿ˰� �浹�ϴ� ���� ������ ������. 
			if (pBullet->OnCollision(pObject))
			{
				pObject->OnCollision(pBullet);
			}
		}

	}
}

void CObjectManager::BulletRender(void)
{
	CList<CBaseObject*>::iterator iter;

	for (iter = BulletList.begin(); iter != BulletList.end(); ++iter)
	{
		(*iter)->Render();
	}
}

void CObjectManager::BulletVisibleCheck(void)
{
	CList<CBaseObject*>::iterator iter = BulletList.begin();

	for (iter = BulletList.begin(); iter != BulletList.end();)
	{
		CBaseObject* Object = *iter;
		if (Object->GetVisible() == false)
		{
			setLog(__FILE__, __LINE__);
			delete Object;

			iter = BulletList.erase(iter);

		}
		else
		{
			++iter;
		}
	}
}

