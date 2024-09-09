
// 프레임 워크라고 하면 Manager와 Base 파일에 변화가 없어야 할거 같음.
// 하지만 메니저에서 씬의 메모리를 할당하고 해제하기 위해서는 
// 씬의 정보가 필수적으로 필요해보임......

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
	// 로직 강제 부하
	//--------------------------------
	CFpsManager::GetInstance()->AddLogicFps();


	//--------------------------------
	// 로직 프레임 증가
	//--------------------------------
	CFpsManager::GetInstance()->LogicTimeUpdate();


	//--------------------------------
	// 강제 게임 끝내기: ESC
	// 
	// 게임을 클리어 후 ESC를 누르고
	// Enter을 눌러서 게임을 끝내면
	// 
	// 메모리 누수가 생김
	// 
	// 게임 클리어 ->
	//type = CLEAR
	//	ESC를 누르면 게임 종료됨
	//
	//
	//	강제 게임 종료 커멘드 : ESC + Enter
	//
	//
	//	-> 여기서 엔터를 누르다가 Esc를 누르게 되면
	//
	//
	//	Esc를 누르는 순간 OVER를 로드하게 된다.
	//	거기서 바로 Enter가 입력된다.
	//	ChangeScene = true
	//	type = OVER
	//
	//	그 다음 바로 CLEAR Update호출
	//	->직전에 입력한 ESC
	//	-- > return false
	//	--->loopEscape = false (메인에 반복문 탈출 조건)
	//
	//
	//	그 다음 바로 loopEscape == false 라서 바로 랜더링
	//
	//	그 다음 바로 _pScene을 지우게 된다.현제 _pScene == CLEAR
	//
	//	하지만 바로 위 ChangeScene == true로 인하여 씬 로드
	//	_pScene = new OVER
	//	ChangeScene = false;
	//
	// 이러고 바로 loop를 탈출하여 OVER이 사라지지 않는다.
	// 
	// 결론 -> 루프를 탈출하기 직전 메모리를 해제한다.
	//-------------------------------------------------------------
	if ((GetAsyncKeyState(VK_ESCAPE) & 0x8001))
		LoadScene(OVER);



	// 로직부
	ConsoleBuffer::GetInstance()->Buffer_Clear();
	loopEscape = _pScene->Update();



	// 랜더링
	// 종료되기 직전은 무조건 출력한다.
	if (CFpsManager::GetInstance()->FpsSkip() || !loopEscape)
	{
		ConsoleBuffer::GetInstance()->Buffer_Flip();
	}

	ConsoleBuffer::GetInstance()->print_FPS();


	// 씬 전환 요청이 있다면 여기서 실질적인 씬 전환 
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
