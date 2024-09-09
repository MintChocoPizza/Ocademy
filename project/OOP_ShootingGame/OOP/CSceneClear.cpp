
#include <Windows.h>
#include "MyNew.h"
#include "FPSManager.h"
#include "ConsoleBuffer.h"

#include "CList.h"
#include "CBaseObject.h"
#include "CSceneBase.h"

#include "CObjectManager.h"
#include "CSceneClear.h"



CSceneClear::CSceneClear()
{
}

CSceneClear::~CSceneClear()
{
}

bool CSceneClear::Update(void)
{
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 2, "게임 클리어");
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 4, "끝내려면 Esc누르세요");

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001)
		return false;
	else
		return true;
}
