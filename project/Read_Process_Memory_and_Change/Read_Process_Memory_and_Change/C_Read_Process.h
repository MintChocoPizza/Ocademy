
#ifndef __C_READ_PROCESS_H__
#define __C_READ_PROCESS_H__

#include <stdio.h>
#include <iostream>
#include <Windows.h>

namespace OreoPizza
{
	class C_Read_Process
	{
	public:
		inline C_Read_Process();
		inline ~C_Read_Process();

		void setPID(void);
		void setProcessHandle(void);
		void setSystemInfo(void);
		void setFindValue(void);

		void findValue(void);
		
		


		DWORD _dw_PID;
		HANDLE _h_Process;
		SYSTEM_INFO _System_Info;

		MEMORY_BASIC_INFORMATION _Memory_Basic_Info;
		SIZE_T _size;

		DWORD _dw_Find_Value;

		unsigned char* _uc_Curr_Memory_Pointer;


	};

	inline OreoPizza::C_Read_Process::C_Read_Process()
	{

	};

	inline OreoPizza::C_Read_Process::~C_Read_Process(void)
	{
	};

	inline void OreoPizza::C_Read_Process::setPID(void)
	{
		scanf_s("%d", &_dw_PID);
	}

	inline void OreoPizza::C_Read_Process::setProcessHandle(void)
	{
		_h_Process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _dw_PID);
		if (_h_Process == NULL)
		{
			std::cout << "프로세스 핸들을 얻을 수 없습니다. \n";
			std::cout << GetLastError() << std::endl;
			throw;
		}
	}

	inline void OreoPizza::C_Read_Process::setSystemInfo(void)
	{
		GetSystemInfo(&_System_Info);
	}

	inline void OreoPizza::C_Read_Process::setFindValue(void)
	{
		scanf_s("%d", &_dw_Find_Value);
	}

	inline void OreoPizza::C_Read_Process::findValue(void)
	{
		bool isSuccessful;

		unsigned char* Buff;
		unsigned int idx;


		//////////////////////////////////////////////////////////////////////
		// _System_Info.lpMinimumApplicationAddress 부터 
		// _System_Info.lpMaximumApplicationAddress 까지
		// 
		// 해당 프로세스에 할당된 가상메모리 시작부터 끝 까지 탐색한다.
		//
		//////////////////////////////////////////////////////////////////////

		// lpMinimumApplicationAddress 메모리 시작 주소부터 VirtualQueryEx 로 정보를 얻는다. 
		_uc_Curr_Memory_Pointer = (unsigned char *)_System_Info.lpMinimumApplicationAddress;

		while (VirtualQueryEx(_h_Process, _uc_Curr_Memory_Pointer, &_Memory_Basic_Info, sizeof(MEMORY_BASIC_INFORMATION)))
		{

			// 얻은 메모리(페이지)의 속성 개인메모리, 커밋 확인 
			if (_Memory_Basic_Info.Type == MEM_PRIVATE && _Memory_Basic_Info.State == MEM_COMMIT)
			{
				// 가드페이지인 경우 미리 표기한다. 
				if (_Memory_Basic_Info.Protect & PAGE_GUARD)
				{
					printf_s("This Page is Guard!!!! \n");
				}

				// 얻은 메모리의 정보를 표기한다. 
				printf_s("[BaseAddress: %p], [AllocationBase: %p], [RegionSize: %d]", _Memory_Basic_Info.BaseAddress, _Memory_Basic_Info.AllocationBase , _Memory_Basic_Info.RegionSize);

				// RegionSize 사이즈의 메모리 확보
				Buff = (unsigned char*)malloc(_size);
				if (Buff == NULL)
					throw;

				isSuccessful = ReadProcessMemory(_h_Process, _uc_Curr_Memory_Pointer, Buff, _size, NULL);

				// 1Byte 씩 증가하여 메모리를 비교한다. 
				for (idx = 0; Buff + idx < _uc_Curr_Memory_Pointer + _Memory_Basic_Info.RegionSize; ++idx)
				{

				}

				
				free(Buff);
			}

		}
	}

}



#endif // !__C_READ_PROCESS_H__

