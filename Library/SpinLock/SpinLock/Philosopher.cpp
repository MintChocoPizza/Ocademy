
#include <vector>
#include <thread>
#include <Windows.h>
#include <stdio.h>
#include "Fork.h"
#include "Philosopher.h"
#include "DiningPhilosophers.h"


Philosopher::Philosopher(int id, DiningPhilosophers* main)
{
	mID = id;
	mMain = main;
}

Philosopher::~Philosopher()
{
}

void Philosopher::run(void)
{
	int i = 0;
	int next = (mID + 1) % mMain->nNum;

	while (i < 1000)
	{
		mMain->mForks[0].get(mID);

		printf("%d th philosopher eats \n", mID);

		mMain->mForks[0].put();

		printf("%d th philosopher thinks \n", mID);

		++i;
	}
}
