

#include <stdio.h>
#include <Windows.h>
#include "MyNew.h"
#include "FPSManager.h"

CFpsManager CFpsManager::_CFpsManager;

CFpsManager::CFpsManager()
{
	timeBeginPeriod(1);
	_CntFps = 0;
	_LogicFps = 0;
	_RanderFps = 0;
	_cpLogicFps = 0;
	_cpRanderFps = 0;

	_StartTime = timeGetTime();
	_EndTime = 0;
	_OneSecondTime = _StartTime;
	_AddTime = 0;
}

CFpsManager::~CFpsManager()
{
	timeEndPeriod(1);
}

CFpsManager* CFpsManager::GetInstance(void)
{
	return &_CFpsManager;
}

void CFpsManager::LogicTimeUpdate(void)
{
	_LogicFps++;
	_CntFps++;
}

bool CFpsManager::FpsSkip(void)
{
	bool ret;
	_EndTime = timeGetTime();


	//---------------------------------------------------------------
	// 1�ʸ��� fps ����
	//---------------------------------------------------------------
	if (_EndTime - _OneSecondTime >= 1000)
	{
		_OneSecondTime += 1000;
		_cpLogicFps = _LogicFps;
		_cpRanderFps = _RanderFps;

		_LogicFps = 0;
		_RanderFps = 0;
	}

	//---------------------------------------------------------------
	// �ڵ带 �ѹ��� �� �ð��� 20ms���� ������ �׸�ŭ ����.
	// 
	//---------------------------------------------------------------
	if (_EndTime - _StartTime < 20)
	{
		_StartTime += 20;
		Sleep(_StartTime - _EndTime);
		++_RanderFps;
		ret = true;
	}
	else
	{
		_StartTime += 20;
		ret = false;
	}

	return ret;
}

void CFpsManager::AddLogicFps(void)
{
	if (GetAsyncKeyState('N') & 0x8000)
	{
		++_AddTime;
	}

	if ((_AddTime > 0) && GetAsyncKeyState('M') & 0x8001)
	{
		--_AddTime;
	}

	Sleep(_AddTime);
}

void CFpsManager::print_FPS(void)
{
	printf_s("����: %d   -   ���� :%d   -   �߰�����: %d \n", _cpLogicFps, _cpRanderFps, _AddTime);
}
