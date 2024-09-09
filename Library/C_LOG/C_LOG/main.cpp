#include <process.h>
#include <windows.h>
#include <iostream>

#include "C_Logger.h"

//// Explained in p. 2 below
//void NTAPI tls_callback(PVOID DllHandle, DWORD dwReason, PVOID)
//{
//    if (dwReason == DLL_THREAD_ATTACH)
//    {
//        MessageBox(0, L"DLL_THREAD_ATTACH", L"DLL_THREAD_ATTACH", 0);
//    }
//
//    if (dwReason == DLL_PROCESS_ATTACH)
//    {
//        MessageBox(0, L"DLL_PROCESS_ATTACH", L"DLL_PROCESS_ATTACH", 0);
//    }
//}
//
//#ifdef _WIN64
//#pragma comment (linker, "/INCLUDE:_tls_used")  // See p. 1 below
//#pragma comment (linker, "/INCLUDE:tls_callback_func")  // See p. 3 below
//#else
//#pragma comment (linker, "/INCLUDE:__tls_used")  // See p. 1 below
//#pragma comment (linker, "/INCLUDE:_tls_callback_func")  // See p. 3 below
//#endif
//
//// Explained in p. 3 below
//#ifdef _WIN64
//#pragma const_seg(".CRT$XLF")
//EXTERN_C const
//#else
//#pragma data_seg(".CRT$XLF")
//EXTERN_C
//#endif
//PIMAGE_TLS_CALLBACK tls_callback_func = tls_callback;
//#ifdef _WIN64
//#pragma const_seg()
//#else
//#pragma data_seg()
//#endif //_WIN64

unsigned int __stdcall ThreadFunction(void* data) {
    // g_pLogBufForFileConsole를 사용한 코드

    LOG(L"Temp", LEVEL_SYSTEM, true, true, L"XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    
    return 0;
}

int main(void)
{
    unsigned int threadID1, threadID2;

    InitLogger(L"TempLog");

    HANDLE hThread1 = (HANDLE)_beginthreadex(nullptr, 0, ThreadFunction, nullptr, 0, &threadID1);

    WaitForSingleObject(hThread1, INFINITE);

    CloseHandle(hThread1);

    return 0;
}