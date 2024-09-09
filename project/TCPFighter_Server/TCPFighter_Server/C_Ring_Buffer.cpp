
#include <string.h>

#include "C_Ring_Buffer.h"

/////////////////////////////////////////////////////////////////////////
// �⺻ ����� 1�� Byte�� �Ҵ��Ѵ�.
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
// WritePos �� ����Ÿ ����.
// ���� ������ ũ�Ⱑ iSize���� ���� ��� �����͸� ���� �ʰ� 0�� ��ȯ�Ѵ�.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��. 
// Return: (int)���� ũ��.
/////////////////////////////////////////////////////////////////////////
size_t C_RING_BUFFER::Enqueue(const char* pData, size_t iSize)
{
	size_t Data_Chunk_Size;
	size_t Temp_Full_Size = _Full_Size;
	size_t Temp_In;

	if(Temp_Full_Size - _Use_Size < iSize)
		return 0;

	// (_Buffer + _Full_Size): ������ ���� �� ���� ��ġ
	// (_Buffer + Full_Size) - _In : ���� ��ġ���� ������ ���۱��� ���� �� �ִ� �������� ���� ���´�. 


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
	// � ���� �������� �������� ���Ͽ���. 
	_Use_Size += iSize;

	return iSize;
}

/////////////////////////////////////////////////////////////////////////
// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��.
// Return: (int)������ ũ��.
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
	else // if(iSize > _Use_Size) ���� �غ��� ���۰� ����ִ� �����ͺ��� �� ũ��. == ��� �����͸� ������.
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
// ReadPos ���� ����Ÿ �о��. ReadPos ����.
//
// flag	== true: if(iSize > _Use_Size ) return 0;
// ����� �ʿ��� �����͸� �� ä���� �ʾҴٸ� 0�� �����Ѵ�.
// 
// Parameters: (char *)����Ÿ ������. (int)ũ��. (bool)defualt = false
// Return: (int)������ ũ��.
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
	else // if(iSize > _Use_Size) ���� �غ��� ���۰� ����ִ� �����ͺ��� �� ũ��. == ��� �����͸� ������.
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


