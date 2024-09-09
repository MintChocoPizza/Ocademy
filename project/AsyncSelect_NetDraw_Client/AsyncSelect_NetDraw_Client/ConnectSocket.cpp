
#include <stdio.h>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#include "ConnectSocket.h"

#pragma comment (lib, "Ws2_32.lib")

extern const char* DEFAULT_PORT;
extern SOCKET Connect_Socket;
extern WSADATA wsa_Data;
 char IP[256];

//--------------------------------------------------------------------
// ������ �����͸� �����Ѵ�. 
// 
// ���ο��� SOCKET �� �� ��ȯ�Ͽ� ����Ѵ�. 
//--------------------------------------------------------------------
bool connect_Server()
{
    int i_Result;
    int i_Cnt;

    //---------------------------------------------------
    // ��Ʈ�� ip�� �Է��ϼ���

    //---------------------------------------------------
    // initialize Winsock
    //WSADATA wsa_Data;

    //---------------------------------------------------
    // SetUp hints 
    struct addrinfo hints;

    //----------------------------------
    // Resolve the server address and port
    // ���� �ּ� �� ��Ʈ Ȯ��
    struct addrinfo* result = NULL;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //----------------------------------
    // Attempt to connect tot an address until one succeeds 
    // �����Ҷ� ���� �ּҿ� ������ �õ��Ѵ�.
    struct addrinfo* ptr = NULL;
    SOCKET Temp_Connect_Socket = Connect_Socket;
    //
    //----------------------------------
    // setsockopt_Linger
    struct linger Linger_Opt;
    //
    //----------------------------------
    // Connect to server

    //---------------------------------------------------
    // getaddrinfo() ���� �Ҵ���� �޸𸮸� �����Ѵ�.

    // ���� ���� �Ϸ�
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //---------------------------------------------------
    // �ͺ��ŷ �������� ��ȯ
    u_long on;

    //---------------------------------------------------
    // initialize Winsock
    i_Result = WSAStartup(MAKEWORD(2, 2), &wsa_Data);
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
    // ���� �ּ� �� ��Ʈ Ȯ��
    i_Result = getaddrinfo(IP, DEFAULT_PORT, &hints, &result);
    if (i_Result != 0)
    {
        printf_s("getadrinfo failed with error: %d \n", i_Result);
        WSACleanup();
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //----------------------------------
    // Attempt to connect tot an address until one succeeds 
    // �����Ҷ� ���� �ּҿ� ������ �õ��Ѵ�.
    // 1. ���� ����
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

        ////////////////////////////////////////////////////////////////////////////
        // int main() {
        // WSADATA wsaData;
        // WSAStartup(MAKEWORD(2, 2), &wsaData);
        //
        // HWND hwnd = CreateWindowA("STATIC", "AsyncSocket", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
        //
        // SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        //
        // WSAAsyncSelect(sock, hwnd, WM_SOCKET, FD_CONNECT);
        //
        // struct sockaddr_in serverAddr;
        // serverAddr.sin_family = AF_INET;
        // serverAddr.sin_port = htons(80); // Example port, change as needed
        // serverAddr.sin_addr.s_addr = inet_addr("93.184.216.34"); /
        ////////////////////////////////////////////////////////////////////////////


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
    // getaddrinfo() ���� �Ҵ���� �޸𸮸� �����Ѵ�.
    freeaddrinfo(result);

    if (Temp_Connect_Socket == INVALID_SOCKET)
    {
        // Connect_Socket == INVALID_SOCKET �� ���� ������ closesocket(Connect_Socket) �Ǿ��ִ�.
        printf_s("Unable to connect to server \n");
        WSACleanup();
        return false;
    }

    // ���� ���� �Ϸ�
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////



    return true;
}
