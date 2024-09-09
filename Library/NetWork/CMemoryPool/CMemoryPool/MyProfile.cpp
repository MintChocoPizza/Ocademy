

#include "MyProfile.h"


#pragma comment( lib, "Winmm.lib")


#define MAX_PROFILE_SIZE			30
#define PROFILE_USED				0B1
#define PROFILE_START_BEGIN			0B10

LARGE_INTEGER Start;
LARGE_INTEGER End;
LARGE_INTEGER Freq;


stPROFILE_SAMPLE stProfile[MAX_PROFILE_SIZE];


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 하나의 함수 Profiling 시작, 끝 함수
// 
// Parameters: (char *)Profiling 이름
// Return: 없음
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProfileBegin(const TCHAR* szName)
{
	int iCnt;

	for (iCnt = 0; iCnt < MAX_PROFILE_SIZE; ++iCnt)
	{
		//-------------------------------------------------------
		// 예외 처리
		// 
		//-------------------------------------------------------
		// 1. 사용되지 않은 배열은 넘어간다. 
		if ((stProfile[iCnt].lFlag & PROFILE_USED) != PROFILE_USED)
			continue;
		// 2. 이름이 맞지 않은 경우 넘어간다. 
		if (_tcscmp(stProfile[iCnt].szName, szName) != NULL)
			continue;
		// 3. Begin 이 연속해서 2번 호출된 경우 강제로 뻑낸다.
		if ((stProfile[iCnt].lFlag & PROFILE_START_BEGIN) == PROFILE_START_BEGIN)
		{
			_tprintf_s(_T("Begin 이 연속해서 2번 호출되었습니다. \n"));
			throw;
		}


		//-------------------------------------------------------
		// 반복적으로 호출된 Profile
		// 
		//-------------------------------------------------------
		stProfile[iCnt].lFlag |= PROFILE_START_BEGIN;
		++stProfile[iCnt].iCall;
		QueryPerformanceCounter(&Start);
		stProfile[iCnt].lStartTime = Start;
		
		return;
	}






	//-------------------------------------------------------
	// 처음 호출되는 경우, 비어있는 배열을 찾아 초기화한다.
	// 
	//-------------------------------------------------------
	for (iCnt = 0; iCnt < MAX_PROFILE_SIZE; ++iCnt)
	{
		if ((stProfile[iCnt].lFlag & PROFILE_USED) == PROFILE_USED)
			continue;
		
		stProfile[iCnt].lFlag |= PROFILE_USED | PROFILE_START_BEGIN;
		_tcscpy_s(stProfile[iCnt].szName, _countof(stProfile[iCnt].szName), szName);
		++stProfile[iCnt].iCall;
		QueryPerformanceCounter(&Start);
		stProfile[iCnt].lStartTime = Start;

		return;
	}
}
void ProfileEnd(const TCHAR* szName)
{
	int iCnt;
	__int64 Time;

	QueryPerformanceCounter(&End);

	for (iCnt = 0; iCnt < MAX_PROFILE_SIZE; ++iCnt)
	{
		//-------------------------------------------------------
		// 예외 처리
		// 
		//-------------------------------------------------------
		// 1. 사용되지 않은 배열은 넘어간다. 
		if ((stProfile[iCnt].lFlag & PROFILE_USED) != PROFILE_USED)
			continue;
		// 2. 이름이 같지 않으면 넘어간다.
		if (_tcscmp(stProfile[iCnt].szName, szName) != NULL)
			continue;
		// 3. End 가 연속해서 2번 호출되는 경우 강제로 뻑낸다.
		if ((stProfile[iCnt].lFlag & PROFILE_START_BEGIN) != PROFILE_START_BEGIN)
		{
			_tprintf_s(_T("End 가 연속해서 2번 호출되었습니다. \n"));
			throw;
		}
		


		//-------------------------------------------------------
		// 측정한 시간을 저장한다.
		// 
		//-------------------------------------------------------
		stProfile[iCnt].lFlag &= ~PROFILE_START_BEGIN;
		Time = End.QuadPart - stProfile[iCnt].lStartTime.QuadPart;
		stProfile[iCnt].iTotalTime += Time;
		
		if (stProfile[iCnt].iMax[0] < Time)
		{
			stProfile[iCnt].iMax[1] = stProfile[iCnt].iMax[0];
			stProfile[iCnt].iMax[0] = Time;
		}
		else if (stProfile[iCnt].iMax[1] < Time)
		{
			stProfile[iCnt].iMax[1] = Time;
		}
		else if (stProfile[iCnt].iMin[0] > Time)
		{
			stProfile[iCnt].iMin[1] = stProfile[iCnt].iMin[0];
			stProfile[iCnt].iMin[0] = Time;
		}
		else if (stProfile[iCnt].iMin[1] > Time)
		{
			stProfile[iCnt].iMin[1] = Time;
		}

		return;
	}


	//-------------------------------------------------------
	// 예외 처리
	// 
	// 여기 코드가 발생할 수 있을까??
	// 
	// 있지도 않은 End가 동작했다.
	//-------------------------------------------------------
	_tprintf_s(_T("Begin 호출 없이 End만 호출되었습니다. \n"));
	throw;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Profiling 된 데이터를 Text 파일로 출력한다. 
// 
// Parameters: (char *)출력될 파일 이름
// Return: 없음
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProfileDataOutText(const TCHAR* szFileName)
{
	errno_t err;
	FILE* pFile;
	__int64 iFreq;
	int iCnt;

	//-------------------------------------------------------
	// 기본속도 3.70GHz
	// 
	// 1초에 37억번 연산
	// 
	// T = 1 / 3,700,000,000
	//-------------------------------------------------------






	//-------------------------------------------------------
	// 1초 -> 1000		밀리초
	// 1초 -> 1 000 000	마이크로초
	// 
	// QueryPerformanceCounter 
	// --> Freq									-> 10,000,000
	// T = 1 / 10,000,000	= 0.0000001 초
	//						= 0.0001 밀리초
	//						= 0.1 마이크로초
	//						- 100 나노초
	// 
	// --> End - Start / Freq					-> 1초
	// --> End - Start / Freq * 1 000 000		-> 1,000,000 마이크로초
	// --> End - Start / (Freq / 1 000 000)
	//-------------------------------------------------------
	iFreq = Freq.QuadPart / 1000000;


	err = _tfopen_s(&pFile, szFileName, _T("w, ccs=UNICODE"));
	if (err != 0 || pFile == NULL)
	{
		_tprintf_s(_T("파일 열기 실패 \n"));
		throw;
	}


	_ftprintf_s(pFile, _T("%s \n"), _T("-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"));
	_ftprintf_s(pFile, _T("%64s | %13s | %13s | %13s | %8s | \n"), 
		_T("Name"), 
		_T("Average"), 
		_T("Min"), 
		_T("Max"), 
		_T("Call"));

	for (iCnt = 0; iCnt < MAX_PROFILE_SIZE; ++iCnt)
	{
		if ((stProfile[iCnt].lFlag & PROFILE_USED) != PROFILE_USED)
			continue;

		long long iMin1;
		long long iMin2;

		if (stProfile[iCnt].iMin[0] == MAXLONGLONG)
			iMin1 = 0;
		else
			iMin1 = stProfile[iCnt].iMin[0];

		if (stProfile[iCnt].iMin[1] == MAXLONGLONG)
			iMin2 = 0;
		else
			iMin2 = stProfile[iCnt].iMin[1];




		_ftprintf_s(pFile, _T("%64s | %0.1lfµs | %0.1lfµs | %0.1lfµs | %I64u | \n"),
			stProfile[iCnt].szName,
		//	((double)(stProfile[iCnt].iTotalTime - stProfile[iCnt].iMin[0] - stProfile[iCnt].iMin[1] - stProfile[iCnt].iMax[0] - stProfile[iCnt].iMax[1]) / iFreq) / (stProfile[iCnt].iCall - 4),
			((double)(stProfile[iCnt].iTotalTime - iMin1 - iMin2 - stProfile[iCnt].iMax[0] - stProfile[iCnt].iMax[1]) / iFreq) / (stProfile[iCnt].iCall - 4),
			(double)(stProfile[iCnt].iMin[0]) / iFreq,
			(double)(stProfile[iCnt].iMax[0]) / iFreq,
			stProfile[iCnt].iCall - 4);
	}


	_ftprintf_s(pFile, _T("%s \n"), _T("-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"));
	

	fclose(pFile);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Profiling 된 데이터를 모두 초기화 한다. 
// 
// Parameters: 없음
// Return: 없음
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProfileReset(void)
{
	int iCnt;

	QueryPerformanceFrequency(&Freq);

	memset(stProfile, 0, sizeof(stPROFILE_SAMPLE) * MAX_PROFILE_SIZE);

	for (iCnt = 0; iCnt < MAX_PROFILE_SIZE; ++iCnt)
	{
		stProfile[iCnt].iMin[0] = MAXLONGLONG;
 		stProfile[iCnt].iMin[1] = MAXLONGLONG;
	}
}