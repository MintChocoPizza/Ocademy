
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
//		2. ����ü�� ����� ��� �޽����� �ִ� ������ ���� ������ ���ؼ� �׷��� ū ����� ���� �ʾƵ� ��? ������ ����ȭ ���۸� ����ϸ� 
//			�޽����� ������ �����̶� Ʋ���� �ȵȴ�. 
// 
//		3. 
//		
// 
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __PACKET_H__
#define __PACKET_H__

#include <Windows.h>
#include "CMemoryPool.h"
#include "NetworkHeader.h"

class Packet
{
public:
	enum en_PACKET
	{
		eBUFFER_DEFAULT = 1400,			// ��Ŷ�� �⺻ ���� ������.
		eBUFFER_ReSize_Max_Size = 10000		// �������� �⺻ �ִ� ������.
	};


	virtual	~Packet();


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
	void	Clear(void) { m_chpReadPos = m_chpWritePos = m_chpPayloadBeginPos; }
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
	size_t		GetPayloadSize(void) { return m_chpWritePos - m_chpReadPos; }
	//////////////////////////////////////////////////////////////////////////
	// ���� ������� �ʴ� ������ ���.
	//
	// Parameters: ����.
	// Return: (int)������� �ʴ� ����Ÿ ������.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetPayloadFreeSize(void) { return m_chpEndPos - m_chpWritePos; }

	//////////////////////////////////////////////////////////////////////////
	// ���� ������� ��� + ���̷ε� ������ ��ȯ
	// Parameters: ����.
	// Return: (int)���� ������� ��� + ���̷ε� ������ ��ȯ ������.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetUseSize(void) { return m_chpWritePos - m_chpHeaderBeginPos; }


	//////////////////////////////////////////////////////////////////////////
	// ���� ������ �б� ������ ���.
	//
	// Parameters: ����.
	// Return: (char *)���� ������.
	//////////////////////////////////////////////////////////////////////////
	char* GetPayloadBeginPtr(void) { return m_chpPayloadBeginPos; }

	//////////////////////////////////////////////////////////////////////////
	// ���� ������ �б� ������ ���.
	//
	// Parameters: ����.
	// Return: (char *)���� ������.
	//////////////////////////////////////////////////////////////////////////
	char* GetPayloadReadPtr(void) { return m_chpReadPos; }
	
	//////////////////////////////////////////////////////////////////////////
	// ���� ���� ������ ���.
	//
	// Parameters: ����.
	// Return: (char *)���� ������.
	//////////////////////////////////////////////////////////////////////////
	char* GetPayloadWritePtr(void) { return m_chpWritePos; }

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
	Packet& operator = (Packet& clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// �ֱ�.	�� ���� Ÿ�Ը��� ��� ����.
	//////////////////////////////////////////////////////////////////////////
	Packet& operator << (unsigned char byValue);
	Packet& operator << (char chValue);

	Packet& operator << (short shValue);
	Packet& operator << (unsigned short wValue);

	Packet& operator << (int iValue);
	Packet& operator << (unsigned long lValue);
	Packet& operator << (float fValue);

	Packet& operator << (__int64 iValue);
	Packet& operator << (double dValue);


	//////////////////////////////////////////////////////////////////////////
	// ����.	�� ���� Ÿ�Ը��� ��� ����.
	//////////////////////////////////////////////////////////////////////////
	Packet& operator >> (unsigned char& byValue);
	Packet& operator >> (char& chValue);

	Packet& operator >> (short& shValue);
	Packet& operator >> (unsigned short& wValue);

	Packet& operator >> (int& iValue);
	Packet& operator >> (unsigned long& dwValue);
	Packet& operator >> (float& fValue);

	Packet& operator >> (__int64& iValue);
	Packet& operator >> (double& dValue);




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


	static Packet* Alloc();
	static void Free(Packet* pPacket);
	static void ReleasePacketPool();



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
	// ���� Begin �ּҰ�.
	//------------------------------------------------------------
	char* m_chpPayloadBeginPos;

	//------------------------------------------------------------
	// ������ End �ּҰ�
	//------------------------------------------------------------
	char* m_chpEndPos; 

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


private:
	template<class DATA> friend class OreoPizza::CMemoryPool;
	template<class DATA> friend struct OreoPizza::st_BLOCK_NODE;

	//////////////////////////////////////////////////////////////////////////
	// ������, �ı���.
	//
	// Return:
	//////////////////////////////////////////////////////////////////////////
	Packet();
	Packet(int iBufferSize);

	//////////////////////////////////////////////////////////////////////////
	// ���� ��� ������ ��ȯ
	// 
	// Parameters: ����.
	// Return: (int)���� ��� ������ ��ȯ ������.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetHeaderSize(void) { return sizeof(st_NETWORKHEADER); }

	//////////////////////////////////////////////////////////////////////////
	// ����� ������ �þ�� �߰��ؾ� �Ѵ�.
	// 
	// Parameters: 
	// Return: 
	//////////////////////////////////////////////////////////////////////////
	size_t		PutHeader(void);

	//////////////////////////////////////////////////////////////////////////
	// ���� ��� ���� ������ ���.
	//
	// Parameters: ����.
	// Return: (char *)���� ������.
	//////////////////////////////////////////////////////////////////////////
	char* GetHeaderBeginPtr(void) { return m_chpHeaderBeginPos; }

	//------------------------------------------------------------
	// ������ Header �ּҰ�
	//------------------------------------------------------------
	char* m_chpHeaderBeginPos;
};



#endif // !__PACKET_H__

