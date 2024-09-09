
#include <stdio.h>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#include "ConnectSocket.h"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "3000"
#define DEFAULT_BUFF_LEN 512

//--------------------------------------------------------------------
// 소켓의 포인터를 전달한다. 
// 
// 내부에서 SOCKET 로 형 변환하여 사용한다. 
//--------------------------------------------------------------------
bool connect_Server(void* Connect_Socket, void* wsa_Data)
{
    int i_Result;
    int i_Cnt;

    //---------------------------------------------------
    // 포트와 ip를 입력하세요
    char IP[128];
    char PORT[128];

    //---------------------------------------------------
    // initialize Winsock
    //WSADATA wsa_Data;

    //---------------------------------------------------
    // SetUp hints 
    struct addrinfo hints;

    //----------------------------------
    // Resolve the server address and port
    // 서버 주소 및 포트 확인
    struct addrinfo* result = NULL;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //----------------------------------
    // Attempt to connect tot an address until one succeeds 
    // 성공할때 까지 주소에 연결을 시도한다.
    struct addrinfo* ptr = NULL;
    SOCKET Temp_Connect_Socket = *(SOCKET*)Connect_Socket;
    //
    //----------------------------------
    // setsockopt_Linger
    struct linger Linger_Opt;
    //
    //----------------------------------
    // Connect to server

    //---------------------------------------------------
    // getaddrinfo() 에서 할당받은 메모리를 해지한다.

    // 소켓 연결 완료
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //---------------------------------------------------
    // 넌블로킹 소켓으로 전환
    u_long on;

    //---------------------------------------------------
    // 포트와 ip를 입력하세요
    printf_s("접속할 IP 주소를 입력하세요(defult 127.0.0.1) :");
    // scanf_s("%s", &IP, (unsigned)_countof(IP)); // 그냥 엔터를 누르면 넘어가지 않는다. 
    fgets(IP, sizeof(IP), stdin);
    if (IP[0] == '\n')
        strcpy_s(IP, DEFAULT_IP);
    else
    {
        // 키보드로 주소를 입력한 경우 '\n' 문자를 제거한다.
        i_Cnt = 0;
        while (IP[i_Cnt] != '\n')
            ++i_Cnt;
        IP[i_Cnt] = '\0';
    }
    printf_s("port 를 입력하세요(defult 3000) :");
    // scanf_s("%s", &PORT, (unsigned)_countof(PORT)); // 그냥 엔터를 누르면 넘어가지 않는다. 
    fgets(PORT, sizeof(PORT), stdin);
    if (PORT[0] == '\n')
        strcpy_s(PORT, DEFAULT_PORT);
    else
    {
        // 키보드로 포트를 입력한 경우 '\n 문자를 제거한다. 
        i_Cnt = 0;
        while (PORT[i_Cnt] != '\n')
            ++i_Cnt;
        PORT[i_Cnt] = '\0';
    }

    //---------------------------------------------------
    // initialize Winsock
    i_Result = WSAStartup(MAKEWORD(2, 2), (WSADATA*)wsa_Data);
    if (i_Result != 0)
    {
        printf_s("WSAStartup failed with error: %d \n", i_Result);
        return false;
    }

    //---------------------------------------------------
    // SetUp hints 
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    //----------------------------------
    // Resolve the server address and port
    // 서버 주소 및 포트 확인
    i_Result = getaddrinfo(IP, PORT, &hints, &result);
    if (i_Result != 0)
    {
        printf_s("getadrinfo failed with error: %d \n", i_Result);
        WSACleanup();
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //----------------------------------
    // Attempt to connect tot an address until one succeeds 
    // 성공할때 까지 주소에 연결을 시도한다.
    // 1. 서버 접속
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        Temp_Connect_Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (Temp_Connect_Socket == INVALID_SOCKET)
        {
            printf_s("socket failed with error: %ld \n", WSAGetLastError());
            WSACleanup();
            return false;
        }

        //----------------------------------
        // setsockopt_Linger
        Linger_Opt.l_onoff = 1;
        Linger_Opt.l_linger = 0;
        i_Result = setsockopt(Temp_Connect_Socket, SOL_SOCKET, SO_LINGER, (char*)&Linger_Opt, sizeof(Linger_Opt));
        if (i_Result == SOCKET_ERROR)
        {
            printf_s("setsockopt failed with error: %ld \n", WSAGetLastError());
            closesocket(Temp_Connect_Socket);
            WSACleanup();
            return false;
        }

        //----------------------------------
        // Connect to server
        i_Result = connect(Temp_Connect_Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (i_Result == SOCKET_ERROR)
        {
            closesocket(Temp_Connect_Socket);
            Temp_Connect_Socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    //---------------------------------------------------
    // getaddrinfo() 에서 할당받은 메모리를 해지한다.
    freeaddrinfo(result);

    if (Temp_Connect_Socket == INVALID_SOCKET)
    {
        // Connect_Socket == INVALID_SOCKET 인 경우는 무조건 closesocket(Connect_Socket) 되어있다.
        printf_s("Unable to connect to server \n");
        WSACleanup();
        return false;
    }

    // 소켓 연결 완료
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////


    //---------------------------------------------------
    // 넌블로킹 소켓으로 전환
    on = 1;
    i_Result = ioctlsocket(Temp_Connect_Socket, FIONBIO, &on);
    if (i_Result == SOCKET_ERROR)
    {
        printf_s("ioctlsocket failed with error: %ld \n", WSAGetLastError());
        WSACleanup();
        return false;
    }

    *(SOCKET*)Connect_Socket = Temp_Connect_Socket;

    return true;
}
