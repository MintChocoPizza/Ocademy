
#include <string.h>

#define MULT_DEBUG 0

#if MULT_DEBUG == 1
#include <Windows.h>
#define INIT			0
#define ENQUEUE			1
#define DEQUEUE			2
#define GETUSESIZE		3
#define GETFREESIZE		4

LONG g_Flag	= INIT;
size_t Debug_Enqueue_Full_Size;
size_t Debug_Enqueue_In;
size_t Debug_Enqueue_Out;
size_t Debug_Enqueue_iSize;
char* Debug_Enqueue_Data;


size_t Debug_Dequeue_Full_Size;
size_t Debug_Dequeue_In;
size_t Debug_Dequeue_Out;
size_t Debug_Dequeue_iSize;
char* Debug_Dequeue_Data;


size_t Debug_DirectEnqueueSize_In;
size_t Debug_DirectEnqueueSize_Out;

size_t Debug_DirectDequeueSize_In;
size_t Debug_DirectDequeueSize_Out;
#endif

#include "C_Ring_Buffer.h"


/////////////////////////////////////////////////////////////////////////
// 기본 사이즈를 1만 Byte로 할당한다.
// 넣고 -> 증가 == 마지막 1칸을 사용하지 못한다. 
// 고로 입력 사이즈 + 1이 실제 사이즈
/////////////////////////////////////////////////////////////////////////
C_RING_BUFFER::C_RING_BUFFER(void) :  _Full_Size(df_C_RING_BUFFER_DEFAULT_LEN), _In(0), _Out(0)
{
	_Buffer = new char[df_C_RING_BUFFER_DEFAULT_LEN];
}

C_RING_BUFFER::C_RING_BUFFER(int i_Buffer_Size) : _Full_Size(i_Buffer_Size), _In(0), _Out(0)
{
	_Buffer = new char[i_Buffer_Size];
}

C_RING_BUFFER::~C_RING_BUFFER()
{
	delete[] _Buffer;
}


/////////////////////////////////////////////////////////////////////////
// WritePos 에 데이타 넣음.
// 남은 버퍼의 크기가 iSize보다 작은 경우 데이터를 넣지 않고 0을 반환한다.
//
// Parameters: (char *)데이타 포인터. (int)크기. 
// Return: (int)넣은 크기.
/////////////////////////////////////////////////////////////////////////
size_t C_RING_BUFFER::Enqueue(const char* pData, size_t iSize)
{
#if MULT_DEBUG == 1
	free(Debug_Dequeue_Data);
	Debug_Enqueue_Full_Size = _Full_Size;
	Debug_Enqueue_In = _In;
	Debug_Enqueue_Out = _Out;
	Debug_Enqueue_iSize = iSize;
	Debug_Enqueue_Data = (char*)malloc(iSize);
	memcpy(Debug_Enqueue_Data, pData, iSize);
#endif

	size_t Data_Chunk_Size;
	size_t Temp_Full_Size = _Full_Size;
	size_t Temp_In;

	if(Temp_Full_Size - GetUseSize() <= iSize)
		return 0;

	// (_Buffer + _Full_Size): 마지막 버퍼 그 다음 위치
	// (_Buffer + Full_Size) - _In : 현재 위치에서 마지막 버퍼까지 넣을 수 있는 데이터의 수가 나온다. 


	//Temp_In = _In;
	//Data_Chunk_Size = _Buffer_End - Temp_In;

	Temp_In = _In;
	Data_Chunk_Size = Temp_Full_Size - Temp_In;

	if(iSize <= Data_Chunk_Size)
	{
		memcpy(_Buffer+ Temp_In, pData, iSize);
	}
	else
	{
		memcpy(_Buffer+ Temp_In, pData, Data_Chunk_Size);
		memcpy(_Buffer, pData + Data_Chunk_Size, iSize - Data_Chunk_Size);
	}



	// _In = (char*)(((uintptr_t)Temp_In + iSize) % (uintptr_t)_Buffer_End);
	_In = (Temp_In + iSize) % Temp_Full_Size;



	return iSize;
}

/////////////////////////////////////////////////////////////////////////
// ReadPos 에서 데이타 가져옴. ReadPos 이동.
//
// Parameters: (char *)데이타 포인터. (int)크기.
// Return: (int)가져온 크기.
/////////////////////////////////////////////////////////////////////////
size_t C_RING_BUFFER::Dequeue(char* chpDest, size_t iSize)
{
#if MULT_DEBUG == 1
	Debug_Dequeue_Full_Size = _Full_Size;
	Debug_Dequeue_In = _In;
	Debug_Dequeue_Out = _Out;
	Debug_Dequeue_iSize = iSize;
	Debug_Dequeue_Data = (char*)malloc(iSize);


#endif



	size_t Data_Chunk_Size;
	size_t Temp_Use_Size = GetUseSize();
	size_t Temp_Out;

	if (Temp_Use_Size == 0)
		return 0;

	Temp_Out = _Out;
	Data_Chunk_Size = _Full_Size - Temp_Out;


	if (iSize <= Temp_Use_Size)
	{
		if (iSize <= Data_Chunk_Size)
		{
			memcpy(chpDest, _Buffer + Temp_Out, iSize);
		}
		else
		{
			memcpy(chpDest, _Buffer + Temp_Out, Data_Chunk_Size);
			memcpy(chpDest + Data_Chunk_Size, _Buffer, iSize - Data_Chunk_Size);
		}


		_Out = (Temp_Out + iSize) % _Full_Size;


		return iSize;
	}
	else // if(iSize > _Use_Size) 내가 준비한 버퍼가 들어있는 데이터보다 더 크다. == 모든 데이터를 꺼낸다.
	{
		//if (iSize <= Data_Chunk_Size)
		if (Temp_Use_Size <= Data_Chunk_Size)
		{
			memcpy(chpDest, _Buffer + Temp_Out, Temp_Use_Size);
		}
		else
		{
			memcpy(chpDest, _Buffer + Temp_Out, Data_Chunk_Size);
			memcpy(chpDest + Data_Chunk_Size, _Buffer, Temp_Use_Size - Data_Chunk_Size);
		}


		//_Out = (Temp_Out + Temp_Use_Size) % _Full_Size;
		_Out = _In;


		return Temp_Use_Size;
	}
}

/////////////////////////////////////////////////////////////////////////
// ReadPos 에서 데이타 읽어옴. ReadPos 고정.
//
// flag	== true: if(iSize > _Use_Size ) return 0;
// 헤더에 필요한 데이터를 다 채우지 않았다면 0을 리턴한다.
// 
// Parameters: (char *)데이타 포인터. (int)크기. (bool)defualt = false
// Return: (int)가져온 크기.
/////////////////////////////////////////////////////////////////////////
size_t C_RING_BUFFER::Peek(char* chpDest, size_t iSize, bool flag)
{
	size_t Data_Chunk_Size;
	size_t Temp_Use_Size = GetUseSize();
	size_t Temp_Out;

	if (Temp_Use_Size == 0)
		return 0;

	Temp_Out = _Out;
	Data_Chunk_Size = _Full_Size - Temp_Out;


	if (iSize <= Temp_Use_Size)
	{
		if (iSize <= Data_Chunk_Size)
		{
			memcpy(chpDest, _Buffer + Temp_Out, iSize);
		}
		else
		{
			memcpy(chpDest, _Buffer + Temp_Out, Data_Chunk_Size);
			memcpy(chpDest + Data_Chunk_Size, _Buffer, iSize - Data_Chunk_Size);
		}

		return iSize;
	}
	else // if(iSize > _Use_Size) 내가 준비한 버퍼가 들어있는 데이터보다 더 크다. == 모든 데이터를 꺼낸다.
	{
		//if (iSize <= Data_Chunk_Size)
		if (Temp_Use_Size <= Data_Chunk_Size)
		{
			memcpy(chpDest, _Buffer + Temp_Out, Temp_Use_Size);
		}
		else
		{
			memcpy(chpDest, _Buffer + Temp_Out, Data_Chunk_Size);
			memcpy(chpDest + Data_Chunk_Size, _Buffer, Temp_Use_Size - Data_Chunk_Size);
		}

		return Temp_Use_Size;
	}

	return 0;
}



















/////////////////////////////////////////////////////////////////////////
// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이.
// (끊기지 않은 길이)
//
// 원형 큐의 구조상 버퍼의 끝단에 있는 데이터는 끝 -> 처음으로 돌아가서
// 2번에 데이터를 얻거나 넣을 수 있음. 이 부분에서 끊어지지 않은 길이를 의미
//
// Parameters: 없음.
// Return: (int)사용가능 용량.
////////////////////////////////////////////////////////////////////////
size_t	C_RING_BUFFER::DirectEnqueueSize(void)
{
#if MULT_DEBUG == 1
	Debug_DirectEnqueueSize_In = _In;
	Debug_DirectEnqueueSize_Out = _Out;
#endif


	size_t Full_Size = _Full_Size;
	size_t In = _In;
	size_t Out = _Out;
	// Enqueue의 경우 _In 바로 다음이 _Out인 경우 꽉 찬 경우이다. 
	//if (_Use_Size == 0) return 0;

	if ((In + 1) % Full_Size == Out)
		return 0;

	if (In <= ((Out + Full_Size - 1) % Full_Size))
		return ((Out + Full_Size - 1) % Full_Size) - In;
	else if (In >= Out)
		return Full_Size - In;

	return 0;
}
size_t	C_RING_BUFFER::DirectDequeueSize(void)
{
#if MULT_DEBUG == 1
	Debug_DirectDequeueSize_In = _In;
	Debug_DirectDequeueSize_Out = _Out;
#endif


	size_t In = _In;
	size_t Out = _Out;



	if (In >= Out)
	{
		return In - Out;
	}
	else
	{
		return _Full_Size - Out;
	}

	//if (_In >= _Out)
	//{
	//	return _In - _Out;
	//}
	//else
	//{
	//	return _Full_Size - _Out;
	//}
}

/////////////////////////////////////////////////////////////////////////
// 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
//
// Parameters: 없음.
// Return: (int)이동크기
/////////////////////////////////////////////////////////////////////////
size_t	C_RING_BUFFER::MoveIn(size_t iSize)
{
	_In = (_In + iSize) % _Full_Size;
	return _In;
}
size_t	C_RING_BUFFER::MoveOut(size_t iSize)
{
	_Out = (_Out + iSize) % _Full_Size;
	return _Out;
}



/////////////////////////////////////////////////////////////////////////
// 버퍼의 Front, _Out 포인터 얻음.
//
// Parameters: 없음.
// Return: (char *) 버퍼 포인터.
/////////////////////////////////////////////////////////////////////////
char* C_RING_BUFFER::GetOutBufferPtr(void)
{
	return _Buffer + _Out;
}


/////////////////////////////////////////////////////////////////////////
// 버퍼의 RearPos, _In 포인터 얻음.
//
// Parameters: 없음.
// Return: (char *) 버퍼 포인터.
/////////////////////////////////////////////////////////////////////////
char* C_RING_BUFFER::GetInBufferPtr(void)
{
	return _Buffer + _In;
}

/////////////////////////////////////////////////////////////////////////
// 버퍼의 _Begin 포인터 얻음.
//
// Parameters: 없음.
// Return: (char *) 버퍼 시작 포인터.
/////////////////////////////////////////////////////////////////////////
char* C_RING_BUFFER::GetBeginBufferPtr(void)
{
	return _Buffer;
}

