
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <tchar.h>
#include <stdarg.h>

#include "c_Save_Log.h"

C_SAVE_LOG c_Save_Log;

C_SAVE_LOG::C_SAVE_LOG() : isWrite(false)
{
	FILE* p_File;
	errno_t err;
	struct tm newtime;
	__time64_t long_time;
	timebuf[0] = '\0';

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
	if (isWrite == false)
	{
		_wremove(timebuf);
	}
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

	isWrite = true;

	fclose(p_File);
}

void C_SAVE_LOG::printfLog(const wchar_t* format, ...)
{
	FILE* p_File;
	errno_t err;

	err = _wfopen_s(&p_File, timebuf, L"at, ccs=UTF-16LE");
	if (err != 0 || p_File == NULL)
	{
		wprintf_s(L"파일 열기 실패 \n");
		throw;
	}

	// 가변 인자 목록 선언 및 초기화
	va_list args;
	va_start(args, format);

	// 가변 인자를 파일에 출력
	vfwprintf(p_File, format, args);

	// 가변 인자 목록 종료
	va_end(args);

	isWrite = true;

	// 파일 닫기
	fclose(p_File);
}
