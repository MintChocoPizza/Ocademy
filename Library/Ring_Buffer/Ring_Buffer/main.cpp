// Ring_Buffer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//



#include <Windows.h>
#include <process.h>
#include <iostream>

#include "C_Ring_Buffer.h"
#include "OSJ_Test.h"

#define MULTTHREAD_TEST					0
#define OBJ_TEST						1
#define SINGLETHREAD_TEST				2
#define DIRECT_SINGLETHREAD_TEST		3
#define DIRECT_MULTTHREAD_TEST			4

#define VERSION 4

using namespace std;

#define MAIN_DEBUG 1
#if MAIN_DEBUG == 1
extern size_t Debug_Enqueue_Full_Size;
extern size_t Debug_Enqueue_In;
extern size_t Debug_Enqueue_Out;
extern size_t Debug_Enqueue_iSize;
extern char* Debug_Enqueue_Data;


extern size_t Debug_Dequeue_Full_Size;
extern size_t Debug_Dequeue_In;
extern size_t Debug_Dequeue_Out;
extern size_t Debug_Dequeue_iSize;
extern char* Debug_Dequeue_Data;


extern size_t Debug_DirectEnqueueSize_In;
extern size_t Debug_DirectEnqueueSize_Out;

extern size_t Debug_DirectDequeueSize_In;
extern size_t Debug_DirectDequeueSize_Out;
#endif // MAIN_DEBUG == 1


constexpr int CONSOLE_WIDTH = 237;
char testString[] = "@234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345*!";
size_t len;
C_RING_BUFFER g_rb(CONSOLE_WIDTH+1);


std::queue<char> Q;
char TempBuf[10000 + 2];



unsigned _stdcall QueuePush(void* pArg);
unsigned _stdcall QueuePop(void* pArg);

unsigned _stdcall QueueDirectPush(void* pArg);
unsigned _stdcall QueueDirectPop(void* pArg);

int main()
{
#if VERSION == MULTTHREAD_TEST
	{
		HANDLE hThread[2];
		DWORD dwStatus;

		len = strlen(testString);

		hThread[0] = (HANDLE)_beginthreadex(NULL, 0, QueuePush, NULL, NULL, NULL);
		hThread[1] = (HANDLE)_beginthreadex(NULL, 0, QueuePop, NULL, NULL, NULL);

		dwStatus = WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

		if (dwStatus == WAIT_OBJECT_0)
		{
			printf("ALL Thread normal shutdown !!!!! \n");
		}
		else
		{
			wprintf(L"WaitForMultipleObject %d error \n", GetLastError());

		}
	}
#elif VERSION == OBJ_TEST
	{
		OSJ_test();
	}
#elif VERSION == SINGLETHREAD_TEST
	{
		len = strlen(testString);
		while (1)
		{
			QueuePush(NULL);
			QueuePop(NULL);
		}
	}
#elif VERSION == DIRECT_SINGLETHREAD_TEST
	{
		len = strlen(testString);
		while (1)
		{
			QueueDirectPush(NULL);
			QueueDirectPop(NULL);
		}
	}
#elif VERSION == DIRECT_MULTTHREAD_TEST
	{
		HANDLE hThread[2];
		DWORD dwStatus;

		len = strlen(testString);

		hThread[0] = (HANDLE)_beginthreadex(NULL, 0, QueueDirectPush, NULL, NULL, NULL);
		hThread[1] = (HANDLE)_beginthreadex(NULL, 0, QueueDirectPop, NULL, NULL, NULL);

		dwStatus = WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

		if (dwStatus == WAIT_OBJECT_0)
		{
			printf("ALL Thread normal shutdown !!!!! \n");
		}
		else
		{
			wprintf(L"WaitForMultipleObject %d error \n", GetLastError());
		}
	}
#endif

	//printf("%d \n", g_rb.DirectEnqueueSize());



    std::cout << "Hello World!\n";
}

unsigned _stdcall QueuePush(void* pArg)
{

	int RandomByteToExtract;
	int temp;
	int index;
	size_t rbFreeSize;
	size_t qSize;
	size_t EnqueueSize;

	srand((unsigned int)GetCurrentThreadId());

#if VERSION == MULTTHREAD_TEST
	while (true)
#elif VERSION == SINGLETHREAD_TEST
#endif
	{
		memset(TempBuf, 0, sizeof(TempBuf));

		// 큐 size가 너무 크다면 한동안 Enqueue만 한다.
		if (Q.size() <= 100000)
		{
			// 큐에 문자열을 저장한다. 
			for (int i = 0; i < len; ++i)
			{
				Q.push(testString[i]);
			}
		}

		// 현재 Queue의 사이즈
		qSize = Q.size();

		// 현재 링버퍼 FreeSize
		rbFreeSize = g_rb.GetFreeSize();

		// 현재 링버퍼의 FreeSize가 0이라면 이번 루프 건너 뛴다.
		if (rbFreeSize == 0)
		{
#if VERSION == MULTTHREAD_TEST
			continue;
#elif VERSION == SINGLETHREAD_TEST
			return 0;
#endif
		}

		// 큐 사이즈와, 링버퍼 FreeSize 중 더 작은 값을 기준으로 랜덤한 값을 생성한다.
		RandomByteToExtract = rand() % (qSize > rbFreeSize ? rbFreeSize : qSize) + 1;

		// 큐에서 원하는 바이트 수 만큼 추출하여 임의의 버퍼에 집어넣는다.
		index = 0;
		temp = RandomByteToExtract;
		while (temp > 0 && !Q.empty())
		{
			TempBuf[index++] = Q.front();
			Q.pop();
			temp--;
		}

		EnqueueSize = g_rb.Enqueue(TempBuf, RandomByteToExtract);
	}

	return 0;
}

unsigned _stdcall QueuePop(void* pArg)
{
	char dequeueBuf[10002];
	int RandomByteToExtract;
	size_t rbUseSize;
	size_t DequeueSize;
	int iCnt;


	srand((unsigned int)GetCurrentThreadId());

#if VERSION == MULTTHREAD_TEST
	while (true)
#elif VERSION == SINGLETHREAD_TEST
#endif
	{
		memset(dequeueBuf, 0, sizeof(dequeueBuf));
		rbUseSize = g_rb.GetUseSize();

		// 뽑을 데이터가 없다면 넘어간다.
		if (rbUseSize == 0)
		{
#if VERSION == MULTTHREAD_TEST
			continue;
#elif VERSION == SINGLETHREAD_TEST
			return 0;
#endif
		}

		// 랜덤 최대값과 링버퍼 UseSize 중 더 작은 값을 랜덤하게 뽑는다. 
		RandomByteToExtract = rand() % (RAND_MAX > rbUseSize ? rbUseSize : RAND_MAX) + 1;

		DequeueSize = g_rb.Dequeue(dequeueBuf, RandomByteToExtract);
		if (DequeueSize > 0)
		{
			for (iCnt = 0; iCnt < DequeueSize; ++iCnt)
			{
				printf("%c", dequeueBuf[iCnt]);
			}
		}

	}

	return 0;
}

unsigned _stdcall QueueDirectPush(void* pArg)
{

	size_t qSize;
	size_t rbFreeSize;
	size_t DirectEnqueueSize;
	int RandomByteToExtract;
	int MinDataSize;
	int index;
	int temp;

	srand((unsigned)GetCurrentThreadId());

#if VERSION == DIRECT_SINGLETHREAD_TEST
#elif VERSION == DIRECT_MULTTHREAD_TEST
	while (true)
#endif
	{
		memset(TempBuf, 0, sizeof(TempBuf));

		// 큐 size가 너무 크다면 한동안 Enqueue만 한다. 
		if (Q.size() <= 100000)
		{
			// 큐에 문자열을 저장한다. 
			for (int i = 0; i < len; ++i)
			{
				Q.push(testString[i]);
			}
		}

		// 현재 Queue의 사이즈
		qSize = Q.size();

		// 현재 링버퍼 FreeSize
		rbFreeSize = g_rb.GetFreeSize();

		// 현재 링버퍼의 FreeSize가 0이라면 이번 루프 건너 뛴다.
		if (rbFreeSize == 0)
		{
#if VERSION == DIRECT_SINGLETHREAD_TEST
			return 0;
#elif VERSION == DIRECT_MULTTHREAD_TEST
			continue;
#endif
		}

		// 큐 사이즈와, 링버퍼 FreeSize 중 더 작은 값을 기준으로 랜덤한 값을 생성한다.
		RandomByteToExtract = rand() % (qSize > rbFreeSize ? rbFreeSize : qSize) + 1;

		// EnqueueDirectSize와 RandomByteToExtract 중 더 작은 값을 Q에서 뽑아서 링버퍼에 넣는다.
		// 여기서 사용하는 값과 링버퍼의 값이 
		// 멀티스레드이기 때문에 언제든지 다를 수 있다는 가정으로 그냥 만든다.
		DirectEnqueueSize = g_rb.DirectEnqueueSize();
		MinDataSize = RandomByteToExtract < DirectEnqueueSize ? RandomByteToExtract : DirectEnqueueSize;
		
		// 링버퍼에 넣기 위하여 임시 버퍼에 데이터를 저장한다. 
		index = 0;
		temp = MinDataSize;

		while (temp > 0 && !Q.empty())
		{

			TempBuf[index++] = Q.front();
			Q.pop();
			temp--;
		}
		
		memcpy(g_rb.GetInBufferPtr(), TempBuf, MinDataSize);
		
		g_rb.MoveIn(MinDataSize);
	}

	return 0;
}

unsigned _stdcall QueueDirectPop(void* pArg)
{
	char dequeueBuf[10002];
	int RandomByteToExtract;
	size_t rbDirectDequeueSize;
	size_t rbUseSize;
	int temp;
	int iCnt;

	srand((unsigned)GetCurrentThreadId());

#if VERSION == DIRECT_SINGLETHREAD_TEST
#elif VERSION == DIRECT_MULTTHREAD_TEST
	while (true)
#endif
	{
		memset(dequeueBuf, 0, sizeof(dequeueBuf));

		rbUseSize = g_rb.GetUseSize();
		if(rbUseSize == 0)
		{
#if VERSION == DIRECT_MULTTHREAD_TEST
			continue;
#elif VERSION == DIRECT_SINGLETHREAD_TEST
			return 0;
#endif
		}

		// 뽑을 데이터가 없다면 건너 뛰어야 하나? ㄴㄴ 그냥 랜덤하게, 0도 뽑을 수 있게 만들자
		rbDirectDequeueSize = g_rb.DirectDequeueSize();

		// 랜덤 최대값과 링버퍼 UseSize 중 더 작은 값을 랜덤하게 뽑는다. 
		// 자꾸 0 나와서 짜증남 .....
		RandomByteToExtract = rand() % (rbDirectDequeueSize) + 1;

		memcpy(dequeueBuf, g_rb.GetOutBufferPtr(), RandomByteToExtract);

		for (iCnt = 0; iCnt < RandomByteToExtract; ++iCnt)
		{
			printf("%c", dequeueBuf[iCnt]);
		}

		g_rb.MoveOut(RandomByteToExtract);
	}

	return 0;
}


