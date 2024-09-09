// hw10_new_Overloding.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <map>
#include <string>

#include "MyNew.h"

//#define  new    new(__FILE__, __LINE__)
//void* operator new(size_t sz, char* filename, int line)
//{
//
//}
 

//--------------------------
// 실행 예제 코드
//--------------------------

 
//class Test
//{
//protected:
//    int a;
//    int b;
//    int c;
//public:
//    Test()
//    {
//        a = 0;
//        b = 0;
//        c = 0;
//        std::cout << "Test에서 메모리 할당 \n";
//    }
//    ~Test()
//    {
//        a = 1;
//        b = 1;
//        c = 1;
//        std::cout << "Test에서 메모리 해제 \n";
//    }
//};
//
//
//int main()
//{
//    //// 메모리 해제
//    //int* a = new int;
//    //int* b = new int[5];
//
//    //// 메모리 헤제 X
//    //// log에 ARRAY로 기록이 남음
//    //int* c = new int;
//    //int* d = new int[5];
//
//    //// 잘못된 메모리 해제
//    //int* e = new int;
//    //int* f = new int[5];
//
//    //// 메모리 해제
//    //Test* aTest = new Test;
//    //Test* bTest = new Test[5];
//
//    //// 메모리 해제 X
//    //Test* cTest = new Test;
//    //Test* dTest = new Test[5];
//
//    //// 잘못된 메모리 해제
//    //Test* eTest = new Test;
//    //Test* fTest = new Test[5];
//
//
//    //// 정상 해제
//    //delete a;
//    //delete[] b;
//
//    //// 정상 해제
//    //delete aTest;
//    //delete[] bTest;
//
//
//    //delete[] e;
//    //delete f;
//
//    //// 무한 메모리 해제 시도함
//    //// delete[] eTest;
//    //delete fTest;
//
//
//
//    return 0;
//}





class aaa
{
protected:
	int a;

public:
	aaa()
	{
		a = 1;
		std::cout << "aaa 생성자" << std::endl;
	}
	virtual ~aaa()
	{
		// a = 0;
		std::cout << "aaa 소멸자" << std::endl;
	}
};

class bbb : public aaa
{
protected:
	int b;

public:
	bbb()
	{
		b = 2;
		std::cout << "bbb 생성자" << std::endl;
	}
	~bbb()
	{
		//b = 0;
		std::cout << "bbb 소멸자" << std::endl;
	}

	void getB(void)
	{
		std::cout << this << std::endl;
	}
};

int main()
{
	setLog(__FILE__, __LINE__);
	bbb* b = new bbb[2];

	delete b;

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
