// test_Server.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#include <time.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "winmm.lib")

int main()
{
    WSADATA wsa;
    int retval;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) return 2;
    
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(10012);
    retval = bind(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) return 3;

    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[256];
    BYTE protocol[10] = { 0xff, 0xee, 0xdd, 0xaa, 0x00, 0x99, 0x77, 0x55, 0x33, 0x11 };
    const wchar_t *send_Check = L"OreoPizza";
    while (1)
    {
        addrlen = sizeof(clientaddr);
        retval = recvfrom(sock, (char*)buf, sizeof(buf), 0, (SOCKADDR*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR)
        {
            return 4;
        }

        if (memcmp(protocol, buf, 10) == 0)
        {
            printf_s("데이터가 일치함");
        }
    
        retval = sendto(sock, (char*)send_Check, sizeof(L"OreoPizza"), 0, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
        if (retval == SOCKET_ERROR)
        {
            return 5;
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
