
#include <Windows.h>
#include "SpinLock.h"

#pragma optimize("",off)

void SpinLockInitialize(LONG* flag)
{
	*flag = 0;
}

void SpinLock(LONG* flag)
{
	while (InterlockedExchange(flag, 1) == 1)
	{
		YieldProcessor();
	}
}

void SpinUnlock(LONG* flag)
{
	//*(LONG*)flag = 0;
	InterlockedExchange(flag, 0);
}
#pragma optimize("",off)
