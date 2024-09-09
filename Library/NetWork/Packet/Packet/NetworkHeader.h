#pragma once

#include <Windows.h>

#define NETWORK_HEADER_SIZE sizeof(st_NETWORKHEADER)

struct st_NETWORKHEADER
{
	WORD Len;
};
