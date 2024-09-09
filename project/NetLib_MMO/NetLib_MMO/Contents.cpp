#include <Windows.h>
#include <Windows.h>
#include <unordered_map>

#include "Define.h"
#include "Protocol.h"
#include "CMemPoolList.h"
#include "MOVE_DIR.h"
#include "Direction.h"
#include "Sector.h"
#include "Client.h"
#include "Contents.h"

std::unordered_map<DWORD64, st_CLIENT*> g_Client_Hash;
SRWLOCK g_srw_Client_Hash;

int g_Shutdown = false;

void __stdcall UpdateThread(void* param)
{
	InitializeSRWLock(&g_srw_Client_Hash);


	
	while (!g_Shutdown)
	{

	}
}
