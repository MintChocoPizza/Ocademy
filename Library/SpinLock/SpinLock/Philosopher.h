#pragma once

class DiningPhilosophers;

class Philosopher
{
public:
	int mID;
	DiningPhilosophers* mMain;

public:
	Philosopher() {};
	Philosopher(int id, DiningPhilosophers* main);
	~Philosopher();

	void run(void);
};
