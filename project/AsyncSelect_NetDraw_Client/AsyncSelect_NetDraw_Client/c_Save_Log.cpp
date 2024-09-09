
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <tchar.h>

#include "c_Save_Log.h"

C_SAVE_LOG c_Save_Log;

C_SAVE_LOG::C_SAVE_LOG() : timebuf(L"\0")
{
	FILE* p_File;
	errno_t err;
	struct tm newtime;
	__time64_t long_time;

	_time64(&long_time);
	err = _localtime64_s(&newtime, &long_time);
	if (err)
	{
		printf("Invalid argument to _localtime64_s.");
		return;
	}
	swprintf_s(timebuf, L"Log_%d%d%d_%d%d%d.txt", newtime.tm_year, newtime.tm_mon, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
	
	err = _wfopen_s(&p_File, timebuf, L"wt, ccs=UTF-16LE");
	if (err != 0 || p_File == NULL)
	{
		wprintf_s(L"파일 열기 실패 \n");
		throw;
	}

	fclose(p_File);
}

C_SAVE_LOG::~C_SAVE_LOG()
{

}

void C_SAVE_LOG::saveLog(const wchar_t* Log_Str)
{
	FILE* p_File;
	errno_t err;

	err = _wfopen_s(&p_File, timebuf, L"at, ccs=UTF-16LE");
	if (err != 0 || p_File == NULL)
	{
		wprintf_s(L"파일 열기 실패 \n");
		throw;
	}

	fwprintf_s(p_File, Log_Str);

	fclose(p_File);
}
