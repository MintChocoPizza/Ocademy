// stack_view_project.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <windows.h>
#include <stdio.h>
using CPUWORD = UINT_PTR;

int main()
{
	// 작업관리자에서 확인해서 Process Id 를 입력하면 됨
	printf("input PID : ");
	DWORD PID;
	scanf_s(" %u", &PID);
	HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);

	printf("Insert the value you want to find in memory : ");
	DWORD valueToFind;
	scanf_s(" %u", &valueToFind);

	DWORD valueToModify;
	printf("Insert the value you want to push in memory : ");
	scanf_s(" %u", &valueToModify);

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	// 시작위치, 유저 가상메모리 가장 낮은주소부터 시작
	UINT_PTR pTargetMemoryPos = (UINT_PTR)si.lpMinimumApplicationAddress;

	// 가상메모리 페이지 정보 얻어오기
	MEMORY_BASIC_INFORMATION mbi;

	// 탐색 시작
	while (pTargetMemoryPos < (UINT_PTR)si.lpMaximumApplicationAddress)
	{
		SIZE_T size = VirtualQueryEx(hTargetProcess, (LPCVOID)pTargetMemoryPos, &mbi, sizeof(mbi));

		// 다른 프로세스와 공유하는 메모리는 찾지 않는다.
		if (!(size == sizeof(mbi) && mbi.Type == MEM_PRIVATE && mbi.State == MEM_COMMIT))
		{
			pTargetMemoryPos += (UINT_PTR)mbi.RegionSize;
			continue;
		}

		//적절한 메모리 구역을 찾앗기 때문에 해당 메모리를 버퍼에 읽어옴.
		CPUWORD* pTempBuffer = new CPUWORD[mbi.RegionSize / sizeof(CPUWORD)];
		BOOL bResult = ReadProcessMemory(hTargetProcess, (LPCVOID)pTargetMemoryPos, (LPVOID)pTempBuffer, mbi.RegionSize, NULL);
		if (!bResult)
		{
			printf("\nRead Process Memory Failed\n addr : %x, protect attributes : %x, Err Code : %u \n", pTargetMemoryPos, mbi.Protect, GetLastError());
			pTargetMemoryPos += (UINT_PTR)mbi.RegionSize;
			delete[] pTempBuffer;
			continue;
		}
		printf("Read Process Memory Success\n addr : %x\n", pTargetMemoryPos);
		for (DWORD i = 0; i < mbi.RegionSize / sizeof(CPUWORD); ++i)
		{
			// 조작하고자 하는 메모리의 값과 일치하는 값을 발견함
			if (pTempBuffer[i] == valueToFind)
			{
				printf("\nsuccess to find value!\nRegion size : %uKbytes", mbi.RegionSize / 1024);
				UINT_PTR memAddrToModify = (UINT_PTR)mbi.BaseAddress + i * sizeof(CPUWORD);
				printf("\nTarget Addr : %x", memAddrToModify);
				SIZE_T NumberOfBytesWritten;
				bResult = WriteProcessMemory(hTargetProcess, (LPVOID)memAddrToModify, &valueToModify, sizeof(valueToModify), &NumberOfBytesWritten);
				if (!bResult) __debugbreak();
				else if (NumberOfBytesWritten != sizeof(CPUWORD)) __debugbreak();

				printf("\nIf successful, press e to end the process : ");
				char TerminateChar;
				scanf_s(" %c", &TerminateChar, sizeof(char));
				if (TerminateChar == 'e')
				{
					delete[] pTempBuffer;
					return 0;
				}
			}
		}
		pTargetMemoryPos += (UINT_PTR)mbi.RegionSize;
	}
	return 0;
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
