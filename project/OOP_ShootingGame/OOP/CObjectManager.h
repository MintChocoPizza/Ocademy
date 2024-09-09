#ifndef __COBJECTMANAGER_H__
#define __COBJECTMANAGER_H__


////////////////////////////////////////////////////////////////////////////
// 오브젝트들을 관리할 싱글톤 클래스
// 
// 
////////////////////////////////////////////////////////////////////////////
class CObjectManager
{
private:
	static CObjectManager _CObjectManager;


	CList<CBaseObject*> ObjectList;
	CList<CBaseObject*> BulletList;

	bool PlayerDie;
	bool EnemyAllDie;

private:
	CObjectManager();
	~CObjectManager();


public:
	static CObjectManager* GetInstance(void);


	void ClearCheck(void);
	bool GetPlayerDie(void);
	bool GetEnemyAllDie(void);



	void CreateObject(void* Object);
	void CreateBullet(void* Object);


	void ObjectClear(void);
	void ObjectUpdate(void);
	void ObjectRender(void);
	void ObjectVisibleCheck(void);


	void BulletClear(void);
	void BulletUpdate(void);
	void BulletRender(void);
	void BulletVisibleCheck(void);
};


#endif // !__COBJECTMANAGER_H__
