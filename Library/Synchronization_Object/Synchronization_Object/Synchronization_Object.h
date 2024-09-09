
#ifndef __SYNCHRONIZATION_OBJECT_H__
#define __SYNCHRONIZATION_OBJECT_H__

#include <Windows.h>

bool TryLock(LONG* flag);
void Lock(LONG* flag);
void UnLock(LONG* flag);

#endif // !__SYNCHRONIZATION_OBJECT_H__
