
#include <vector>
#include <thread>
#include <Windows.h>
#include <iostream>

#include "Fork.h"
#include "Philosopher.h"
#include "DiningPhilosophers.h"

int main()
{
	// 식사하는 철학자를 구현해보자
	// SpinLock을 활용하면 while(1)을 돌면서 무한 대기에 빠지기 때문에
	// 양손을 활용한 식사를 할 수 없다. 
	// 
	// 고로 이번 예시에서는 포크를 1개만 생성하여, 딱 1명만 식사하고 나머지 4명은 구경하는 예시이다.

	DiningPhilosophers* phs = new DiningPhilosophers();

	phs->start();
	phs->join();
	
	printf("All threads have finished.\n");
	return 0;
}

