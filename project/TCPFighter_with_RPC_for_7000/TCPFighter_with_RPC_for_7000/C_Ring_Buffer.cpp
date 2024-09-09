
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
// �⺻ ����� 1�� Byte�� �Ҵ��Ѵ�.
// �ְ� -> ���� == ������ 1ĭ�� ������� ���Ѵ�. 
// ��� �Է� ������ + 1�� ���� ������
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
// WritePos �� ����Ÿ ����.
// ���� ������ ũ�Ⱑ iSize���� ���� ��� �����͸� ���� �ʰ� 0�� ��ȯ�Ѵ�.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��. 
// Return: (int)���� ũ��.
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



















/////////////////////////////////////////////////////////////////////////
// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����.
// (������ ���� ����)
//
// ���� ť�� ������ ������ ���ܿ� �ִ� �����ʹ� �� -> ó������ ���ư���
// 2���� �����͸� ��ų� ���� �� ����. �� �κп��� �������� ���� ���̸� �ǹ�
//
// Parameters: ����.
// Return: (int)��밡�� �뷮.
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
	// Enqueue�� ��� _In �ٷ� ������ _Out�� ��� �� �� ����̴�. 
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
// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
//
// Parameters: ����.
// Return: (int)�̵�ũ��
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
// ������ Front, _Out ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char* C_RING_BUFFER::GetOutBufferPtr(void)
{
	return _Buffer + _Out;
}


/////////////////////////////////////////////////////////////////////////
// ������ RearPos, _In ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char* C_RING_BUFFER::GetInBufferPtr(void)
{
	return _Buffer + _In;
}

/////////////////////////////////////////////////////////////////////////
// ������ _Begin ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ���� ������.
/////////////////////////////////////////////////////////////////////////
char* C_RING_BUFFER::GetBeginBufferPtr(void)
{
	return _Buffer;
}

