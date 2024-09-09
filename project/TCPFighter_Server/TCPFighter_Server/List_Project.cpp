// List_Project.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <list>

#include "CList.h"

using namespace std;

//class aaa
//{
//public:
//    aaa(int t);
//    ~aaa();
//
//    CList<aaa*> cl;
//    CList<aaa*>::iterator iter;
//
//    void ttt(void)
//    {
//        for (int i = 0; i < 10; ++i)
//        {
//            aaa* temp = new aaa(i);
//
//            cl.push_back(temp);
//        }
//
//
//
//        for (iter = cl.begin(); iter != cl.end();)
//        {
//            aaa* temp = *iter;
//
//            delete temp;
//
//            iter = cl.erase(iter);
//        }
//    }
//private:
//    int a;
//
//};
//
//aaa::aaa(int t)
//{
//    cout << "생성자" << endl;
//    a = t;
//}
//
//aaa::~aaa()
//{
//    cout << "소멸자" << endl;
//}
//
//
//int main()
//{
//    aaa t(2);
//    t.ttt();
//
//
//    std::cout << "\nHello World!\n";
//
//}

//class bbb
//{
//public:
//    bbb(int b);
//    ~bbb();
//
//private:
//    int b;
//};
//
//bbb::bbb(int b)
//{
//    this->b = b;
//    cout << "b 생성자 \n";
//}
//
//bbb::~bbb()
//{
//    cout << "b 소멸자 \n";
//}
//
//class aaa
//{
//public:
//    aaa(int t);
//    ~aaa();
//
//    CList<bbb*> cl;
//    CList<bbb*>::iterator iter;
//
//    void ttt(void)
//    {
//        for (int i = 0; i < 10; ++i)
//        {
//            bbb* temp = new bbb(10);
//            cl.push_back(temp);
//        }
//
//
//
//        for (iter = cl.begin(); iter != cl.end();)
//        {
//            bbb* temp = *iter;
//
//            delete temp;
//
//            iter = cl.erase(iter);
//        }
//    }
//private:
//    int a;
//
//};
//
//aaa::aaa(int t)
//{
//    cout << "a 생성자" << endl;
//    a = t;
//}
//
//aaa::~aaa()
//{
//    cout << "a 소멸자" << endl;
//}
//
//int main()
//{
//    aaa t(10);
//    t.ttt();
//
//    cout << "Hello World \n";
//    return 0;
//}






// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
