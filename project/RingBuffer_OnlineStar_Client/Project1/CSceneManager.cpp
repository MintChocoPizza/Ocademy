
#include <Windows.h>

#include "CSceneBase.h"
#include "CSceneManager.h"


C_SceneManager C_SceneManager::_C_SceneManager;

C_SceneManager::C_SceneManager(): type(LOAD)
{
	
}

C_SceneManager::~C_SceneManager()
{
}

C_SceneManager* C_SceneManager::GetInstance(void)
{
	return nullptr;
}

bool C_SceneManager::run(void)
{
	//-------------------------------------------------------------
	// ESC 키를 누르면 종료한다. 
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001)
		return false;

	


	return true;
}
