#ifndef __FPSMANAGER_H__
#define __FPSMANAGER_H__

class CFpsManager
{
public:
	// 모든 움직임을 Fps 단위로 움직인다. 
	// 전역으로 풀거나 public으로 풀거나.
	// 대부분의 오브젝트마다 프렌드를 해야함.
	unsigned int _CntFps;

private:
	static CFpsManager _CFpsManager;

	//--------------------------------
	// 시간 측정 변수
	// 
	//--------------------------------
	DWORD _StartTime;
	DWORD _EndTime;
	DWORD _OneSecondTime;
	DWORD _AddTime;

	//--------------------------------
	// 프레임 전역 변수
	// 전역이라 기본 0
	//--------------------------------
	unsigned int _LogicFps;
	unsigned int _RanderFps;
	unsigned int _cpLogicFps;
	unsigned int _cpRanderFps;

private:
	CFpsManager();
	~CFpsManager();


public:
	static CFpsManager* GetInstance(void);

	void LogicTimeUpdate(void);
	//---------------------------------------------------------------
	// 50FPS -> 1초에 50번 실행 -> 1000ms 50번 실행
	// ==> 20ms에 1번 실행
	// 
	//---------------------------------------------------------------
	bool FpsSkip(void);


	void AddLogicFps(void);


	// friend 싫어서 편법
	void print_FPS(void);
};



#endif // !__FPSMANAGER_H__


