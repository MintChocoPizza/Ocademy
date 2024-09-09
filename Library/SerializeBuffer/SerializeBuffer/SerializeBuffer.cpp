
#include <Windows.h>
#include "SerializeBuffer.h"

SerializeBuffer::SerializeBuffer()
{
	m_chpBeginPos = new char[eBUFFER_DEFAULT];
	m_iBufferSize = eBUFFER_DEFAULT;

	m_chpReadPos = m_chpBeginPos;
	m_chpWritePos = m_chpBeginPos;
	m_chpTempPos = m_chpBeginPos;
	m_chpEndPos = m_chpBeginPos + eBUFFER_DEFAULT;

	m_iReSizeMaxSize = eBUFFER_ReSize_Max_Size;
}

SerializeBuffer::SerializeBuffer(int iBufferSize)
{
	m_chpBeginPos = new char[iBufferSize];
	m_iBufferSize = iBufferSize;

	m_chpReadPos = m_chpBeginPos;
	m_chpWritePos = m_chpBeginPos;
	m_chpTempPos = m_chpBeginPos;
	m_chpEndPos = m_chpBeginPos + iBufferSize;

	m_iReSizeMaxSize = eBUFFER_ReSize_Max_Size;
}

SerializeBuffer::~SerializeBuffer()
{
	delete[] m_chpBeginPos;
}

SerializeBuffer& SerializeBuffer::operator=(SerializeBuffer& clSrcPacket)
{
	// TODO: 여기에 return 문을 삽입합니다.

	return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(unsigned char byValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpEndPos - m_chpWritePos < sizeof(unsigned char))
	{
		ReSize();
	}


	memcpy(m_chpWritePos, &byValue, sizeof(unsigned char));
	m_chpWritePos += sizeof(unsigned char);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(char chValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpEndPos - m_chpWritePos < sizeof(char))
	{
		ReSize();
	}

	memcpy(m_chpWritePos, &chValue, sizeof(char));
	m_chpWritePos += sizeof(char);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(short shValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpEndPos - m_chpWritePos < sizeof(short))
	{
		ReSize();
	}

	memcpy(m_chpWritePos, &shValue, sizeof(short));
	m_chpWritePos += sizeof(short);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(unsigned short wValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpEndPos - m_chpWritePos < sizeof(unsigned short))
	{
		ReSize();
	}

	memcpy(m_chpWritePos, &wValue, sizeof(unsigned short));
	m_chpWritePos += sizeof(unsigned short);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(int iValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpEndPos - m_chpWritePos < sizeof(int))
	{
		ReSize();
	}

	memcpy(m_chpWritePos, &iValue, sizeof(int));
	m_chpWritePos += sizeof(int);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(unsigned long lValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpEndPos - m_chpWritePos < sizeof(unsigned long))
	{
		ReSize();
	}

	memcpy(m_chpWritePos, &lValue, sizeof(unsigned long));
	m_chpWritePos += sizeof(unsigned long);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(float fValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpEndPos - m_chpWritePos < sizeof(float))
	{
		ReSize();
	}

	memcpy(m_chpWritePos, &fValue, sizeof(float));
	m_chpWritePos += sizeof(float);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(__int64 iValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpEndPos - m_chpWritePos < sizeof(__int64))
	{
		ReSize();
	}

	memcpy(m_chpWritePos, &iValue, sizeof(__int64));
	m_chpWritePos += sizeof(__int64);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(double dValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpEndPos - m_chpWritePos < sizeof(double))
	{
		ReSize();
	}

	memcpy(m_chpWritePos, &dValue, sizeof(double));
	m_chpWritePos += sizeof(double);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(unsigned char& byValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpWritePos - m_chpReadPos < sizeof(unsigned char))
	{
		throw;
	}

	memcpy(&byValue, m_chpReadPos, sizeof(unsigned char));
	m_chpReadPos += sizeof(unsigned char);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(char& chValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpWritePos - m_chpReadPos < sizeof(char))
	{
		throw;
	}

	memcpy(&chValue, m_chpReadPos, sizeof(char));
	m_chpReadPos += sizeof(char);

	return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(short& shValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpWritePos - m_chpReadPos < sizeof(short))
	{
		throw;
	}

	memcpy(&shValue, m_chpReadPos, sizeof(short));
	m_chpReadPos += sizeof(short);
	return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(unsigned short& wValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
		// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpWritePos - m_chpReadPos < sizeof(unsigned short))
	{
		throw;
	}

	memcpy(&wValue, m_chpReadPos, sizeof(unsigned short));
	m_chpReadPos += sizeof(unsigned short);
	return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(int& iValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpWritePos - m_chpReadPos < sizeof(int))
	{
		throw;
	}

	memcpy(&iValue, m_chpReadPos, sizeof(int));
	m_chpReadPos += sizeof(int);
	return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(unsigned long& dwValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpWritePos - m_chpReadPos < sizeof(unsigned long))
	{
		throw;
	}

	memcpy(&dwValue, m_chpReadPos, sizeof(unsigned long));
	m_chpReadPos += sizeof(unsigned long);
	return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(float& fValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpWritePos - m_chpReadPos < sizeof(float))
	{
		throw;
	}

	memcpy(&fValue, m_chpReadPos, sizeof(float));
	m_chpReadPos += sizeof(float);
	return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(__int64& iValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpWritePos - m_chpReadPos < sizeof(__int64))
	{
		throw;
	}

	memcpy(&iValue, m_chpReadPos, sizeof(__int64));
	m_chpReadPos += sizeof(__int64);
	return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(double& dValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_chpWritePos - m_chpReadPos < sizeof(double))
	{
		throw;
	}

	memcpy(&dValue, m_chpReadPos, sizeof(double));
	m_chpReadPos += sizeof(double);
	return *this;
}

size_t SerializeBuffer::GetData(char* chpDest, size_t iSize)
{
	if (m_chpWritePos - m_chpReadPos < (unsigned)iSize)
		return 0;

	memcpy(chpDest, m_chpReadPos, iSize);
	m_chpReadPos += iSize;
	return iSize;
}

size_t SerializeBuffer::PutData(char* chpSrc, size_t iSrcSize)
{
	// 해당 함수에서 넣을 수 있는 사이즈를 계산하나 
	// 다른 함수를 호출하여 넣을 수 있는 사이즈를 계산하나 
	// 조삼모사 이다.
	// 매번 use, free를 호출하여 갱신하는것과
	// size - use를 계산하는 것과 큰 차이는 없어보인다. 
	// 오히려 Free 를 매번 계산하는 것보다 이게 더 빠르다. 
	// 저장하는데 this호출이 2번 일어나기 때문....

	// this 콜을 지역변수로 고치는거 또한 어셈블리의 수가 같다. 
	if (m_chpEndPos - m_chpWritePos < (unsigned)iSrcSize)
	{
		// FreeSize보다 iSrcSize가 큰 경우 리사이즈를 한다. 
		// 그리고 직렬화 버퍼의 바이트를 로그로 남긴다. 

		ReSize();
	}

	memcpy(m_chpWritePos, chpSrc, iSrcSize);
	m_chpWritePos += iSrcSize;
	return iSrcSize;
}

int SerializeBuffer::ReSize()
{
	size_t NewBufferSize;
	size_t ReadPosIndex;
	size_t WritePosIndex;
	char* chpBeginPos;

	NewBufferSize = m_iBufferSize * 2;

	if (NewBufferSize >= m_iReSizeMaxSize)
	{
		// 최대 사이즈보다 큰걸 요구할 경우 그냥 뻑낸다.
		__debugbreak();
	}

	chpBeginPos = m_chpBeginPos;
	// 리사이즈 하면 할 일 
	// 1. 리사이즈 최대 크기보다 작은지 확인.
	// 2. 리사이즈 후 원래 있던 메모리를 새로운 메모리로 복사
	char* chpNewSize = new char[NewBufferSize];
	// memcpy(chpNewSize, chpBeginPos, NewBufferSize >> 2);
	memcpy(chpNewSize, chpBeginPos, m_iBufferSize);
	// 3. 인덱스 대신 사용하던 Pos들 갱신해야함
	// ReSize 하는 상황이 가끔 나와서 그냥 감당함.
	// 4. 기존 메모리 삭제

	ReadPosIndex = m_chpReadPos - chpBeginPos;
	WritePosIndex = m_chpWritePos - chpBeginPos;


	m_chpReadPos = chpNewSize + ReadPosIndex;
	m_chpWritePos = chpNewSize + WritePosIndex;
	m_chpEndPos = chpNewSize + NewBufferSize;
	m_chpBeginPos = chpNewSize;
	m_iBufferSize = NewBufferSize;

	delete[] chpBeginPos;

	return 0;
}

int SerializeBuffer::ReSize(size_t Size)
{
	size_t NewBufferSize;
	size_t ReadPosIndex;
	size_t WritePosIndex;
	char* chpBeginPos;

	NewBufferSize = m_iBufferSize + Size;

	if (NewBufferSize >= m_iReSizeMaxSize)
	{
		// 최대 사이즈보다 큰걸 요구할 경우 그냥 뻑낸다.
		__debugbreak();
	}

	chpBeginPos = m_chpBeginPos;
	// 리사이즈 하면 할 일 
	// 1. 리사이즈 최대 크기보다 작은지 확인.
	// 2. 리사이즈 후 원래 있던 메모리를 새로운 메모리로 복사
	char* chpNewSize = new char[NewBufferSize];
	// memcpy(chpNewSize, chpBeginPos, NewBufferSize - Size);
	memcpy(chpNewSize, chpBeginPos, m_iBufferSize);
	// 3. 인덱스 대신 사용하던 Pos들 갱신해야함
	// ReSize 하는 상황이 가끔 나와서 그냥 감당함.
	// 4. 기존 메모리 삭제

	ReadPosIndex = m_chpReadPos - chpBeginPos;
	WritePosIndex = m_chpWritePos - chpBeginPos;


	m_chpReadPos = chpNewSize + ReadPosIndex;
	m_chpWritePos = chpNewSize + WritePosIndex;
	m_chpEndPos = chpNewSize + NewBufferSize;
	m_chpBeginPos = chpNewSize;
	m_iBufferSize = NewBufferSize;

	delete[] chpBeginPos;

	return 0;
}


