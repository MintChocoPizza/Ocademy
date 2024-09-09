
#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__


void SpinLockInitialize(LONG* flag);
void SpinLock(LONG*flag);
void SpinUnlock(LONG*flag);

#endif // !__SPINLOCK_H__

