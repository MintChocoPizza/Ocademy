#include <Windows.h>
#include "MyNew.h"
#include "FPSManager.h"
#include "ConsoleBuffer.h"

#include "CList.h"
#include "CBaseObject.h"
#include "CSceneBase.h"

#include "CObjectManager.h"
#include "CSceneOver.h"

CSceneOver::CSceneOver()
{
}

CSceneOver::~CSceneOver()
{
}

bool CSceneOver::Update(void)
{
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 2, "게임 오버");
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 4, "끝내려면 Enter를 누르세요");

	if (GetAsyncKeyState(VK_RETURN) & 0x8001)
		return false;
	else
		return true;
}
