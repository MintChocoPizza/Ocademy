
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
	int		GetBufferSize(void) { return m_iBufferSize; }
	//////////////////////////////////////////////////////////////////////////
	// ���� ������� ������ ���.
	//
	// Parameters: ����.
	// Return: (int)������� ����Ÿ ������.
	//////////////////////////////////////////////////////////////////////////
	int		GetDataSize(void) { return m_chpWritePos - m_chpReadPos; }
	//////////////////////////////////////////////////////////////////////////
	// ���� ������� �ʴ� ������ ���.
	//
	// Parameters: ����.
	// Return: (int)������� �ʴ� ����Ÿ ������.
	//////////////////////////////////////////////////////////////////////////
	int		GetFreeSize(void) { return m_chpEndPos - m_chpWritePos; }



	//////////////////////////////////////////////////////////////////////////
	// ���� ������ ���.
	//
	// Parameters: ����.
	// Return: (char *)���� ������.
	//////////////////////////////////////////////////////////////////////////
	char* GetBufferPtr(void) { return m_chpBeginPos; }

	//////////////////////////////////////////////////////////////////////////
	// ���� Pos �̵�. (�����̵��� �ȵ�)
	// GetBufferPtr �Լ��� �̿��Ͽ� �ܺο��� ������ ���� ������ ������ ��� ���. 
	//
	// Parameters: (int) �̵� ������.
	// Return: (int) �̵��� ������.
	//////////////////////////////////////////////////////////////////////////
	int			RetMoveWritePos(int iSize) { m_chpWritePos += iSize; return iSize; }
	int			RetMoveReadPos(int iSize) { m_chpReadPos += iSize; return iSize; }
	void		MoveWritePos(int iSize) { m_chpWritePos += iSize; }
	void		MoveReadPos(int iSize) { m_chpReadPos += iSize; }
	// ���� �ش� �Լ��� �� ����� �����ؾ� �ϴ��� ��Ȯ�� ������ �𸣰ڴ�. 





	/* ============================================================================= */
	// ������ �����ε�
	/* ============================================================================= */
	SerializeBuffer& operator = (SerializeBuffer& clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// �ֱ�.	�� ���� Ÿ�Ը��� ��� ����.
	//////////////////////////////////////////////////////////////////////////
	SerializeBuffer& operator << (unsigned char byValue);
	SerializeBuffer& operator << (char chValue);

	SerializeBuffer& operator << (short shValue);
	SerializeBuffer& operator << (unsigned short wValue);

	SerializeBuffer& operator << (int iValue);
	SerializeBuffer& operator << (unsigned long lValue);
	SerializeBuffer& operator << (float fValue);

	SerializeBuffer& operator << (__int64 iValue);
	SerializeBuffer& operator << (double dValue);


	//////////////////////////////////////////////////////////////////////////
	// ����.	�� ���� Ÿ�Ը��� ��� ����.
	//////////////////////////////////////////////////////////////////////////
	SerializeBuffer& operator >> (unsigned char& byValue);
	SerializeBuffer& operator >> (char& chValue);

	SerializeBuffer& operator >> (short& shValue);
	SerializeBuffer& operator >> (unsigned short& wValue);

	SerializeBuffer& operator >> (int& iValue);
	SerializeBuffer& operator >> (unsigned long& dwValue);
	SerializeBuffer& operator >> (float& fValue);

	SerializeBuffer& operator >> (__int64& iValue);
	SerializeBuffer& operator >> (double& dValue);




	//////////////////////////////////////////////////////////////////////////
	// ����Ÿ ���.
	// 
	// Parameters: (char *)Dest ������. (int)Size.
	// Return: (int)������ ������.	�����Ͱ� ���ٸ� 0
	//////////////////////////////////////////////////////////////////////////
	int		GetData(char* chpDest, size_t iSize);

	//////////////////////////////////////////////////////////////////////////
	// ����Ÿ ����.
	//
	// Parameters: (char *)Src ������. (int)SrcSize.
	// Return: (int)������ ������.	�����͸� ���� �ڸ��� ���ٸ�  0
	//////////////////////////////////////////////////////////////////////////
	int		PutData(char* chpSrc, size_t iSrcSize);


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



#endif // !__SERIALIZEBUFFER_H__

