#ifndef __MAIN_H__
#define __MAIN_H__

enum SCENE
{
	TITLE,
	LOAD,
	GAME,
	CLEAR,
	OVER,
	GAME_ERROR
};

extern enum SCENE g_Scene;
extern int g_Stage;
extern unsigned int g_CntFps;
extern unsigned int g_LogicFps;
extern unsigned int g_RanderFps;
extern DWORD g_AddTime;

#endif // !__MAIN_H__
