
#include <Windows.h>
#include <stdio.h>
#include <iostream>

#include <list>

#include "CList.h"

using namespace std;

class A
{
public:
	int a;
};

int main()
{
	CList<A*> c_li[10][20];

	CList<int> aaa;

	aaa.push_back(10);
	aaa.push_back(20);

	int a = aaa.front();
	int b = aaa.back();
	
	printf("%d, %d \n", a, b);



	return 0;
}