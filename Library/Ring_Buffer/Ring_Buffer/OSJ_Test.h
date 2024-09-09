
#ifndef __OSJ_TEST_H__
#define __OSJ_TEST_H__

#include <queue>


void printArray(size_t width, char* pBuf);

// 큐에서 원하는 바이트 수만큼 추출하여 임의의 버퍼에 집어넣는 함수
void extractBytes(std::queue<char>& myQueue, char* buffer, int bytes);

void OSJ_test();

unsigned _stdcall QueuePush(void* pArg);
unsigned _stdcall QueuePop(void* pArg);

#endif // !__OSJ_TEST_H__



