// Force_RST_Transmission_Client.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <stdio.h>
#include <map>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#include "Console.h"
#include "Buffer.h"
#include "ConnectSocket.h"
#include "C_Queue.h"

#pragma comment (lib, "Ws2_32.lib")

using namespace std;
using namespace OreoPizza;

struct st_PACKET
{
    int Type;
    int ID;
    int X;
    int Y;
};

struct st_PLAYER
{
    int ID;
    int Y;
    int X;
    
    bool is_Create_Id;
    bool is_Create_Star;
};

struct st_Other_PLAYER
{
    int Y;
    int X;
};

//---------------------------------------------------
// 전역으로 선연된 소켓
SOCKET Connect_Socket = INVALID_SOCKET;
WSADATA wsa_Data;

//---------------------------------------------------
// 플레이어, 0, false 로 초기화 
st_PLAYER g_st_Player{ 0, };

//---------------------------------------------------
// 다른 플레이어
// 해야할 기능: ID에 맞는 플레이어 찾기, 순회 
map<int, st_Other_PLAYER> g_Other_Player;


int main()
{
    FD_SET Read_Set;
    FD_SET Write_Set;
    char Connection_Info[sizeof(char) * dfSCREEN_WIDTH];
    C_Queue<char> Recv_Buff(sizeof(st_PACKET) * 65535 * 8);
    st_PACKET st_Temp_Packet;
    TIMEVAL Time_Val;
    map<int, st_Other_PLAYER>::iterator iter_Other_Player;
    int i_Recv_Size;
    int i_Recv_Buff_Empty_Size = 0;
    int i_Result;
    int i_Cnt;

    int i_Cnt_Connect_Client = 0;

    int y;
    int x;

    char* c_Temp__Buff;

    bool Is_True;
    bool is_Player_Move;



    /////////////////////////////////////////////////////////////
    // 초기화
    /////////////////////////////////////////////////////////////
    cs_Initial();


    Is_True = connect_Server(&Connect_Socket, &wsa_Data);
    if (Is_True == false)
    {
        printf_s("연결을 실패했습니다. \n");
        return -1;
    }

    cs_ClearScreen();

    while (1)
    {
        //---------------------------------------------------
        // 키보드 입력
        // 키 입력을 받는 함수 
        if (g_st_Player.is_Create_Star == true)
        {
            is_Player_Move = false;
            y = g_st_Player.Y;
            x = g_st_Player.X;

            if (GetAsyncKeyState(VK_UP) & 0x8000)
            {
                y -= 1;
                is_Player_Move = true;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000)
            {
                y += 1;
                is_Player_Move = true;
            }
            if (GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                x -= 1;
                is_Player_Move = true;
            }
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                x += 1;
                is_Player_Move = true;
            }

            //---------------------------------------------------
            // 플레이어 좌표 조정
            if (y < 0) y = 0;
            if (y >= dfSCREEN_HEIGHT-1) y = dfSCREEN_HEIGHT - 1-1;
            if (x < 0) x = 0;
            if (x >= dfSCREEN_WIDTH - 1) x = dfSCREEN_WIDTH - 2;
            g_st_Player.Y = y;
            g_st_Player.X = x;



            //---------------------------------------------------
            // 좌표가 이동되었으면 좌표를 전송한다. 
            if (is_Player_Move == true)
            {
                FD_ZERO(&Write_Set);
                FD_SET(Connect_Socket, &Write_Set);
                Time_Val.tv_sec = 0;
                Time_Val.tv_usec = 0;
                i_Result = select(0, 0, &Write_Set, 0, &Time_Val);
                if (i_Result > 0)
                {
                    if (FD_ISSET(Connect_Socket, &Write_Set))
                    {
                        // st_Temp_Packet = { 3, g_st_Player.ID, x, y };
                        st_Temp_Packet.Type = 3;
                        st_Temp_Packet.ID = g_st_Player.ID;
                        st_Temp_Packet.X = x;
                        st_Temp_Packet.Y = y;
                        c_Temp__Buff = (char*)malloc(sizeof(st_Temp_Packet));
                        if (c_Temp__Buff == NULL)
                        {
                            printf_s("malloc failed with error: %d\n", WSAGetLastError());
                            closesocket(Connect_Socket);
                            WSACleanup();
                            return -1;
                        }
                        memcpy(c_Temp__Buff, &st_Temp_Packet, sizeof(st_Temp_Packet));
                        i_Result = send(Connect_Socket, c_Temp__Buff, sizeof(st_Temp_Packet), 0);
                        free(c_Temp__Buff);
                    }
                }
            }
        }
        

        //---------------------------------------------------
        // 네트워크
        FD_ZERO(&Read_Set);
        FD_SET(Connect_Socket, &Read_Set);
        Time_Val.tv_sec = 0;
        Time_Val.tv_usec = 0;
        i_Recv_Buff_Empty_Size = 0;

        i_Result = select(0, &Read_Set, 0, 0, &Time_Val);
        if (i_Result > 0)
        {
            if (FD_ISSET(Connect_Socket, &Read_Set))
            {
                i_Recv_Buff_Empty_Size = Recv_Buff._Size - Recv_Buff._Data_Size;
                c_Temp__Buff = (char*)malloc(i_Recv_Buff_Empty_Size);
                if (c_Temp__Buff == NULL)
                    return -1;

                i_Recv_Size = recv(Connect_Socket, c_Temp__Buff, i_Recv_Buff_Empty_Size, 0);
                if (i_Recv_Size == SOCKET_ERROR)
                {
                    if (WSAGetLastError() == WSAEWOULDBLOCK)
                    {

                    }
                    else
                    {
                        printf_s("send failed with error: %d\n", WSAGetLastError());
                        closesocket(Connect_Socket);
                        WSACleanup();
                        return -1;
                    }
                }

                // 임시 버퍼에 받은 데이터를 버퍼에 하나하나 저장한다. 
                for (i_Cnt = 0; i_Cnt < i_Recv_Size; ++i_Cnt)
                {
                    Recv_Buff.enqueue(c_Temp__Buff[i_Cnt]);
                }


                //---------------------------------------------------
                // 분기에 따른 행동
                // 버퍼에 있는 데이터가 패킷의 크기보다 크다면, 패킷을 읽고 실행한다. 
                i_Recv_Buff_Empty_Size = Recv_Buff._Data_Size / sizeof(st_PACKET);
                for (i_Cnt = 0; i_Cnt < i_Recv_Buff_Empty_Size; ++i_Cnt)
                {
                    // 형변환은 사실 내부에서 함수가 동작하여 변환해준다.
                    // Recv_Buff.back(((char)st_Temp_Packet), sizeof(st_PACKET));
                    if (Recv_Buff.back(&st_Temp_Packet, sizeof(st_PACKET)))
                    {
                        if (!Recv_Buff.dequeue(sizeof(st_PACKET)))
                        {
                            return -1;
                        }
                    }

                    switch (st_Temp_Packet.Type)
                    {
                    case 0:
                    {
                        // 나의 별이 할당되었다. 
                        g_st_Player.ID = st_Temp_Packet.ID;
                        g_st_Player.is_Create_Id = true;
                    }
                        break;
                    case 1:
                    {
                        // 누군가의 별이 생성되었다. ;
                        if (st_Temp_Packet.ID == g_st_Player.ID)
                        {
                            // 이건 나의 별이다. 
                            g_st_Player.X = st_Temp_Packet.X;
                            g_st_Player.Y = st_Temp_Packet.Y;
                            g_st_Player.is_Create_Star = true;
                        }
                        else
                        {
                            // 다른 사람의 별이 생성되었다. 
                            // 별의 ID는 서버가 정해주기 때문에 중복이 없다. 
                            g_Other_Player.insert(pair<int, st_Other_PLAYER>(
                                st_Temp_Packet.ID, st_Other_PLAYER{ st_Temp_Packet.Y, st_Temp_Packet.X }
                            ));
                        }
                        ++i_Cnt_Connect_Client;
                    }
                        break;
                    case 2:
                    {
                        // 별의 삭제
                        // 나의 별이 삭제되는 경우가 있나????
                        // 응 내가 안받아, 차라리 연결까지 끊으셈
                        g_Other_Player.erase(st_Temp_Packet.ID);
                        --i_Cnt_Connect_Client;
                    }
                        break;
                    case 3:
                    {
                        // 별의 이동
                        // 나에 대한 별의 이동은 나한테 보내지 않는다. 
                        iter_Other_Player = g_Other_Player.find(st_Temp_Packet.ID);
                        (*iter_Other_Player).second.Y = st_Temp_Packet.Y;
                        (*iter_Other_Player).second.X = st_Temp_Packet.X;
                    }
                        break;
                    default:
                        break;
                    }

                }


                free(c_Temp__Buff);
            }
        }
        
        //---------------------------------------------------
        // 출력 버퍼에 그리기
        buff_Buffer_Clear();

        if (g_st_Player.is_Create_Star)
            buff_Sprite_Draw(g_st_Player.Y+1, g_st_Player.X, '*');

        for (iter_Other_Player = g_Other_Player.begin(); iter_Other_Player != g_Other_Player.end(); ++iter_Other_Player)
        {
            int Y = (*iter_Other_Player).second.Y+1;
            int X = (*iter_Other_Player).second.X;
            buff_Sprite_Draw(Y, X, '*');
        }

        sprintf_s(Connection_Info, "Connect Client: %d / Packet: %d", i_Cnt_Connect_Client, i_Recv_Buff_Empty_Size);
        buff_Sprite_Draw_String(0, 0, Connection_Info, strnlen_s(Connection_Info, sizeof(char)* dfSCREEN_WIDTH));

        

        
        

        //---------------------------------------------------
        // 랜더
        buff_Buffer_Flip();

        Sleep(12);



        //if (g_st_Player.is_Create_Star == true)
        //    printf_s("Player!! ID: %d, Y: %d, X: %d \n", g_st_Player.ID, g_st_Player.Y, g_st_Player.X);

        //for (iter_Other_Player = g_Other_Player.begin(); iter_Other_Player != g_Other_Player.end(); ++iter_Other_Player)
        // {
        //     int Y = (*iter_Other_Player).second.Y+1;
        //     int X = (*iter_Other_Player).second.X;
        //     int ID = (*iter_Other_Player).first;

        //     printf_s("ID: %d, Y: %d, X: %d \n", ID, Y, X);
        // }
    }







    //---------------------------------------------------
    // 할당 받은 메모리를 해지한다.


    //---------------------------------------------------
    // Connect_Socket을 해지한다. 
    closesocket(Connect_Socket);

    //---------------------------------------------------
    // WinSock 종료
    WSACleanup();

    std::cout << "Hello World! " << std::endl;

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
