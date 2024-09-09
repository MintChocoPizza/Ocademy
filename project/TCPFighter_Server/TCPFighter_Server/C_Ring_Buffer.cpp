
#include <string.h>

#include "C_Ring_Buffer.h"

/////////////////////////////////////////////////////////////////////////
// 기본 사이즈를 1만 Byte로 할당한다.
/////////////////////////////////////////////////////////////////////////
C_RING_BUFFER::C_RING_BUFFER(void) :  _Full_Size(df_C_RING_BUFFER_DEFAULT_LEN + 1), _Use_Size(0), _In(0), _Out(0)
{
	_Buffer = new char[df_C_RING_BUFFER_DEFAULT_LEN + 1];
}

C_RING_BUFFER::C_RING_BUFFER(int i_Buffer_Size) : _Full_Size(i_Buffer_Size), _Use_Size(0), _In(0), _Out(0)
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
	size_t Data_Chunk_Size;
	size_t Temp_Full_Size = _Full_Size;
	size_t Temp_In;

	if(Temp_Full_Size - _Use_Size < iSize)
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

	//-----------------------------------------------------------------------
	// 어떤 것을 유지할지 결정하지 못하였다. 
	_Use_Size += iSize;

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
	size_t Data_Chunk_Size;
	size_t Temp_Use_Size = _Use_Size;
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

		_Use_Size = Temp_Use_Size - iSize;

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

		_Use_Size = 0;

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
	size_t Temp_Use_Size = _Use_Size;
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

		return (int)iSize;
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


