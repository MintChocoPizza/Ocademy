
#ifndef __OSJ_TEST_H__
#define __OSJ_TEST_H__

#include <queue>


void printArray(size_t width, char* pBuf);

// ť���� ���ϴ� ����Ʈ ����ŭ �����Ͽ� ������ ���ۿ� ����ִ� �Լ�
void extractBytes(std::queue<char>& myQueue, char* buffer, int bytes);

void OSJ_test();

unsigned _stdcall QueuePush(void* pArg);
unsigned _stdcall QueuePop(void* pArg);

#endif // !__OSJ_TEST_H__



