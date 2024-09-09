



#include <stdio.h>
#include <time.h>
#include <Windows.h>

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "Ws2_32.lib")

void LoadData(void);
void ServerControl(void);
void Monitor(void);

int wmain(int argc, wchar_t* argv[])
{
    timeBeginPeriod(1);
    srand((unsigned int)time(NULL));

    //// 초기 시간을 세팅한다.
    //g_Start_Time = timeGetTime();

    //LoadData();                          // 설정 및 게임데이터, DB 데이터 로딩
    //netStartUp();                           // 네트워크 초기화, 리슨소켓 새엇ㅇ 및 listen  But Sesstion Class 의 싱글톤 생성자가 역할을 당담한다.

    //while (!g_bShutdown)                               // 서버 메인 루프, 전역의 g_bShutdown 값에 의해 종료 결정   
    //{
    //    // 네트워크 처리 함수()             // 네트워크 송수신 처리
    //    netIOProcess();

    //    // 업데이트는 게임의 로직 (월드, 캐릭터, 몬스터, 이벤트 등...)
    //    // 실제 게임에서 돌아가야 하는 로직을 처리 한다. 

    //    Update();                           // 게임 로직 업데이트 

    //    ServerControl();                    // 키보드 입력을 통해서 서버를 제어할 경우 사용 
    //    Monitor();                          // 모니터링 정보를 표시, 저장, 전송하는 경우 사용
    //}

    //// 서버의 종료 대기 
    //// 서버는 함부로 종료 해도 안된다. 
    //// DB에 저장할 데이터나 기타 마무리 할 일들이 모두 끝났는지 확인한 뒤에 꺼주어야 한다. 

    //netCleanUp();                        // Sesstion class 싱글톤 소멸자가 메인함수 호출 뒤 atexit()에 의해 호출되어 처리된다. 



	wprintf(L"Hello World\n");

    return 0;
}