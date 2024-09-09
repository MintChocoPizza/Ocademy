
#include <string.h>
#include "C_Ring_Buffer.h"



#define GetDirectEnqueueSize_MACRO(in,out,iRet) do\
{\
if(in >= out){\
	if(out > 0){\
		iRet = ACTUAL_SIZE - in;\
	}else{\
		iRet = BUFFER_SIZE - in;\
}}\
else{\
	iRet = out - in - 1;\
}\
}while(0)\

#define GetDirectDequeueSize_MACRO(in,out,iRet) do{\
if(in >= out){\
	iRet = in - out;\
}\
else{\
	iRet = ACTUAL_SIZE - out;\
}\
}while(0)\


#define GetUseSize_MACRO(in,out,iRet) do\
{\
	if(in >= out){\
		iRet = in - out;\
	}\
	else{\
		iRet = ACTUAL_SIZE - out + in;\
	}\
}while(0)\

#define GetFreeSize_MACRO(in,out,iRet) do\
{\
int iUseSize;\
GetUseSize_MACRO(in,out,iUseSize);\
iRet = BUFFER_SIZE - iUseSize;\
}while(0)\


#define GetInStartPtr_MACRO(in,iRet) do{\
iRet = (Buffer_ + in);\
}while(0)\

#define GetOutStartPtr_MACRO(out,iRet) do{\
iRet = (Buffer_ + out);\
}while(0)\

#define GetOutStartPtr_MACRO(out,iRet) do{\
iRet = (Buffer_ + out);\
}while(0)\

#define MoveInOrOutPos_MACRO(iPos,iMoveSize) do{\
iPos = (iPos + iMoveSize) % (ACTUAL_SIZE);\
}while(0);\


/////////////////////////////////////////////////////////////////////////
// �⺻ ����� 1�� Byte�� �Ҵ��Ѵ�.
// �ְ� -> ���� == ������ 1ĭ�� ������� ���Ѵ�. 
// ��� �Է� ������ + 1�� ���� ������
/////////////////////////////////////////////////////////////////////////
C_RING_BUFFER::C_RING_BUFFER(void) :  _Full_Size(df_C_RING_BUFFER_DEFAULT_LEN), _In(0), _Out(0)
{
	_Buffer = new char[df_C_RING_BUFFER_DEFAULT_LEN];
	InitializeSRWLock(&_srwlock);
}

C_RING_BUFFER::C_RING_BUFFER(int i_Buffer_Size) : _Full_Size(i_Buffer_Size), _In(0), _Out(0)
{
	_Buffer = new char[i_Buffer_Size];
	InitializeSRWLock(&_srwlock);
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
	size_t Temp_DirectEnqueueSize;
	size_t Temp_Full_Size = _Full_Size;
	size_t Temp_In = _In;
	size_t Temp_Out = _Out;

	if (Temp_Full_Size - GetUseSize(Temp_In, Temp_Out) <= iSize)
		return 0;

	// (_Buffer + _Full_Size): ������ ���� �� ���� ��ġ
	// (_Buffer + Full_Size) - _In : ���� ��ġ���� ������ ���۱��� ���� �� �ִ� �������� ���� ���´�. 


	if (Temp_Full_Size - GetUseSize(Temp_In, Temp_Out) <= iSize)
		return 0;

	Temp_DirectEnqueueSize = DirectEnqueueSize(Temp_In, Temp_Out);

	if(iSize <= Temp_DirectEnqueueSize)
	{
		memcpy(_Buffer+ Temp_In, pData, iSize);
	}
	else
	{
		memcpy(_Buffer+ Temp_In, pData, Temp_DirectEnqueueSize);
		memcpy(_Buffer, pData + Temp_DirectEnqueueSize, iSize - Temp_DirectEnqueueSize);
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
	size_t Temp_DirectDequeueSize;
	size_t Temp_In = _In;
	size_t Temp_Out = _Out;
	size_t Temp_Use_Size = GetUseSize(_In, _Out);

	if (Temp_Use_Size == 0)
		return 0;

	Temp_DirectDequeueSize = DirectDequeueSize(Temp_In, Temp_Out);

	if (iSize <= Temp_Use_Size)
	{
		if (iSize <= Temp_DirectDequeueSize)
		{
			memcpy(chpDest, _Buffer + Temp_Out, iSize);
		}
		else
		{
			memcpy(chpDest, _Buffer + Temp_Out, Temp_DirectDequeueSize);
			memcpy(chpDest + Temp_DirectDequeueSize, _Buffer, iSize - Temp_DirectDequeueSize);
		}


		_Out = (Temp_Out + iSize) % _Full_Size;


		return iSize;
	}
	else // if(iSize > _Use_Size) ���� �غ��� ���۰� ����ִ� �����ͺ��� �� ũ��. == ��� �����͸� ������.
	{
		//if (iSize <= Temp_DirectDequeueSize)
		if (Temp_Use_Size <= Temp_DirectDequeueSize)
		{
			memcpy(chpDest, _Buffer + Temp_Out, Temp_Use_Size);
		}
		else
		{
			memcpy(chpDest, _Buffer + Temp_Out, Temp_DirectDequeueSize);
			memcpy(chpDest + Temp_DirectDequeueSize, _Buffer, Temp_Use_Size - Temp_DirectDequeueSize);
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
size_t C_RING_BUFFER::Peek(char* chpDest, size_t iSize)
{
	size_t Temp_DirectDequeueSize;
	size_t Temp_In = _In;
	size_t Temp_Out = _Out;
	size_t Temp_Use_Size = GetUseSize(_In, _Out);

	if (Temp_Use_Size == 0)
		return 0;

	Temp_DirectDequeueSize = DirectDequeueSize(Temp_In, Temp_Out);

	if (iSize <= Temp_Use_Size)
	{
		if (iSize <= Temp_DirectDequeueSize)
		{
			memcpy(chpDest, _Buffer + Temp_Out, iSize);
		}
		else
		{
			memcpy(chpDest, _Buffer + Temp_Out, Temp_DirectDequeueSize);
			memcpy(chpDest + Temp_DirectDequeueSize, _Buffer, iSize - Temp_DirectDequeueSize);
		}

		return iSize;
	}
	else // if(iSize > _Use_Size) ���� �غ��� ���۰� ����ִ� �����ͺ��� �� ũ��. == ��� �����͸� ������.
	{
		//if (iSize <= Temp_DirectDequeueSize)
		if (Temp_Use_Size <= Temp_DirectDequeueSize)
		{
			memcpy(chpDest, _Buffer + Temp_Out, Temp_Use_Size);
		}
		else
		{
			memcpy(chpDest, _Buffer + Temp_Out, Temp_DirectDequeueSize);
			memcpy(chpDest + Temp_DirectDequeueSize, _Buffer, Temp_Use_Size - Temp_DirectDequeueSize);
		}

		return Temp_Use_Size;
	}

	return 0;
}

size_t C_RING_BUFFER::PeekAt(char* pOutTarget, size_t out, size_t sizeToPeek)
{
	size_t useSize;
	size_t directPeekSize;
	size_t firstSize;
	size_t secondSize;
	size_t in;
	char* pPeekStartPos;

	in = _In;
	useSize = GetUseSize(in, out);
	if (sizeToPeek > useSize)
	{
		// ����ִ� �����ͺ��� ���� �����͸� �������� �ϸ� �׳� ��ȯ�Ѵ�. 
		return 0;
	}

	directPeekSize = DirectDequeueSize(in, out);
	if (directPeekSize > sizeToPeek) // �߷��� �ι��� ���ļ� ����
		firstSize = sizeToPeek;
	else // �ѹ��� ����
		firstSize = directPeekSize;

	pPeekStartPos = _Buffer + out;
	memcpy(pOutTarget, pPeekStartPos, firstSize);

	secondSize = sizeToPeek - firstSize;
	if (secondSize <= 0)
		return firstSize;

	memcpy(pOutTarget + firstSize, _Buffer, secondSize);
	return firstSize + secondSize;
}



