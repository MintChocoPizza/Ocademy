
#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>
#include "MyNew.h"
#include "CParsing_ANSI.h"
#include "ConsoleBuffer.h"

//-----CSceneManager.h --------------
#include "CSceneBase.h"
#include "CSceneManager.h"

//-----CTitleObject.h---------------
#include "CBaseObject.h"
#include "CTitleObject.h"


//CTitleObject::CTitleObject()
//{
// 
//}

CTitleObject::CTitleObject(const char* ccpFileName, const char* ccpGameInfo, int ObjectType, bool Visible) : CBaseObject(ObjectType, Visible)
{
	// 생성시 파일을 읽는다. 

	errno_t err;
	FILE* pFile;
	long lFileSize;

	//-----------------------------------------------------------------------------------
	// 배경화면 
	// 
	// 텍스트로 작성된 배경화면 -> 바이트로 읽으면 화면 위치가 깨진다.
	//-----------------------------------------------------------------------------------
	err = fopen_s(&pFile, ccpFileName, "r");
	if (err != 0)
	{
		printf_s("%s file open false \n", ccpFileName);
		throw;
	}

	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	_Buff = (unsigned char*)malloc(lFileSize);
	if (_Buff == NULL)
	{
		printf_s("CTitieObject malloc false \n");
		throw;
	}
	fread_s(_Buff, lFileSize, 1, lFileSize, pFile);
	fclose(pFile);


	//-----------------------------------------------------------------------------------
	// 게임의 타이틀과 게임의 버전
	// 
	//-----------------------------------------------------------------------------------
	CParsing_ANSI cParsing;
	cParsing.LoadFile(ccpGameInfo);
	cParsing.GetValue("gameName", cGameName, 30);
	cParsing.GetValue("Version", cGameVersion, 10);
}

CTitleObject::~CTitleObject()
{
	free(_Buff);
}

bool CTitleObject::Update(void)
{
	KeyboardInput();

	return true;
}

void CTitleObject::Render(void)
{
	unsigned int iCnt;
	size_t szStringLength;

	ConsoleBuffer::GetInstance()->Sprite_Background(_Buff);

	//-------------------------
	// 버퍼에 타이틀 출력
	// 
	//-------------------------
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT/2-2, cGameName);

	//-------------------------
	// 버퍼에 게임 버전 출력
	// 
	//-------------------------
	szStringLength = strlen(cGameVersion);
	for (iCnt = 0; iCnt < szStringLength; ++iCnt)
	{
		ConsoleBuffer::GetInstance()->Sprite_Draw(dfSCREEN_HEIGHT - 2, 1 + iCnt, cGameVersion[iCnt]);
	}
	//-------------------------
	// 버퍼에 조작에 대한 설명 
	// 
	// 출력
	// 
	//-------------------------
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 2, "Move: WASD, Weapon: J, Escape: ESC");
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 8, "Press Enter.......");
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 4, "로직 시간 늘리기: N");
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 6, "로직 시간 줄이기: M");
}

bool CTitleObject::OnCollision(CBaseObject* ptr)
{
	return false;
}

void CTitleObject::KeyboardInput(void)
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8001)
	{
		CSceneManager::GetInstance()->LoadScene(CSceneManager::LOAD);
	}
}
