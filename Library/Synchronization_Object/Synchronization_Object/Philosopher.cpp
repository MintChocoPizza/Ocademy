
#include <vector>
#include <thread>
#include <Windows.h>
#include <stdio.h>
#include "Synchronization_Object.h"
#include "Fork.h"
#include "Philosopher.h"
#include "DiningPhilosophers.h"

void Philosopher::run(void)
{
	int i = 0;
	int next = (mID + 1) % mMain->nNum;

	while (i < 1000)
	{
		mMain->mForks[mID].get(mID);
		if (mMain->mForks[next].tryGet(mID) == false)
		{
			mMain->mForks[mID].put();
			Sleep(1);
			continue;
		}

		printf("%dth philosopher eats \n", mID);

		mMain->mForks[next].put();
		mMain->mForks[mID].put();

		printf("%dth philosopher thinks \n", mID);
		i++;
	}
}
