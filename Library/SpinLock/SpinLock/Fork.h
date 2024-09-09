#pragma once

class Fork
{
public:
	int mID;
	int mOner;
	int mMutex;

public:
	Fork();
	Fork(int id);
	~Fork();

	void printGet(int who);
	void get(int who);
	bool tryGet(int who);
	void put(void);
};
