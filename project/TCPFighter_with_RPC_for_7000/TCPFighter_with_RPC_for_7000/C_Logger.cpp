
#include <Windows.h>
#include <strsafe.h>
#include <cstdarg>

#include <crtdbg.h>

enum en_LOG_LEVEL
{
	LEVEL_DEBUG,
	LEVEL_ERROR,
	LEVEL_SYSTEM
};


static DWORD g_dwLogCount = 0;
static en_LOG_LEVEL g_LogLevel = LEVEL_DEBUG;

#define CONSOLE 3
#define TEXTFILE 4


__declspec(thread) WCHAR* g_pszFolerPath;
__declspec(thread) WCHAR* g_pLogBufForFileConsole;

#define LOCAL_LOG_BUF_SIZE 3000
int g_CurSize = LOCAL_LOG_BUF_SIZE;

SRWLOCK g_srwForFILEIO;

void LOG(const WCHAR* szType, en_LOG_LEVEL LogLevel, bool bConsole, bool bTextFile, const WCHAR* szStringFormat, ...);


void TempBufferToFile(const WCHAR* szType, const WCHAR* Buffer)
{
	// 파일명 생성
	SYSTEMTIME st;
	FILE* pFile;
	WCHAR FilePath[MAX_PATH];

	GetLocalTime(&st);

	AcquireSRWLockExclusive(&g_srwForFILEIO);
	StringCchPrintf(FilePath, MAX_PATH, L"%s\\%04d%02d_%s.txt", g_pszFolerPath, st.wYear, st.wMonth, szType);
	_wfopen_s(&pFile, FilePath, L"a");
	fputws(Buffer, pFile);
	fclose(pFile);
	ReleaseSRWLockExclusive(&g_srwForFILEIO);
}
void TempBufferToFileBinary(const WCHAR* szType, const CHAR* Buffer, SIZE_T size)
{
	// 파일 명 생성
	SYSTEMTIME st;
	GetLocalTime(&st);

	AcquireSRWLockExclusive(&g_srwForFILEIO);
	FILE* pFile;
	WCHAR FilePath[MAX_PATH];
	StringCchPrintf(FilePath, MAX_PATH, L"%s\\%04d%02d_%s.txt", g_pszFolerPath, st.wYear, st.wMonth, szType);
	_wfopen_s(&pFile, FilePath, L"a");
	fwprintf(pFile, L"%02X %02X %02X %02X %02X %02X %02X %02X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3], Buffer[4], Buffer[5], Buffer[6], Buffer[7]);
	fclose(pFile);
	ReleaseSRWLockExclusive(&g_srwForFILEIO);
}


__forceinline void ExceptLogBufInSuf()
{
	HeapFree(GetProcessHeap(), 0, g_pLogBufForFileConsole);
	g_pLogBufForFileConsole = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, g_CurSize * sizeof(WCHAR));
	g_CurSize *= 2;
	LOG(L"SYSTEM", LEVEL_SYSTEM, false, true, L"LOG_BUFSIZE INSUFFICIENT INSUFFICIENT %d -> %dBytes", g_CurSize / 2, g_CurSize);
}
__forceinline void ExceptLogBufInSufForTempBuffer()
{
	g_pLogBufForFileConsole = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, g_CurSize * 2 * sizeof(WCHAR));
	g_CurSize *= 2;
	LOG(L"SYSTEM", LEVEL_SYSTEM, false, true, L"LogBUFSIZE INSUFFICIENT ALLOCATE %d -> %dBytes", g_CurSize / 2, g_CurSize);
}
void LOG(const WCHAR* szType, en_LOG_LEVEL LogLevel, bool bConsole, bool bTextFile, const WCHAR* szStringFormat, ...)
{
	SYSTEMTIME st;
	size_t len;
	HRESULT hResult;
	WCHAR LogLevelStr[10];
	DWORD dwLogCount;
	va_list va;
	FILE* pFile;
	WCHAR FilePath[MAX_PATH];

	// 개인적으로 로그 레벨을 판단하는 것은 해당 순간 완벽하지 않아도 된다고 생각한다.
	if (g_LogLevel > LogLevel)
		return;

	// Thread Local Storage
	while (1)
	{
		//[BATTLE] 같은 문자열 붙이기
		//StringCbPrintf(g_pLogBufForFileConsole, sizeof(WCHAR) * g_CurSize, L"[%s]  ", szType);
		StringCchPrintf(g_pLogBufForFileConsole, g_CurSize, L"[%s]  ", szType);

		GetLocalTime(&st);

		// 연월일시분초
		len = wcslen(g_pLogBufForFileConsole);
	
		hResult = StringCchPrintf(g_pLogBufForFileConsole + len, 
			g_CurSize - len, 
			L"[%04d-%02d-%02d %02d:%02d:%02d / ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		// 메모리가 부족하면 추가한다.
		if (hResult == STRSAFE_E_INSUFFICIENT_BUFFER)
		{
			ExceptLogBufInSuf();
			continue;
		}

		// 로그 레벨 구하기
		switch (LogLevel)
		{
		case LEVEL_DEBUG:
			memcpy(LogLevelStr, L"DEBUG", sizeof(L"DEBUG"));
			break;
		case LEVEL_ERROR:
			memcpy(LogLevelStr, L"ERROR", sizeof(L"ERROR"));
			break;
		case LEVEL_SYSTEM:
			memcpy(LogLevelStr, L"SYSTEM", sizeof(L"SYSTEM"));
			break;
		default:
			__debugbreak();
			break;
		}
		
		len = wcslen(g_pLogBufForFileConsole);
		// 로그가 찍힌 순서를 멀티스레드 안전성 있게 파악하기 위해서 설정
		dwLogCount = InterlockedIncrement((LONG*)&g_dwLogCount);

		hResult = StringCchPrintf(g_pLogBufForFileConsole + len, g_CurSize - len, L"%-6s / %09d]  ", LogLevelStr, dwLogCount);
		if (hResult == STRSAFE_E_INSUFFICIENT_BUFFER)
		{
			ExceptLogBufInSuf();
			continue;
		}

		// 진짜 로깅하고자 하는 것 가변인자로 넣기
		len = wcslen(g_pLogBufForFileConsole);
		va_start(va, szStringFormat);
		_ASSERTE(_CrtCheckMemory());
		hResult = StringCchVPrintf(g_pLogBufForFileConsole + len, g_CurSize - len, szStringFormat, va);
		va_end(va);
		if (hResult == STRSAFE_E_INSUFFICIENT_BUFFER)
		{
			ExceptLogBufInSuf();
			continue;
		}

		if (bConsole)
		{
			wprintf(L"%s\n", g_pLogBufForFileConsole);
		}

		if (bTextFile)
		{
			// 파일명 생성
			StringCchPrintf(FilePath, MAX_PATH, L"%s\\%04d%02d_%s.txt", g_pszFolerPath, st.wYear, st.wMonth, szType);


			// 파일 열고 쓴 다음 닫기
			// 여러개의 스레드들이 동시에 같은 파일에 접근하면 파일열기에 실패할 수도 있음
			AcquireSRWLockExclusive(&g_srwForFILEIO);
			_wfopen_s(&pFile, FilePath, L"a");
			fputws(g_pLogBufForFileConsole, pFile);
			fputc(L'\n', pFile);
			fclose(pFile);
			ReleaseSRWLockExclusive(&g_srwForFILEIO);
		}

		break;
	}
}


void SET_LOG_LEVEL(en_LOG_LEVEL LogLevel)
{
	if (LogLevel >= LEVEL_DEBUG && LogLevel <= LEVEL_SYSTEM)
		InterlockedExchange((LONG*)&g_LogLevel, LogLevel);
}
en_LOG_LEVEL subtype_INCREASE_LOG_LEVEL(void)
{
	en_LOG_LEVEL Ret;

	AcquireSRWLockExclusive(&g_srwForFILEIO);
	if (g_LogLevel >= LEVEL_DEBUG && g_LogLevel <= LEVEL_SYSTEM)
		++(*(LONG*)&g_LogLevel);
	Ret = g_LogLevel;
	ReleaseSRWLockExclusive(&g_srwForFILEIO);

	return Ret;
}
en_LOG_LEVEL subtype_DECREASE_LOG_LEVEL(void)
{
	en_LOG_LEVEL Ret;

	AcquireSRWLockExclusive(&g_srwForFILEIO);
	if (g_LogLevel >= LEVEL_DEBUG && g_LogLevel <= LEVEL_SYSTEM)
		--(*(LONG*)&g_LogLevel);
	Ret = g_LogLevel;
	ReleaseSRWLockExclusive(&g_srwForFILEIO);

	return Ret;
}


static void GetParentDir(WCHAR* szTargetPath)
{
	WCHAR* lastSlash = wcsrchr(szTargetPath, L'\\');
	if(lastSlash)
		*lastSlash = L'\0';
}

 static void SYSLOG_DIRECTORY(const WCHAR* szLogPath)
{
	WCHAR exePath[MAX_PATH];
	WCHAR ParentDir[MAX_PATH];
	DWORD dwErrCode;

	// 실행파일 이름 얻어오기
	GetModuleFileName(NULL, exePath, MAX_PATH);
	//StringCbCopy(ParentDir, sizeof(WCHAR) * MAX_PATH, exePath);
	StringCchCopy(ParentDir, MAX_PATH, exePath);

	// 두 단계 윗 경로 얻어오기
	GetParentDir(ParentDir);
	GetParentDir(ParentDir);

	// 폴더 이름 만들기
	StringCchPrintf(g_pszFolerPath, MAX_PATH, L"%s\\%s", ParentDir, szLogPath);

	// 폴더 존재하면 넘어가고 없으면 만들기
	DWORD dwAttr = GetFileAttributes(g_pszFolerPath);
	if (dwAttr == INVALID_FILE_ATTRIBUTES)
	{
		if (!CreateDirectory(g_pszFolerPath, NULL))
		{
			dwErrCode = GetLastError();
			__debugbreak();
		}
	}
}

void InitLogger(const WCHAR* szLogPath)
{
	InitializeSRWLock(&g_srwForFILEIO);

	g_pszFolerPath = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, MAX_PATH * sizeof(WCHAR));
	g_pLogBufForFileConsole = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, g_CurSize * sizeof(WCHAR));

	SYSLOG_DIRECTORY(szLogPath);
}

void ClearLogger(void)
{
	LOG(L"SYSTEM", LEVEL_SYSTEM, true, true, L"ClearLogger # \n\n\n");
	HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, g_pszFolerPath);
	HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, g_pLogBufForFileConsole);
}
