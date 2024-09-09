#pragma once

#define dfLOG_LEVEL_DEBUG	0
#define dfLOG_LEVEL_ERROR	1
#define dfLOG_LEVEL_SYSTEM	2

extern int g_iLogLevel;
extern wchar_t g_szLogBuff[1024];
extern char g_sz_c_LogBuff[1024];

void Log(wchar_t * szString, int iLogLevel);
void c_Log(char* szString, int iLogLevel);

#define _LOG(LogLevel, fmt, ...)									\
do {																\
		if(g_iLogLevel <= LogLevel)									\
		{															\
			wsprintf(g_szLogBuff, fmt, ##__VA_ARGS__);				\
			Log(g_szLogBuff, LogLevel);								\
		}															\
																	\
}while(0)															\


#define _c_LOG(LogLevel, fmt, ...)									\
do {																\
		if(g_iLogLevel <= LogLevel)									\
		{															\
			sprintf_s(g_sz_c_LogBuff, fmt, ##__VA_ARGS__);			\
			c_Log(g_sz_c_LogBuff, LogLevel);						\
		}															\
																	\
}while(0)															\
