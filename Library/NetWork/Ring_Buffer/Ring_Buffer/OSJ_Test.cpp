
#include <time.h>
#include <stdlib.h>

#include "OSJ_Test.h"
#include "C_Ring_Buffer.h"

#define OSJ_SINGLE_DEBUG 1

constexpr int CONSOLE_WIDTH = 237;

C_RING_BUFFER rbTemp;
C_RING_BUFFER rb;
char dequeueBuf[CONSOLE_WIDTH + 2];
char peekBuf[10000 + 2];

char tempBuf[10000 + 2];
size_t g_OSJ_len;

char OSJ_testString[] = "@234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345*!";


#if OSJ_SINGLE_DEBUG == 1

size_t g_Cnt = 0;
char g_TemptestString[] = "@234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345*!";

extern size_t Debug_Enqueue_Full_Size;
extern  size_t Debug_Enqueue_In;
extern size_t Debug_Enqueue_Out;
extern size_t Debug_Enqueue_iSize;
extern char* Debug_Enqueue_Data;


extern size_t Debug_Dequeue_Full_Size;
extern size_t Debug_Dequeue_In;
extern size_t Debug_Dequeue_Out;
extern size_t Debug_Dequeue_iSize;
extern char* Debug_Dequeue_Data;

#endif // OSJ_SINGLE_DEBUG == 1


void printArray(size_t width, char* pBuf)
{
	for (int i = 0; i < width; ++i) 
	{
		printf("%c", pBuf[i]);

#if OSJ_SINGLE_DEBUG == 1
		if (g_Cnt == g_OSJ_len)
			g_Cnt = 0;

		if (pBuf[i] != OSJ_testString[g_Cnt])
		{
			__debugbreak();
		}
		++g_Cnt;

#endif // OSJ_SINGLE_DEBUG == 1
	}
}

// 큐에서 원하는 바이트 수만큼 추출하여 임의의 버퍼에 집어넣는 함수
void extractBytes(std::queue<char>& myQueue, char* buffer, int bytes)
{
	int index = 0;
	while (bytes > 0 && !myQueue.empty()) {
		buffer[index++] = myQueue.front();
		myQueue.pop();
		--bytes;
	}
}

void OSJ_test()
{
	g_OSJ_len = strlen(OSJ_testString);
	size_t sz = sizeof(OSJ_testString);
	std::queue<char> q;
	srand((unsigned)time(nullptr));

#if OSJ_SINGLE_DEBUG == 1
#endif


	while (true)
	{
		memset(peekBuf, 0, sizeof(peekBuf));
		memset(dequeueBuf, 0, sizeof(dequeueBuf));
		memset(tempBuf, 0, sizeof(tempBuf));

		size_t enqueueSize = 0;
		size_t dequeueSize = 0;

		// 큐에 문자열을 저장한다. 
		for (int i = 0; i < g_OSJ_len; ++i)
		{
			q.push(OSJ_testString[i]);
		}

		// 현재 Queue의 사이즈
		size_t qSize = q.size();

		// 현재 링버퍼의 FreeSize
		size_t rbSize = rb.GetFreeSize();
		if (rbSize == 0)
		{
			// 링버퍼가 가득 찼다면 무조건!!!, 랜덤한 크기만큼 Dequeue를 하고, 출력한다. 
			dequeueSize = rb.Dequeue(dequeueBuf, rand() % rb.GetUseSize() + 1);
			printArray(dequeueSize, dequeueBuf);
			continue;
		}

		// 큐 사이즈와, 링버퍼 FreeSize 중 더 작은 값을 기준으로
		// 랜덤한 값을 생성한다.
		int RandomByteToExtract = rand() % (qSize > rbSize ? rbSize : qSize) + 1;

		int index = 0;
		extractBytes(q, tempBuf, RandomByteToExtract);
		enqueueSize = rb.Enqueue(tempBuf, RandomByteToExtract);

		int randDqSize = rand() % RAND_MAX + 1;

		dequeueSize = rb.Dequeue(dequeueBuf, randDqSize);
		if (dequeueSize > 0)
			printArray(dequeueSize, dequeueBuf);

		if (q.size() > 100000)
		{
			memset(dequeueBuf, 0, sizeof(dequeueBuf));
			dequeueSize = rb.Dequeue(dequeueBuf, rb.GetUseSize());
			printArray(dequeueSize, dequeueBuf);
			while (q.size() > 0)
			{
				putchar(q.front());
				q.pop();
			}
		}
	}
}
