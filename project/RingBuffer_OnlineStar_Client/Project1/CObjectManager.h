
#ifndef __CSOBJECTMANAGER_H__
#define __CSOBJECTMANAGER_H__

class C_ObjectManager
{
private:
	static C_ObjectManager _C_ObjectManager;
	
	CList<C_ObjectBase* > ObjectList;

private:
	C_ObjectManager();
	~C_ObjectManager();

public:
	static C_ObjectManager* GetInstance(void);

	//////////////////////////////////////////
	// 필수 함수
	// 
	//////////////////////////////////////////
	void ObjectUpdate(void);
	void ObjectRender(void);



};


#endif // !__CSOBJECTMANAGER_H__
