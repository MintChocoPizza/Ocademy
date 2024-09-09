

#ifndef __C_LOGGER_H__
#define __C_LOGGER_H__
#include <Windows.h>
#include <cstdarg>


enum en_LOG_LEVEL
{
	LEVEL_DEBUG,
	LEVEL_ERROR,
	LEVEL_SYSTEM
};

#define CONSOLE 3
#define TEXTFILE 4

void InitLogger(const WCHAR* szPath);
void ClearLogger(void);

en_LOG_LEVEL INCREASE_LOG_LEVEL(void);
en_LOG_LEVEL DECREASE_LOG_LEVEL(void);
void SET_LOG_LEVEL(en_LOG_LEVEL LogLevel);

void LOG(const WCHAR* szType, en_LOG_LEVEL LogLevel, bool bConsole, bool bTextFile, const WCHAR* szStringFormat, ...);

void TempBufferToFile(const WCHAR* szType, const WCHAR* Buffer);
void TempBufferToFileBinary(const WCHAR* szType, const CHAR* Buffer, SIZE_T size);



#endif // !__C_LOG_H__
