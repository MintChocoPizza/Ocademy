#include <random>

#include "MyProfile.h"



int main()
{
	ProfileReset();

	int iCnt;

	for(iCnt=0; iCnt<10; ++iCnt)
	{
		cMYPROFILE cTest(_T("1��"));
		Sleep(1000);
	}

	for (iCnt = 0; iCnt < 10; ++iCnt)
	{
		cMYPROFILE cTest(_T("4��"));
		Sleep(4000);
	}

	for (iCnt = 0; iCnt < 10; ++iCnt)
	{
		cMYPROFILE cTest(_T("2��"));
		Sleep(2000);
	}

	for (iCnt = 0; iCnt < 10; ++iCnt)
	{
		cMYPROFILE cTest(_T("5��"));
		Sleep(5000);
	}

	ProfileDataOutText(_T("BBB.txt"));
	return 0;
}