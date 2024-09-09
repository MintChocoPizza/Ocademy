
#ifndef __C_MEMORY_FLOW_H__
#define __C_MEMORY_FLOW_H__

#include <Windows.h>

inline void* MemoryOverflowAlloc(size_t size)
{
	const int page_Size = 4096;
	DWORD old_Protest = NULL;
	size_t mem_Size;
	unsigned char* alloc_Address;
	unsigned char* protect_Address;
	unsigned char* ret_Address;
	bool err_check;
	DWORD dw_Error_Code;

	// size의 값을 반올림 후 + 4KB(1페이지)를 더한다.
	mem_Size = ((size + page_Size - 1)  & ~(page_Size-1)) + 4096;

	alloc_Address = (unsigned char *)VirtualAlloc(NULL, mem_Size, MEM_COMMIT, PAGE_READWRITE);
	if (alloc_Address == NULL)
		throw;

	// NoAccess 속성을 걸 페이지의 시작 주소를 계산한다. 
	protect_Address = alloc_Address + mem_Size - 4096;
	if (protect_Address == 0)
		throw;

	err_check = VirtualProtect(protect_Address, 4096, PAGE_NOACCESS, &old_Protest);
	if (err_check == NULL)
	{
		dw_Error_Code = GetLastError();
		std::cerr << "error code: " << dw_Error_Code << std::endl;
	}


	// 반환할 주소를 계산한다. 
	ret_Address = protect_Address - size;

	printf("할당한 메모리주소: %p \n", alloc_Address);

	return ret_Address;
}

inline void MemoryOverflowFree(void* address)
{
	const int page_Size = 4096;
	size_t free_Address = (size_t)address & ~(page_Size-1);

	printf("해지한 메모리 주소: %p \n", (void*)free_Address);
	
	VirtualFree((void *)free_Address, 0, MEM_RELEASE);
}




#endif // !__C_MEMORY_FLOW_H__
