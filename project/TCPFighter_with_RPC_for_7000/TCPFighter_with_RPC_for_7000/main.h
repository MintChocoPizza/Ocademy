#pragma once

#include "C_Logger.h"

extern DWORD g_Start_Time;
extern DWORD g_End_Time;       // 매 루프마다 timeGetTime을 호출한다.
extern DWORD g_One_Second;
extern SerializeBuffer g_Packet;


extern DWORD dwUpdateFPS;
extern DWORD dwDebug_Loop;
extern DWORD g_dwFirst;
