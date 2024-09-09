

#include <Windows.h>
#include "MyNew.h"
#include "ConsoleBuffer.h"
#include "CList.h"


/////////////////////////////////////////////////////////////////////////////////
// CTitleObject.h ���Ͽ��� CBaseObject.h Ŭ������ ����ϱ� ������ 
// 
// �ΰ��� ���� CBaseObject.h�� ���� �����Ͽ��� �Ѵ�.
// 
/////////////////////////////////////////////////////////////////////////////////
#include "CBaseObject.h"
#include "CTitleObject.h"
#include "CObjectManager.h"
#include "CSceneBase.h"
#include "CSceneTitle.h"


CSceneTitle::CSceneTitle()
{
	// Ÿ��Ʋ Scene�� �´� �⺻ ��ü ���� �� �ʱ�ȭ
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

