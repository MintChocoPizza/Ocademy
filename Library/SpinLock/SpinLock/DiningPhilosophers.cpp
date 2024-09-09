
#include <vector>
#include <thread>
#include <new>
#include <Windows.h>
#include "Fork.h"
#include "Philosopher.h"
#include "DiningPhilosophers.h"

DiningPhilosophers::DiningPhilosophers()
{
	mPhilosophers = new Philosopher[nNum];
	mForks = new Fork[1];

	for (int i = 0; i < nNum; ++i)
	{
		new(&mPhilosophers[i]) Philosopher(i, this);
		new(&mForks[0]) Fork(0);
	}
}

DiningPhilosophers::~DiningPhilosophers()
{
}

void DiningPhilosophers::start(void)
{

	for (int i = 0; i < nNum; ++i)
	{
		// 스레드로 변경해야함.
		threads.push_back(std::thread (&Philosopher::run, &(mPhilosophers[i])));
		//mPhilosophers[i].run();
	}

}

void DiningPhilosophers::join(void)
{
	for (auto& thread : threads)
	{
		thread.join();
	}
}
