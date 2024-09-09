
// ������ ��ũ��� �ϸ� Manager�� Base ���Ͽ� ��ȭ�� ����� �Ұ� ����.
// ������ �޴������� ���� �޸𸮸� �Ҵ��ϰ� �����ϱ� ���ؼ��� 
// ���� ������ �ʼ������� �ʿ��غ���......

#include <stdio.h>
#include <Windows.h>

#include "MyNew.h"
#include "CParsing_ANSI.h"
#include "ConsoleBuffer.h"
#include "FPSManager.h"

#include "CBaseObject.h"
#include "CTitleObject.h"
#include "CollisionObject.h"
#include "CSceneBase.h"
#include "CPlayer.h"
#include "CSceneTitle.h"
#include "CSceneGame.h"
#include "CSceneLoad.h"
#include "CSceneClear.h"
#include "CSceneOver.h"

#include "CSceneManager.h"


CSceneManager CSceneManager::_CSceneManager;

CSceneManager::CSceneManager() : ChangeScene(false), type(TITLE), gameStage(1), loopEscape(true)
{
	_pScene = new CSceneTitle;
	maxStage = GetMaxStage();
}

CSceneManager::~CSceneManager()
{
}

int CSceneManager::GetMaxStage(void)
{
	int iMaxStage;
	CParsing_ANSI cParsing;
	cParsing.LoadFile("GameFile\\MaxStage.txt");
	cParsing.GetValue("MaxStage", &iMaxStage);
	return iMaxStage;
}

CSceneManager* CSceneManager::GetInstance(void)
{
	return &_CSceneManager;
}

bool  CSceneManager::run(void)
{

	//--------------------------------
	// ���� ���� ����
	//--------------------------------
	CFpsManager::GetInstance()->AddLogicFps();


	//--------------------------------
	// ���� ������ ����
	//--------------------------------
	CFpsManager::GetInstance()->LogicTimeUpdate();


	//--------------------------------
	// ���� ���� ������: ESC
	// 
	// ������ Ŭ���� �� ESC�� ������
	// Enter�� ������ ������ ������
	// 
	// �޸� ������ ����
	// 
	// ���� Ŭ���� ->
	//type = CLEAR
	//	ESC�� ������ ���� �����
	//
	//
	//	���� ���� ���� Ŀ��� : ESC + Enter
	//
	//
	//	-> ���⼭ ���͸� �����ٰ� Esc�� ������ �Ǹ�
	//
	//
	//	Esc�� ������ ���� OVER�� �ε��ϰ� �ȴ�.
	//	�ű⼭ �ٷ� Enter�� �Էµȴ�.
	//	ChangeScene = true
	//	type = OVER
	//
	//	�� ���� �ٷ� CLEAR Updateȣ��
	//	->������ �Է��� ESC
	//	-- > return false
	//	--->loopEscape = false (���ο� �ݺ��� Ż�� ����)
	//
	//
	//	�� ���� �ٷ� loopEscape == false �� �ٷ� ������
	//
	//	�� ���� �ٷ� _pScene�� ����� �ȴ�.���� _pScene == CLEAR
	//
	//	������ �ٷ� �� ChangeScene == true�� ���Ͽ� �� �ε�
	//	_pScene = new OVER
	//	ChangeScene = false;
	//
	// �̷��� �ٷ� loop�� Ż���Ͽ� OVER�� ������� �ʴ´�.
	// 
	// ��� -> ������ Ż���ϱ� ���� �޸𸮸� �����Ѵ�.
	//-------------------------------------------------------------
	if ((GetAsyncKeyState(VK_ESCAPE) & 0x8001))
		LoadScene(OVER);



	// ������
	ConsoleBuffer::GetInstance()->Buffer_Clear();
	loopEscape = _pScene->Update();



	// ������
	// ����Ǳ� ������ ������ ����Ѵ�.
	if (CFpsManager::GetInstance()->FpsSkip() || !loopEscape)
	{
		ConsoleBuffer::GetInstance()->Buffer_Flip();
	}

	ConsoleBuffer::GetInstance()->print_FPS();


	// �� ��ȯ ��û�� �ִٸ� ���⼭ �������� �� ��ȯ 
	if (ChangeScene)
	{
		setLog(__FILE__, __LINE__);
		delete _pScene;

		switch (type)
		{
		case CSceneManager::LOAD:		
			_pScene = new CSceneLoad(gameStage, maxStage);
			++gameStage;
			break;
		case CSceneManager::GAME:
			_pScene = new CSceneGame;
			break;
		case CSceneManager::CLEAR:
			_pScene = new CSceneClear;
			break;
		case CSceneManager::OVER:
			_pScene = new CSceneOver;
			break;
		}

		ChangeScene = false;
	}

	
	if (loopEscape == false)
	{
		setLog(__FILE__, __LINE__);
		delete _pScene;
	}

	return loopEscape;
}

void CSceneManager::LoadScene(SceneType type)
{
	ChangeScene = true;

	this->type = type;
}
