
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// 성능을 우려하여 RPC 붙히지 않음. 
// 추후 성능이 괜찮게 나오면 수정함.
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <time.h>
#include <conio.h>
#include <string>

#include <crtdbg.h>

#include <unordered_map>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>


#include "C_Logger.h"
#include "LOG.h"
#include "C_Ring_Buffer.h"
#include "SerializeBuffer.h"
#include "Disconnect.h"
#include "Contents.h"
#include "Session.h"
#include "Player.h"
#include "CCrashDump.h"

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "Ws2_32.lib")

#define _CRTDBG_MAP_ALLOC
#define TICK_PER_FRAME 40

//LONG OreoPizza::CCrashDump::_DumpCount = 0;
//OreoPizza::CCrashDump crashDump;

//---------------------------------
// 시간 측정 변수
//---------------------------------
DWORD g_Start_Time;
DWORD g_One_Second;
DWORD g_dwFirst;

//---------------------------------
// 프로그램을 종료시키는 변수
//---------------------------------
bool g_bShutdown = false;

//---------------------------------
// 직렬화 버퍼, 
// 내부에 동적할당이 존재하여
// 전역으로 선언함
//---------------------------------
SerializeBuffer g_Packet;


DWORD dwUpdateFPS;

//---------------------------------------
// 디버깅용, 무한히 증가한다. 
DWORD dwDebug_Loop = 0;

void LoadData(void);
void ServerControl(void);
void Monitor(void);

int wmain(int argc, wchar_t* argv[])
{
    DWORD dwCurTime;
    //DWORD dwUpdateFPS;
    DWORD dwNetFPS;
    DWORD dwCheckFPS;


    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    timeBeginPeriod(1);
    srand((unsigned int)time(NULL));
    InitLogger(L"LOG");

    dwUpdateFPS = 0;
    dwNetFPS = 0;
    dwDebug_Loop = 0;

    // 초기 시간을 세팅한다.
    g_Start_Time = timeGetTime();
    dwCheckFPS = g_Start_Time;
    g_dwFirst = g_Start_Time;
    g_dwPreTime = g_Start_Time;

    LoadData();                          // 설정 및 게임데이터, DB 데이터 로딩
    netStartUp();                           // 네트워크 초기화, 리슨소켓 새엇ㅇ 및 listen  But Sesstion Class 의 싱글톤 생성자가 역할을 당담한다.

    while (!g_bShutdown)                               // 서버 메인 루프, 전역의 g_bShutdown 값에 의해 종료 결정   
    {

        // 네트워크 처리 함수()             // 네트워크 송수신 처리
        netIOProcess();
        ++dwNetFPS;

        // 업데이트는 게임의 로직 (월드, 캐릭터, 몬스터, 이벤트 등...)
        // 실제 게임에서 돌아가야 하는 로직을 처리 한다. 
        dwCurTime = timeGetTime();
        if (dwCurTime - g_Start_Time >= TICK_PER_FRAME)
        {
            netHeartbeat(dwCurTime);

            Update();                           // 게임 로직 업데이트 
            //g_Start_Time += 40;
            g_Start_Time = dwCurTime - ((dwCurTime - g_dwFirst) % TICK_PER_FRAME);
            ++dwUpdateFPS;
            dwDebug_Loop++;
        }

        if (dwCurTime - dwCheckFPS >= 1000)
        {
            dwCheckFPS += 1000;
            _LOG(dfLOG_LEVEL_SYSTEM, L"NetFPS:%d / UpdateFPS:%d", dwNetFPS, dwUpdateFPS);

            dwNetFPS = 0;
            dwUpdateFPS = 0;

            ServerControl();                    // 키보드 입력을 통해서 서버를 제어할 경우 사용 
        }


        Monitor();                          // 모니터링 정보를 표시, 저장, 전송하는 경우 사용
    }

    // 서버의 종료 대기 
    // 서버는 함부로 종료 해도 안된다. 
    // DB에 저장할 데이터나 기타 마무리 할 일들이 모두 끝났는지 확인한 뒤에 꺼주어야 한다. 

    netCleanUp();                        // Sesstion class 싱글톤 소멸자가 메인함수 호출 뒤 atexit()에 의해 호출되어 처리된다. 



    std::cout << "Hello World!\n";
    return 0;
}


void LoadData(void)
{
    _LOG(dfLOG_LEVEL_SYSTEM, L"Control Mode Key : Press U......");
    SetSession();
    InitDisconnectSessionIds();
    Init_CharacterArr();
}
//void netCleanUp(void)
//{
//    atexit(ClearCharacterHash);
//}

void ServerControl(void)
{
    // 개발, 디버깅, 긴급 등의 상황에서 사용할
    // 비공식적 기능, 특정 함수 호출, 기확자용 으로 사용된다. 

    // 기본적으로는 외부, 원격 네트워크등을 통하여 서버를 컨트롤 하게 된다.

    // 키보드 컨트롤 잠금, 풀림 변수 
    static bool bControlMode = false;
    static bool bControlLog = false;
    __wchar_t ControlKey;

    //-------------------------------------------------------------------
    // L: 컨트롤 Lock  / U : 컨트롤 Unlock    / Q : 서버 종료
    //-------------------------------------------------------------------
    if (_kbhit())
    {
        ControlKey = _getwch();

        // 키보드 제어 허용 
        if (L'u' == ControlKey || L'U' == ControlKey)
        {
            bControlMode = true;

            // 관련 키 도움말 출력
            wprintf(L"Control Mode : Press Q - Quit \n");
            wprintf(L"Control Mode : Press L - Key Lock \n");
            wprintf(L"Control Mode : Press S - Log Level \n");
            wprintf(L"Control Mode : Press c - Console Clear \n");

        }

        // 키보드 제어 잠금 
        if ((L'l' == ControlKey || L'L' == ControlKey) && bControlMode)
        {
            wprintf(L"Control Lock...! press U - Control Unlock \n");

            bControlMode = false;
        }

        // 키보드 제어 풀림 상태에서 특정 기능
        if (bControlMode == true)
        {
            // 서버 종료
            if ((L'q' == ControlKey || L'Q' == ControlKey))
            {
                g_bShutdown = true;
            }

            // 로그 레벨 제어 허용
            if (L's' == ControlKey || L'S' == ControlKey)
            {
                wprintf(L"Control Log : Press 0 - LOG_LEVEL_DEBUG \n");
                wprintf(L"Control Log : Press 1 - LOG_LEVEL_ERROR \n");
                wprintf(L"Control Log : Press 2 - LOG_LEVEL_SYSTEM \n");
                bControlLog = true;
            }

            if (bControlLog == true)
            {
				if (L'0' == ControlKey)
				{
                    wprintf(L"LOG_LEVEL_DEBUG...! \n");
					g_iLogLevel = dfLOG_LEVEL_DEBUG;
					bControlLog = false;
				}
                else if (L'1' == ControlKey)
                {
                    wprintf(L"LOG_LEVEL_ERROR...! \n");
                    g_iLogLevel = dfLOG_LEVEL_ERROR;
                    bControlLog = false;
                }
                else if (L'2' == ControlKey)
                {
                    wprintf(L"LOG_LEVEL_SYSTEM...! \n");
                    g_iLogLevel = dfLOG_LEVEL_SYSTEM;
                    bControlLog = false;
                }
            }
            
            // 콘솔창 클리어
            if (L'c' == ControlKey || L'C' == ControlKey)
            {
                system("cls");
            }
           



        }
    }
}

void Monitor(void)
{
}



