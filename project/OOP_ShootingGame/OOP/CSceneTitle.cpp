

#include <Windows.h>
#include "MyNew.h"
#include "ConsoleBuffer.h"
#include "CList.h"


/////////////////////////////////////////////////////////////////////////////////
// CTitleObject.h 파일에서 CBaseObject.h 클래스를 사용하기 때문에 
// 
// 두개를 같이 CBaseObject.h를 위에 선언하여야 한다.
// 
/////////////////////////////////////////////////////////////////////////////////
#include "CBaseObject.h"
#include "CTitleObject.h"
#include "CObjectManager.h"
#include "CSceneBase.h"
#include "CSceneTitle.h"


CSceneTitle::CSceneTitle()
{
	// 타이틀 Scene에 맞는 기본 객체 생성 및 초기화
	CTitleObject* TitleObject = new CTitleObject("GameFile\\Title", "GameFile\\GameInfo.txt",0, true);
	
		
	CObjectManager::GetInstance()->CreateObject(TitleObject);
}

CSceneTitle::~CSceneTitle()
{
	CObjectManager::GetInstance()->ObjectClear();
}

bool CSceneTitle::Update(void)
{

	CObjectManager::GetInstance()->ObjectUpdate();
	CObjectManager::GetInstance()->ObjectRender();

	return true;
}

