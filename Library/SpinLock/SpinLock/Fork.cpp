
#include <stdio.h>
#include <Windows.h>
#include "SpinLock.h"
#include "Fork.h"

Fork::Fork()
{
}

Fork::Fork(int id)
{
	mID = id;
	mMutex = 0;
	mOner = -1;
}

Fork::~Fork()
{
}

void Fork::printGet(int who)
{
	mOner = who;
	printf("%d th philosoper picks %d th fork \n", mOner, mID);
}

void Fork::get(int who)
{
	SpinLock(&mMutex);

	printGet(who);
}

bool Fork::tryGet(int who)
{
	return false;
}

void Fork::put(void)
{
	printf("%d th philosoper releases %d th fork \n", mOner, mID);
	mOner = -1;
	SpinUnlock(&mMutex);
}
