// StackExtendTest.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <stdio.h>
#include <Windows.h>

#define kSTACKSIZE (1024) * (1024)
#define kPAGESIZE (4) * (1024)

int main()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(&si, &mbi, sizeof(mbi));
    // 스택을 1MB 최하단 주소를 구함.
    UINT_PTR pStackEnd = (UINT_PTR)mbi.AllocationBase + kSTACKSIZE;
    BYTE* pGuard = nullptr;

    while (true)
    {
        UINT_PTR pPosition = (UINT_PTR)mbi.AllocationBase;
        while (pPosition < pStackEnd)
        {
            SIZE_T size = VirtualQuery((LPCVOID)pPosition, &mbi, sizeof(mbi));
            if (!size) __debugbreak();

            printf("addr : %x, region size : %d KBytes\n", pPosition, mbi.RegionSize / 1024);
            printf("Page Protection : ");
            if (mbi.Protect & PAGE_READWRITE)
                printf("PAGE_READWRITE ");
            if (mbi.Protect & PAGE_GUARD)
                printf("PAGE_GUARD ");
            printf("\n");

            printf("Virtual Memory Allocation State : ");
            switch (mbi.State)
            {
            case MEM_COMMIT:
                printf("MEM_COMMIT ");
                break;
            case MEM_RESERVE:
                printf("MEM_RESERVE");
                break;
            default:
                printf("MEM_FREE");
                break;
            }
            printf("\n\n");
            pPosition += mbi.RegionSize;
        }
        printf("--------------------------------------\n");

        // 스레드 스택의 메모리상 최하단의 가드페이지의 최상단 주소를 구함
        pGuard = (BYTE*)(pPosition - mbi.RegionSize - kPAGESIZE);
        __try
        {
            // 구한 주소의 가드페이지를 건드려서 가드 속성 해제하고 가드페이지의 범위가 변경됨
            *pGuard = 1;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            if (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW)
            {
                printf("Stack OverFlow Occur");
                return 0;
            }
            else
            {
                __debugbreak();
            }
        }
    }
    std::cout << "Hello World!\n";
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
