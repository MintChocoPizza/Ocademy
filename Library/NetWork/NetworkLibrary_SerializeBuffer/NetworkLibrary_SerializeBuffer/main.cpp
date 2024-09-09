
#include <stdio.h>

#include <Windows.h>
#include "SerializeBuffer.h"

int main()
{
	SerializeBuffer sb;
	const char* t = "abcd";

	sb.PutData((char*)t, strlen(t));

	sb.PutHeader();




	return 0;
}