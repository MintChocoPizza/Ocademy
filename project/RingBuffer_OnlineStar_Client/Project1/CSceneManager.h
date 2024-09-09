
#ifndef __CSCENEMANAGER_H__
#define __CSCENEMANAGER_H__

class C_SceneManager
{
public:
	enum SceneType
	{
		LOAD, 
		GAME
	};

private:
	static C_SceneManager _C_SceneManager;
	C_SceneBase* _pScene;

	SceneType type;
	
private:
	C_SceneManager();
	~C_SceneManager();

public:
	static C_SceneManager* GetInstance(void);
	bool run(void);
};


#endif // !__CSCENEMANAGER_H__
