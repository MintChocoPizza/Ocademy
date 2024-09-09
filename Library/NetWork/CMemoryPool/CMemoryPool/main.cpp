// CMemoryPool.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include <vector>

#include "MyProfile.h"
#include "CMemoryPool.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


using namespace std;
using namespace OreoPizza;

class CTEST
{
	// 64번씩 1만번
public:
	CTEST();
	~CTEST();
private:
	char a[1024];
};

CTEST::CTEST()
{
	a[0] = '\0';
}

CTEST::~CTEST() 
{

}


CTEST* Arr[100000];
CMemoryPool<CTEST> MemPool(10000, true);


int main()
{
	// 메모리 누수 발생 시 그 지점 정보를 출력하도록 한다.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);


	ProfileReset();


	for (int j = 0; j < 5; ++j)
	{
		cMYPROFILE cTest(_T("Pool"));
		for (int i = 0; i < 10000; ++i)
		{
			Arr[i] = MemPool.Alloc();
		}



		for (int i = 0; i < 10000; ++i)
		{
			MemPool.Free(Arr[i]);
		}


	}


	memset(Arr, 0, sizeof(Arr));

	for (int j = 0; j < 5; ++j)
	{
		cMYPROFILE cTest(_T("new delete"));
		for (int i = 0; i < 10000; ++i)
		{
			Arr[i] = new CTEST();
		}
		for (int i = 0; i < 10000; ++i)
		{
			delete Arr[i];
		}
	}




	ProfileDataOutText(L"AAA.txt");
	MemPool.~CMemoryPool();
	_CrtDumpMemoryLeaks();

	return 0;
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
