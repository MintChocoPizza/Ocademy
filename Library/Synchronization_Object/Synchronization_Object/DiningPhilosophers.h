#pragma once


class DiningPhilosophers
{
public:
	const int nNum = 5;
	Philosopher* mPhilosophers;
	Fork* mForks;
	std::vector<std::thread> threads;

public:
	DiningPhilosophers()
	{
		mPhilosophers = new Philosopher[nNum];
		mForks = new Fork[nNum];

		for (int i = 0; i < nNum; ++i)
		{
			new(&mPhilosophers[i])Philosopher(i, this);
			new(&mForks[i])Fork(i);
		}
	}

	~DiningPhilosophers()
	{
		delete[] mPhilosophers;
		delete[] mForks;
	}

	void start(void)
	{
		for (int i = 0; i < nNum; ++i)
		{
			// mPhilosophers[i].run();
			threads.push_back(std::thread(&Philosopher::run, &(mPhilosophers[i])));
		}
	}

	void join(void)
	{
		// 스레드의 종료를 확인한다. 
		for (auto& thread : threads)
		{
			thread.join();
		}
	}
};
