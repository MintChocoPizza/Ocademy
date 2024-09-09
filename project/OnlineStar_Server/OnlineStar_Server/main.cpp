// OnlineStar_Server.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <map>
#include <list>
#include <set>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#include "Init_Winsock_Server.h"
#include "Console.h"
#include "Buffer.h"

#pragma comment(lib, "Ws2_32.lib")

//#define dfSCREEN_WIDTH 80
//#define dfSCREEN_HEIGHT 23

using namespace std;

struct st_PACKET
{
    int _Type;
    int _ID;
    int _X;
    int _Y;
};

struct st_PLAYER
{
    sockaddr_in Socket_Addr;
    int ID;
    int X;
    int Y;
    //SOCKET Socket;
};

map<SOCKET, st_PLAYER> Player_List;
set<SOCKET> set_Delete_List;
int g_ID = 0;

//--------------------------------------------------------------------
// 1. accept
// 2. 새로운 플레이어 생성 및 초기화
// 3. ID할당, 메시지 생성
// 4. sendunicast()
// 5. 플레이어 리스트에 등록
// 6. 플레이어 별 생성 메시지 
// 7. sendBroadcast() 
//--------------------------------------------------------------------
bool new_Player_Connect(SOCKET* Listen_Socket);

//--------------------------------------------------------------------
// Connect_Socket 에 message를 보낸다.
//  
//--------------------------------------------------------------------
bool send_Unicast(SOCKET* Client_Socket, char* message, int Message_Size);

//--------------------------------------------------------------------
// 모두에게 message를 보낸다.
//  
//--------------------------------------------------------------------
void send_Broadcast(char* message, int Message_Size);

//--------------------------------------------------------------------
// 메시지를 받는다.
//   
//--------------------------------------------------------------------
void recv_Procedure(const SOCKET* Client_Socket);

//--------------------------------------------------------------------
// 플레이어를 삭제한다.
//--------------------------------------------------------------------
void DisConnect();

int main()
{
    //---------------------------------------------------
    // return 값 저장 변수 
    int Ret_select;
    bool Is_Init_Listen_Socket;
    

    //---------------------------------------------------
    //
    WSADATA wsa_Data;
    FD_SET Read_Set;
    map<SOCKET, st_PLAYER>::iterator iter_Player_List;
    // TIMEVAL Time_Val;
    SOCKET Listen_Socket = INVALID_SOCKET;
    SOCKET Client_Socket;
    int Y;
    int X;


    cs_Initial();
    cs_ClearScreen();

    Is_Init_Listen_Socket = init_Listen_Socket(&Listen_Socket, &wsa_Data);
    if (Is_Init_Listen_Socket == false)
    {
        printf_s("소켓 초기화를 실패했습니다. \n");
        return -1;
    }

    while (1)
    {
        // 네트워크 
        FD_ZERO(&Read_Set);
        FD_SET(Listen_Socket, &Read_Set);
        for (iter_Player_List = Player_List.begin(); iter_Player_List != Player_List.end(); ++iter_Player_List)
        {
            FD_SET((*iter_Player_List).first, &Read_Set);
        }
        Ret_select = select(0, &Read_Set, 0, 0, NULL);
        if (Ret_select > 0)     // 사실 time_val == NULL 이라 무조건 무조건 처리할게 있다.
        {
            //---------------------------------------------------
            // 신규 접속 처리
            if (FD_ISSET(Listen_Socket, &Read_Set))
            {
                if (new_Player_Connect(&Listen_Socket) == false)
                {
                    // 유저 한명이 접속 못했으면 그냥 못한거다. 
                    // 다른 행동을 하지 않는다.
                }
            }

            //---------------------------------------------------
            // 메시지 처리
            // 메시지를 받아서 모든 유저에게? 나를 제외한 모두에게? 전송한다.
            for (iter_Player_List = Player_List.begin(); iter_Player_List != Player_List.end(); ++iter_Player_List)
            {
                if (FD_ISSET((*iter_Player_List).first, &Read_Set))
                {
                    // 수신에 실패하면 플레이어를 삭제한다.
                    recv_Procedure(&(*iter_Player_List).first);
                }
            }
            
            //---------------------------------------------------
            // 삭제해야할 Player를 한번에 모아서 삭제한다. 
            DisConnect();

        }


        // 로직 -> 현재 서버는 따로 연산하는 값이 없다.

        //---------------------------------------------------
        // 랜더는 어떻게 할까?
        // 그냥 출력
        buff_Buffer_Clear();
        for (iter_Player_List = Player_List.begin(); iter_Player_List != Player_List.end(); ++iter_Player_List)
        {
            Y = (*iter_Player_List).second.Y + 1;
            X = (*iter_Player_List).second.X;

            buff_Sprite_Draw(Y, X, '*');
        }
        buff_Buffer_Flip();

    }


    //---------------------------------------------------
    // Listen_Socket을 해지한다. 
    closesocket(Listen_Socket);

    //---------------------------------------------------
    // WinSock 종료
    WSACleanup();

    std::cout << "Hello World!\n";
}


//--------------------------------------------------------------------
// 1. accept
// 2. 새로운 플레이어 생성 및 초기화
// 3. ID할당, 메시지 생성
// 4. sendunicast()
// 5. 플레이어 리스트에 등록
// 6. 플레이어 별 생성 메시지 
// 7. sendBroadcast() 
// 
// 여기서 오류가 발생하면 모든 행동을 초기화 하고, 없던일로 한다.
//--------------------------------------------------------------------
bool new_Player_Connect(SOCKET* Listen_Socket)
{
    map<SOCKET, st_PLAYER>::iterator iter_Player_List;
    st_PLAYER st_New_Player;
    st_PACKET st_Message;
    char c_Message[sizeof(st_PACKET)];
    sockaddr_in Client_Addr;
    SOCKET Client_Socket;
    u_long on;
    linger Linger_Opt;
    int Client_Addr_Len = sizeof(Client_Addr);


    //---------------------------------------------------
    // 반환값 저장
    int Ret_ioctlsocket;
    int Ret_setsockopt;


     
    
    //---------------------------------------------------
    // 나는 Client_Socket을 논블러킹 소켓으로 만든적이 없다 == 아마 상속되는거 같음 ?? 실제로 확인하기
    // accept로 연결에 실패하면 그냥 연결을 안하면 된다. 
    Client_Socket = accept(*Listen_Socket, (sockaddr*)&Client_Addr, &Client_Addr_Len);
    if (Client_Socket == INVALID_SOCKET)
    {
        if (Client_Socket != WSAEWOULDBLOCK)
        {
            printf_s("accept failed with error: %d \n", WSAGetLastError());
            return false;
        }
    }

    //---------------------------------------------------
    // 넌블로킹 소켓으로 전환
    on = 1;
    Ret_ioctlsocket = ioctlsocket(Client_Socket, FIONBIO, &on);
    if (Ret_ioctlsocket == SOCKET_ERROR)
    {
        printf_s("ioctlsocket failed with error: %ld \n", WSAGetLastError());
        closesocket(Client_Socket);
        return false;
    }

    //---------------------------------------------------
    // setsockopt_Linger
    Linger_Opt.l_onoff = 1;
    Linger_Opt.l_linger = 0;
    Ret_setsockopt = setsockopt(Client_Socket, SOL_SOCKET, SO_LINGER, (char*)&Linger_Opt, sizeof(Linger_Opt));
    if (Ret_setsockopt == SOCKET_ERROR)
    {
        printf_s("setsockopt failed with error: %ld \n", WSAGetLastError());
        closesocket(Client_Socket);
        return false;
    }


    st_New_Player.ID = ++g_ID;
    st_New_Player.Y = dfSCREEN_HEIGHT / 2;
    st_New_Player.X = dfSCREEN_WIDTH / 2;
    st_New_Player.Socket_Addr = Client_Addr;

    //---------------------------------------------------
    // 새로운 플레이어에게 ID 전송
    st_Message._Type = 0;
    st_Message._ID = g_ID;
    memcpy_s(c_Message, sizeof(st_Message), &st_Message, sizeof(st_Message));
    if (send_Unicast(&Client_Socket, c_Message, sizeof(st_Message)) == false)
    {
        return false;
    }

    //---------------------------------------------------
    // 새로운 플레이어에게 기존 플레이어들의 별 생성 메시지를 보낸다.
    for (iter_Player_List = Player_List.begin(); iter_Player_List != Player_List.end(); ++iter_Player_List)
    {
        st_Message._Type = 1;
        st_Message._ID = (*iter_Player_List).second.ID;
        st_Message._X = (*iter_Player_List).second.X;
        st_Message._Y = (*iter_Player_List).second.Y;
        memcpy_s(c_Message, sizeof(st_Message), &st_Message, sizeof(st_Message));
        send_Unicast(&Client_Socket, c_Message, sizeof(st_Message));
    }


    Player_List.insert(pair<SOCKET, st_PLAYER>(Client_Socket, st_New_Player));

    // 모든 플레이어에게 새로운 별 생성 메시지를 보낸다. 
    st_Message._Type = 1;
    st_Message._ID = st_New_Player.ID;
    st_Message._Y = st_New_Player.Y;
    st_Message._X = st_New_Player.X;
    memcpy_s(c_Message, sizeof(st_Message), &st_Message, sizeof(st_Message));
    send_Broadcast(c_Message, sizeof(st_Message));


    return true;
}

//--------------------------------------------------------------------
// Connect_Socket 에 message를 보낸다.
//  
//--------------------------------------------------------------------
bool send_Unicast(SOCKET* Client_Socket, char* message, int Message_Size)
{
    FD_SET Write_Set;
    TIMEVAL Time_Val;

    //---------------------------------------------------
    // 반환값 저장
    int Ret_select;
    int Ret_send;
    int Ret_WSAGetLastError;


    Time_Val.tv_sec = 0;
    Time_Val.tv_usec = 0;
    FD_ZERO(&Write_Set);
    FD_SET(*Client_Socket, &Write_Set);

    Ret_select = select(0, 0, &Write_Set, 0, &Time_Val);

    if (Ret_select > 0)
    {
        if (FD_ISSET(*Client_Socket, &Write_Set))
        {
            Ret_send = send(*Client_Socket, message, Message_Size, 0);
            if (Ret_send == SOCKET_ERROR)
            {
                Ret_WSAGetLastError = WSAGetLastError();

                //---------------------------------------------------
                // 소켓 해지는 여기서 안한다.
                // list에 한번에 모아서, 한번에 제거한다.
                if (Ret_WSAGetLastError == WSAEWOULDBLOCK)
                {
                    printf_s("send_Unicast failed with error: %d\n", WSAGetLastError());
                    set_Delete_List.insert(*Client_Socket);
                    return false;
                }
                else
                {
                    printf_s("send_Unicast failed with error: %d\n", WSAGetLastError());
                    set_Delete_List.insert(*Client_Socket);
                    throw;
                }
            }
        }
    }
    return true;
}

//--------------------------------------------------------------------
// 모두에게 message를 보낸다.
//  
//--------------------------------------------------------------------
void send_Broadcast(char* message, int Message_Size)
{
    FD_SET Write_Set;
    st_PACKET st_Message;
    char c_Message[sizeof(st_PACKET)];
    TIMEVAL Time_Val;
    map<SOCKET, st_PLAYER>::iterator iter_Player_List;
    int i_Temp_ID;

    //---------------------------------------------------
    // 반환값 저장
    int Ret_select;
    int Ret_send;
    int Ret_WSAGetLastError;

    Time_Val.tv_sec = 0;
    Time_Val.tv_usec = 0;
    FD_ZERO(&Write_Set);

    //---------------------------------------------------
    // Write_Set에 플레이어들의 소켓을 담는다.
    for (auto iter = Player_List.begin(); iter != Player_List.end(); ++iter)
    {
        FD_SET((*iter).first, &Write_Set);
    }

    Ret_select = select(0, 0, &Write_Set, 0, &Time_Val);
    if (Ret_select > 0)
    {
        for (iter_Player_List = Player_List.begin(); iter_Player_List != Player_List.end(); ++iter_Player_List)
        {
            if (FD_ISSET((*iter_Player_List).first, &Write_Set))
            {
                Ret_send = send((*iter_Player_List).first, message, Message_Size, 0);
                if (Ret_send == SOCKET_ERROR)
                {
                    Ret_WSAGetLastError = WSAGetLastError();
                    if (Ret_WSAGetLastError == WSAEWOULDBLOCK)
                    {
                        printf_s("send_Broadcast failed with error: %d\n", WSAGetLastError());
                        // 보낼 수 없다면 연결을 끊어버릴거야
                        set_Delete_List.insert((*iter_Player_List).first);
                    }
                    else if (Ret_WSAGetLastError == 10054)
                    {
                        ////---------------------------------------------------
                        //// rst를 전송하는 강제종료이다. 
                        //// 1. 리스트에서 삭제 
                        //// 2. 삭제 메시지 브로드 캐스트로 뿌려주기
                        //st_Message._Type = 2;
                        //st_Message._ID = (*iter_Player_List).second.ID;
                        //memcpy_s(c_Message, sizeof(st_Message), &st_Message, sizeof(st_Message));
                        //send_Broadcast(c_Message, sizeof(st_Message));
                        //Player_List.erase(iter_Player_List);;
                    
                        //---------------------------------------------------
                        // rst가 도착하면 closesocket을 할 필요가 없는가?
                        // 7계측 차원에서 소켓에 대한 리소스를 해지하여야 한다.
                        printf_s("send_Broadcast failed with error: %d\n", WSAGetLastError());
                        set_Delete_List.insert((*iter_Player_List).first);

                    }
                    else if (Ret_WSAGetLastError == 10053)
                    {
                        printf_s("recv failed with error: %d\n", WSAGetLastError());
                        set_Delete_List.insert((*iter_Player_List).first);
                    }
                    else
                    {
                        printf_s("send_Broadcast failed with error: %d\n", WSAGetLastError());
                        // 보낼 수 없다면 연결을 끊어버릴거야
                        set_Delete_List.insert((*iter_Player_List).first);
                        throw;
                    }
                }
            }
            else
            {
                // 보낼 수 없다면 연결을 끊어버릴거야
                set_Delete_List.insert((*iter_Player_List).first);
            }
        }
    }
}

//--------------------------------------------------------------------
// 메시지를 받는다.
//   
//--------------------------------------------------------------------
void recv_Procedure(const SOCKET* Client_Socket)
{
    char c_Buffer[1024];
    st_PACKET st_Packet;
    map<SOCKET, st_PLAYER>::iterator iter_Player_List;
    int i_Cnt;

    //---------------------------------------------------
    // 반환값 저장
    int Ret_recv;
    int Ret_WSAGetLastError;


    // 메시지는 끊어져서 오지 않는다고 가정
    Ret_recv = recv(*Client_Socket, c_Buffer, sizeof(c_Buffer), 0);
    
    if (Ret_recv == 0)
    {
        set_Delete_List.insert(*Client_Socket);
    }
    else if (Ret_recv == -1)
    {
        Ret_WSAGetLastError = WSAGetLastError();
        if (Ret_WSAGetLastError == 10054)
        {
            printf_s("recv failed with error: %d\n", WSAGetLastError());
            set_Delete_List.insert(*Client_Socket);
        }
        else if (Ret_WSAGetLastError == 10053)
        {
            printf_s("recv failed with error: %d\n", WSAGetLastError());
            set_Delete_List.insert(*Client_Socket);
        }
        else if (Ret_WSAGetLastError == WSAEWOULDBLOCK)
        {
            printf_s("recv failed with error: %d\n", WSAGetLastError());
            set_Delete_List.insert(*Client_Socket);
        }
        else
        {
            printf_s("recv failed with error: %d\n", WSAGetLastError());
            set_Delete_List.insert(*Client_Socket);
            throw;
        }
    }
    else
    {
        // 정상적인 경우
        for (i_Cnt = 0; i_Cnt < Ret_recv / sizeof(st_PACKET); i_Cnt += sizeof(st_PACKET))
        {
            memcpy_s(&st_Packet, sizeof(st_Packet), c_Buffer + i_Cnt, sizeof(st_Packet));

            //---------------------------------------------------
            // 이상한 패킷이면 연결을 끝낸다.
            if (st_Packet._Type < 0 || 3 < st_Packet._Type)
            {
                set_Delete_List.insert(*Client_Socket);
                return;
            }
            if(st_Packet._ID > g_ID)
            {
                set_Delete_List.insert(*Client_Socket);
                return;
            }
            if (st_Packet._X < 0 || st_Packet._Y < 0 || dfSCREEN_WIDTH-2 < st_Packet._X || dfSCREEN_HEIGHT-2 < st_Packet._Y)
            {
                set_Delete_List.insert(*Client_Socket);
                return;
            }

            iter_Player_List = Player_List.find(*Client_Socket);
            (*iter_Player_List).second.Y = st_Packet._Y;
            (*iter_Player_List).second.X = st_Packet._X;
            send_Broadcast(c_Buffer + i_Cnt, sizeof(st_PACKET));
           

        }
    }
}

//--------------------------------------------------------------------
// 플레이어를 삭제한다.
//--------------------------------------------------------------------
void DisConnect()
{
    st_PACKET st_Message;
    char c_Message[sizeof(st_Message)];
    map<SOCKET, st_PLAYER>::iterator iter_Player_List;
    set<SOCKET>::iterator iter_set_Delete_List;
    SOCKET Temp_Socket;

    st_Message._Type = 2;
    
    /////////////////////////////////////////////////////////////////////////////////////
    // 구조가 잘못되었다. 
    // 
    // 삭제 메시지 송신과 리스트에서 삭제를 동시에 하는 것이 아닌 
    // 
    // 삭제 메시지를 보내면서 그 때 바로 삭제 리스트를 전부 업데이트 해야한다. 
    // 
    // 그리고 전부 돌리고 나서
    // 
    // 삭제 메시지 송신이 끝난 다음에
    // 
    // 최종적으로 리스트를 전부 삭제해야한다.
    // 
    /////////////////////////////////////////////////////////////////////////////////////
    //---------------------------------------------------
    // set_Delete_List가 빌때까지 전부 삭제한다.
    while(!set_Delete_List.empty())
    {
        //---------------------------------------------------
        // 1. iter을 뽑는다. 
        // 2. iter에서 SOCKET를 뽑는다. 
        // 3. SOCKET을 Player_List에서 찾는다. 
        // 4. SOCKET에 대한 ID를 찾는다.
        // 5. Player_List에서 SOCKET를 삭제한다.
        // 6. 메시지를 만든다. 
        // 7. 삭제 메시지를 브로드캐스트로 보낸다. 
        // 8. 
        iter_set_Delete_List = set_Delete_List.begin();
        Temp_Socket = *iter_set_Delete_List;
        
        iter_Player_List = Player_List.find(Temp_Socket);
        if (iter_Player_List == Player_List.end())
            continue;
        st_Message._ID = (*iter_Player_List).second.ID;
        Player_List.erase(iter_Player_List);
        memcpy_s(c_Message, sizeof(st_Message), &st_Message, sizeof(st_Message));
        send_Broadcast(c_Message, sizeof(st_Message));

        set_Delete_List.erase(iter_set_Delete_List);

        closesocket(Temp_Socket);
    }
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

