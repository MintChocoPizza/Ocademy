#pragma once

class DiningPhilosophers
{
public:
	const int		nNum = 5;
	Philosopher		*mPhilosophers;
	Fork			*mForks;
	std::vector<std::thread> threads;
public:
	DiningPhilosophers();
	~DiningPhilosophers();

	void start(void);

	void join(void);
};
