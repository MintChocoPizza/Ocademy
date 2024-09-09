#ifndef __FPSMANAGER_H__
#define __FPSMANAGER_H__

class CFpsManager
{
public:
	// ��� �������� Fps ������ �����δ�. 
	// �������� Ǯ�ų� public���� Ǯ�ų�.
	// ��κ��� ������Ʈ���� �����带 �ؾ���.
	unsigned int _CntFps;

private:
	static CFpsManager _CFpsManager;

	//--------------------------------
	// �ð� ���� ����
	// 
	//--------------------------------
	DWORD _StartTime;
	DWORD _EndTime;
	DWORD _OneSecondTime;
	DWORD _AddTime;

	//--------------------------------
	// ������ ���� ����
	// �����̶� �⺻ 0
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
	// 50FPS -> 1�ʿ� 50�� ���� -> 1000ms 50�� ����
	// ==> 20ms�� 1�� ����
	// 
	//---------------------------------------------------------------
	bool FpsSkip(void);


	void AddLogicFps(void);


	// friend �Ⱦ ���
	void print_FPS(void);
};



#endif // !__FPSMANAGER_H__


