#pragma once


class Fork
{
public:
	int mID;
	int mOner;
	LONG mMutex = 0;

public:
	Fork() {};
	Fork(int id) 
	{
		mID = id;
		mMutex = 0;
		mOner = -1;
	}
	~Fork() {};

	void printGet(int who)
	{
		mOner = who;
		printf("%dth philosoper picks %dth fork \n", mOner, mID);
	}

	void get(int who)
	{
		Lock(&mMutex);
		printGet(who);
	}

	bool tryGet(int who)
	{
		if (TryLock(&mMutex) == false)
			return false;
		printGet(who);
		return true;
	}

	void put(void)
	{
		printf("%dth philosoper releases %dth fork \n", mOner, mID);
		mOner = -1;
		UnLock(&mMutex);
	}
};
