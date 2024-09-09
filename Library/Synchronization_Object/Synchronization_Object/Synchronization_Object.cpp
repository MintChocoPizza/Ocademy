
#pragma comment(lib, "Synchronization.lib")

#include "Synchronization_Object.h"


bool TryLock(LONG* flag)
{
	if (InterlockedExchange(flag, 1) == 1)
		return false;

	return true;
}

void Lock(LONG* flag)
{
	LONG compare = 1;

	// 내가 드러가려고(1) 하는데 이미 누가있는경우(1) block 된다. 
	while (InterlockedExchange(flag, 1) == 1)
	{
		// 나는 해당 주소를 기다립니다. 
		WaitOnAddress(flag, &compare, sizeof(LONG), INFINITE);
	}
}

void UnLock(LONG* flag)
{
	InterlockedExchange(flag, 0);
	WakeByAddressSingle(flag);
}
