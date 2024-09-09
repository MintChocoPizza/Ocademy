
#include <vector>
#include <thread>
#include <Windows.h>
#include <iostream>

#include "Synchronization_Object.h"
#include "Fork.h"
#include "Philosopher.h"
#include "DiningPhilosophers.h"

int main()
{
    DiningPhilosophers* phs = new DiningPhilosophers();

    phs->start();
    phs->join();

    std::cout << "All threads have finished. \n";
    std::cout << "Hello World!\n";

    return 0;
}

