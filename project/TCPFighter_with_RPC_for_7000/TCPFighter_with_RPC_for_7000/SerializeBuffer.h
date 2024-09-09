
/*---------------------------------------------------------------

	Packet.

	��Ʈ��ũ ��Ŷ�� Ŭ����.
	�����ϰ� ��Ŷ�� ������� ����Ÿ�� In, Out �Ѵ�.

	- ����.

	CPacket cPacket;  or CMessage Message;

	�ֱ�.
	clPacket << 40030;		or	clPacket << iValue;	(int �ֱ�)
	clPacket << 1.4;		or	clPacket << fValue;	(float �ֱ�)


	����.
	clPacket >> iValue;		(int ����)
	clPacket >> byValue;		(BYTE ����)
	clPacket >> fValue;		(float ����)

	CPacket Packet2;

	!.	���ԵǴ� ����Ÿ FIFO ������ �����ȴ�.
		ȯ�� ť�� �ƴϹǷ�, �ֱ�(<<).����(>>) �� ȥ���ؼ� ������� �ʵ��� �Ѵ�



	* ���� ��Ŷ ���ν��������� ó��

	BOOL	netPacketProc_CreateMyCharacter(CPacket *clpPacket)
	{
		DWORD dwSessionID;
		short shX, shY;
		char chHP;
		BYTE byDirection;

//		*clpPacket >> dwSessionID >> byDirection >> shX >> shY >> chHP;


		*clpPacket >> dwSessionID;
		*clpPacket >> byDirection;
		*clpPacket >> shX;
		*clpPacket >> shY;
		*clpPacket >> chHP;

		...
		...
	}


	* ���� �޽���(��Ŷ) �����ο����� ó��

	CPacket MoveStart;
	mpMoveStart(&MoveStart, dir, x, y);
	SendPacket(&MoveStart);


	void	mpMoveStart(CPacket *clpPacket, BYTE byDirection, short shX, short shY)
	{
		st_NETWORK_PACKET_HEADER	stPacketHeader;
		stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
		stPacketHeader.bySize = 5;
		stPacketHeader.byType = dfPACKET_CS_MOVE_START;

		clpPacket->PutData((char *)&stPacketHeader, dfNETWORK_PACKET_HEADER_SIZE);

		*clpPacket << byDirection;
		*clpPacket << shX;
		*clpPacket << shY;

	}

----------------------------------------------------------------*/


//------------------------------------------------------------------------------------------------------------------------------
// ����ȭ ���ۿ� �������� ����� �ʿ��Ѱ�??? 
// �������� ����� ž���Ǹ� �ϴ� ������. 
// 
// ���� �׳� ������ ������ �޽����� �ִ���� ex) �κ��丮 ����, ä�� ���� ����, ��� ���� ���� �޽����� ��� �ʿ����� ��Ȯ�� �𸣰���
//		������ �ɾ�ΰų� Ȥ��, ����, �ʿ��� ��, �����Ͽ� ũ�⸦ Ű���� �ٽ� �����ϴ� ���� �� ���ƺ���
// 
// ���� �޽��� ���̿� ���Ͽ� ������ ���ٸ�, �װ� ���� �ϳ��� ���ݹ���� �� �� �ְ�, ��������� �޽����� �ִ� ���̿� ���� ������ ����� �ȴ�. 
//		�׷� ó������ �޽��� ���̿� ���� ������ �δ°Ͱ� �ٸ��� ����. 
// 
// ����: ���������� �޽����� ���� �� �ִ�. 
// 
// ����: 1. ���� �޽����� ������ ���� �޽��� ũ�⿡ ���缭 �޽����� ��������, �޴����� ��� �ִ� �������� �޸𸮸� �Ҵ��ؾ���.
//		Ȥ�� ���� ũ�� ��� ������� �޽��� ���̸� üũ�Ͽ� �޸𸮸� �Ҵ��ؾ��� ==> �޸� �Ҵ� ������ ������ ==> �޸� Ǯ�� ����ؾ��� 
//		
//		2. ����ü�� ����� ��� �޽����� �ִ� ������ ���� ������ ���ؼ� �׷��� ū ����� ���� �ʾƵ� ���. ������ ����ȭ ���۸� ����ϸ� 
//			�޽����� ������ �����̶� Ʋ���� �ȵȴ�. 
// 
//		3. 
//		
// 
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __SERIALIZEBUFFER_H__
#define __SERIALIZEBUFFER_H__

class SerializeBuffer
{
public:
	enum en_PACKET
	{
		eBUFFER_DEFAULT = 1400,			// ��Ŷ�� �⺻ ���� ������.
		eBUFFER_ReSize_Max_Size = 10000		// �������� �⺻ �ִ� ������.
	};

	//////////////////////////////////////////////////////////////////////////
	// ������, �ı���.
	//
	// Return:
	//////////////////////////////////////////////////////////////////////////
	SerializeBuffer();
	SerializeBuffer(int iBufferSize);

	virtual	~SerializeBuffer();


	//////////////////////////////////////////////////////////////////////////
	// ���� ���� ���ؼ� m_chpWritePos ��ġ�� m_chpReadPos ��ġ�� �����Ѵ�
	// 
	//////////////////////////////////////////////////////////////////////////
	void	ReWrite(void) { m_chpTempPos = m_chpWritePos; m_chpWritePos = m_chpReadPos; }
	//////////////////////////////////////////////////////////////////////////
	// m_chpWritePos��ġ�� ���� ��ġ�� �ǵ�����.
	// 
	//////////////////////////////////////////////////////////////////////////
	void	ReturnPos(void) { m_chpWritePos = m_chpTempPos; }
	//////////////////////////////////////////////////////////////////////////
	// ��Ŷ û��.
	//
	// Parameters: ����.
	// Return: ����.
	//////////////////////////////////////////////////////////////////////////
	void	Clear(void) { m_chpReadPos = m_chpWritePos = m_chpBeginPos; }
	//////////////////////////////////////////////////////////////////////////
	// ���� ������ ���.
	//
	// Parameters: ����.
	// Return: (int)��Ŷ ���� ������ ���.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetBufferSize(void) { return m_iBufferSize; }
	//////////////////////////////////////////////////////////////////////////
	// ���� ������� ������ ���.
	//
	// Parameters: ����.
	// Return: (int)������� ����Ÿ ������.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetDataSize(void) { return m_chpWritePos - m_chpReadPos; }
	//////////////////////////////////////////////////////////////////////////
	// ���� ������� �ʴ� ������ ���.
	//
	// Parameters: ����.
	// Return: (int)������� �ʴ� ����Ÿ ������.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetFreeSize(void) { return m_chpEndPos - m_chpWritePos; }



	//////////////////////////////////////////////////////////////////////////
	// ���� ������ ���.
	//
	// Parameters: ����.
	// Return: (char *)���� ������.
	//////////////////////////////////////////////////////////////////////////
	char* GetBufferPtr(void) { return m_chpBeginPos; }

	//////////////////////////////////////////////////////////////////////////
	// ���� ���� ������ ���.
	//
	// Parameters: ����.
	// Return: (char *)���� ������.
	//////////////////////////////////////////////////////////////////////////
	char* GetBufferWritePtr(void) { return m_chpWritePos; }

	//////////////////////////////////////////////////////////////////////////
	// ���� Pos �̵�. (�����̵��� �ȵ�)
	// GetBufferPtr �Լ��� �̿��Ͽ� �ܺο��� ������ ���� ������ ������ ��� ���. 
	//
	// Parameters: (int) �̵� ������.
	// Return: (int) �̵��� ������.
	//////////////////////////////////////////////////////////////////////////
	size_t			RetMoveWritePos(size_t iSize) { m_chpWritePos += iSize; return iSize; }
	size_t			RetMoveReadPos(size_t iSize) { m_chpReadPos += iSize; return iSize; }
	void		MoveWritePos(size_t iSize) { m_chpWritePos += iSize; }
	void		MoveReadPos(size_t iSize) { m_chpReadPos += iSize; }
	// ���� �ش� �Լ��� �� ����� �����ؾ� �ϴ��� ��Ȯ�� ������ �𸣰ڴ�. 





	/* ============================================================================= */
	// ������ �����ε�
	/* ============================================================================= */
	SerializeBuffer& operator = (SerializeBuffer& clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// �ֱ�.	�� ���� Ÿ�Ը��� ��� ����.
	//////////////////////////////////////////////////////////////////////////
	inline SerializeBuffer& operator << (unsigned char byValue);
	inline SerializeBuffer& operator << (char chValue);

	inline SerializeBuffer& operator << (short shValue);
	inline SerializeBuffer& operator << (unsigned short wValue);

	inline SerializeBuffer& operator << (int iValue);
	inline SerializeBuffer& operator << (unsigned long lValue);
	inline SerializeBuffer& operator << (float fValue);

	inline SerializeBuffer& operator << (__int64 iValue);
	inline SerializeBuffer& operator << (double dValue);


	//////////////////////////////////////////////////////////////////////////
	// ����.	�� ���� Ÿ�Ը��� ��� ����.
	//////////////////////////////////////////////////////////////////////////
	inline SerializeBuffer& operator >> (unsigned char& byValue);
	inline SerializeBuffer& operator >> (char& chValue);

	inline SerializeBuffer& operator >> (short& shValue);
	inline SerializeBuffer& operator >> (unsigned short& wValue);

	inline SerializeBuffer& operator >> (int& iValue);
	inline SerializeBuffer& operator >> (unsigned long& dwValue);
	inline SerializeBuffer& operator >> (float& fValue);

	inline SerializeBuffer& operator >> (__int64& iValue);
	inline SerializeBuffer& operator >> (double& dValue);




	//////////////////////////////////////////////////////////////////////////
	// ����Ÿ ���.
	// 
	// Parameters: (char *)Dest ������. (int)Size.
	// Return: (int)������ ������.	�����Ͱ� ���ٸ� 0
	//////////////////////////////////////////////////////////////////////////
	size_t		GetData(char* chpDest, size_t iSize);

	//////////////////////////////////////////////////////////////////////////
	// ����Ÿ ����.
	//
	// Parameters: (char *)Src ������. (int)SrcSize.
	// Return: (int)������ ������.	�����͸� ���� �ڸ��� ���ٸ�  0
	//////////////////////////////////////////////////////////////////////////
	size_t		PutData(char* chpSrc, size_t iSrcSize);


	//////////////////////////////////////////////////////////////////////////
	// ���� ������ ��������
	//
	// Parameters: (size_t) Size
	// Return: 
	//////////////////////////////////////////////////////////////////////////
	int		ReSize();
	int		ReSize(size_t Size);

	//////////////////////////////////////////////////////////////////////////
	// << ������ ����ϱ� ��, �ѹ��� 
	//
	// Parameters: (size_t) Size
	// Return: 
	//////////////////////////////////////////////////////////////////////////




protected:

	//------------------------------------------------------------
	// ť ó�� �����Ǵ� ����ȭ ���ۿ��� Read(Out), Write(In)
	// �����͸� ��� ���� �� �����..... 
	// �����͸� ��� ������ ���� �ʱⰪ�� ȣ�� ���ص� �ȴ�. 
	// this ������ �پ���.
	//------------------------------------------------------------
	char* m_chpReadPos;
	char* m_chpWritePos;
	char* m_chpTempPos;

	//------------------------------------------------------------
	// ������ End �ּҰ�
	//------------------------------------------------------------
	char* m_chpEndPos;

	//------------------------------------------------------------
	// ���� Begin �ּҰ�.
	//------------------------------------------------------------
	char* m_chpBeginPos;

	//------------------------------------------------------------
	// ���� ������.
	//------------------------------------------------------------
	size_t	m_iBufferSize;

	//------------------------------------------------------------
	// ���� ���ۿ� ������� ������. �̰� �� �վ�� �ϳ�???
	//------------------------------------------------------------
	// size_t	m_iDataSize;

	//------------------------------------------------------------
	// ���� �������� �ִ� ������.
	//------------------------------------------------------------
	size_t m_iReSizeMaxSize;
};


SerializeBuffer& SerializeBuffer::operator<<(unsigned char byValue)
{
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
		// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
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
	// TODO: ���⿡ return ���� �����մϴ�.
	if (m_chpWritePos - m_chpReadPos < sizeof(double))
	{
		throw;
	}

	memcpy(&dValue, m_chpReadPos, sizeof(double));
	m_chpReadPos += sizeof(double);
	return *this;
}




#endif // !__SERIALIZEBUFFER_H__

