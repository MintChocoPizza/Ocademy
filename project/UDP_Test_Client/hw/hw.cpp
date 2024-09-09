// hw.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#include <time.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "winmm.lib")

DWORD g_Start_Time;
DWORD g_End_Time;

int wmain(int argc, wchar_t* argv[])
{
    timeBeginPeriod(1);

    WSADATA wsaData;
    SOCKET sock;
    SOCKADDR_IN serveraddr;
    SOCKADDR_IN peeraddr;
    int peeraddrLen;
    u_short Port;
    int Ret_Sendto;
    int Ret_RecvFrom;
    int Ret_setsockopt;
    int Ret_ioctl;
    wchar_t wRoom_Name[256] = L"Error";
    BYTE protocol[10] = { 0xff, 0xee, 0xdd, 0xaa, 0x00, 0x99, 0x77, 0x55, 0x33, 0x11 };
    char host[NI_MAXHOST];

    FD_SET ReadSet;
    FD_SET WriteSet;
   
    timeval Time;
    int i_Result;

    int err = NULL;


    // 윈속 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    // socket()
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) return 2;

    // 브로드캐스팅 활성화
    bool bEnable = TRUE;
    Ret_setsockopt = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable));

    // 논블로킹 소켓으로 전환
    u_long on = 1;
    Ret_ioctl = ioctlsocket(sock, FIONBIO, &on);
    if (Ret_ioctl == SOCKET_ERROR) return 1;

    // 소켓 주소 구조체 초기화
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    //serveraddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    InetPton(AF_INET, L"255.255.255.255", &serveraddr.sin_addr);

    // 순회를 돌면서 포트범위 - 10001 ~ 10099 를 탐색한다. 
    for (Port = 10001; Port <= 10099; ++Port)
    {
        // 순서대로 포트를 바인드 한다. 
        serveraddr.sin_port = htons(Port);
        Ret_Sendto = sendto(sock, (char*)protocol, sizeof(protocol), 0, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
        if (Ret_Sendto == SOCKET_ERROR)
        {
            // wprintf_s(L"sendto () Error \n");
            continue;
        }
        printf_s("send 완료 / Port:%d 탐색 \n", Port);

        //peeraddrLen = sizeof(peeraddr);
        //g_Start_Time = timeGetTime();

        //while (1)
        //{
        //    g_End_Time = timeGetTime();
        //    if (g_End_Time - g_Start_Time >= 200)
        //    {
        //        break;
        //    }

        //    Ret_RecvFrom = recvfrom(sock, (char*)wRoom_Name, sizeof(wRoom_Name), 0, (SOCKADDR*)&peeraddr, &peeraddrLen);
        //    if (Ret_RecvFrom == SOCKET_ERROR)
        //    {
        //        err = WSAGetLastError();
        //        if (err == WSAEWOULDBLOCK)
        //        {
        //            continue;
        //        }
        //        else
        //        {
        //            printf_s("err %ld \n", err);
        //            DebugBreak();
        //        }
        //    }
        //    else if (Ret_RecvFrom > 0)
        //    {
        //        printf_s("get message \n");
        //        break;
        //    }
        //}

        FD_ZERO(&ReadSet);
        FD_ZERO(&WriteSet);

        FD_SET(sock, &ReadSet);
        FD_SET(sock, &WriteSet);
        Time.tv_sec = 0;
        Time.tv_usec = 200000;
        //i_Result = select(0, 0, &WriteSet, 0, &Time);
        i_Result = select(0, &ReadSet, 0, 0, &Time);

        if (i_Result == SOCKET_ERROR)
        {
            printf_s("select error: %ld \n", WSAGetLastError());
            return 0;
        }

        printf_s("select Ret: %d \n", i_Result);
        if (i_Result > 0)
        {

            if (FD_ISSET(sock, &ReadSet))
            {
                peeraddrLen = sizeof(peeraddr);
                Ret_RecvFrom = recvfrom(sock, (char*)wRoom_Name, sizeof(wRoom_Name), 0, (SOCKADDR*)&peeraddr, &peeraddrLen);
                if (Ret_RecvFrom == SOCKET_ERROR)
                    DebugBreak();
                else if (Ret_RecvFrom == 0)
                {
                    printf_s("ret == 0 \n");
                }
            }

            //if (FD_ISSET(sock, &WriteSet))
            //{
            //    peeraddrLen = sizeof(peeraddr);
            //    Ret_RecvFrom = recvfrom(sock, (char*)wRoom_Name, sizeof(wRoom_Name), 0, (SOCKADDR*)&peeraddr, &peeraddrLen);
            //    if (Ret_RecvFrom == SOCKET_ERROR)
            //    {
            //        int err = WSAGetLastError();
            //        if (err != WSAEWOULDBLOCK)
            //        {
            //            printf_s("err %ld \n", WSAGetLastError());
            //            DebugBreak();
            //        }
            //    }
            //    else if (Ret_RecvFrom == 0)
            //    {
            //        printf_s("ret == 0 \n");
            //    }
            //}

            wprintf_s(L"%d Port serch -> \n", Port);
            if (Ret_RecvFrom > 0)
            {
                inet_ntop(AF_INET, &peeraddr.sin_addr, host, NI_MAXHOST);

                wRoom_Name[Ret_RecvFrom] = L'\0';

                wprintf_s(L"Room name: %s", wRoom_Name);
                printf_s("/ IP: % s / Port : % d \n", wRoom_Name, host, ntohs(peeraddr.sin_port));
            }
            else
            {
                printf_s("err: %ld \n", err);
            }
        }
        


        
    }


    std::cout << "Hello World!\n";
    timeEndPeriod(1);

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
