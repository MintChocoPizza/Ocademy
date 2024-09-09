
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
	// ������ ������ �д´�. 

	errno_t err;
	FILE* pFile;
	long lFileSize;

	//-----------------------------------------------------------------------------------
	// ���ȭ�� 
	// 
	// �ؽ�Ʈ�� �ۼ��� ���ȭ�� -> ����Ʈ�� ������ ȭ�� ��ġ�� ������.
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
	// ������ Ÿ��Ʋ�� ������ ����
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
	// ���ۿ� Ÿ��Ʋ ���
	// 
	//-------------------------
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT/2-2, cGameName);

	//-------------------------
	// ���ۿ� ���� ���� ���
	// 
	//-------------------------
	szStringLength = strlen(cGameVersion);
	for (iCnt = 0; iCnt < szStringLength; ++iCnt)
	{
		ConsoleBuffer::GetInstance()->Sprite_Draw(dfSCREEN_HEIGHT - 2, 1 + iCnt, cGameVersion[iCnt]);
	}
	//-------------------------
	// ���ۿ� ���ۿ� ���� ���� 
	// 
	// ���
	// 
	//-------------------------
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 2, "Move: WASD, Weapon: J, Escape: ESC");
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 8, "Press Enter.......");
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 4, "���� �ð� �ø���: N");
	ConsoleBuffer::GetInstance()->Sprite_Center_String(dfSCREEN_HEIGHT / 2 + 6, "���� �ð� ���̱�: M");
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
