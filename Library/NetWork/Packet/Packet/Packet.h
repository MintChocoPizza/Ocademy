
/*---------------------------------------------------------------

	Packet.

	네트워크 패킷용 클래스.
	간편하게 패킷에 순서대로 데이타를 In, Out 한다.

	- 사용법.

	CPacket cPacket;  or CMessage Message;

	넣기.
	clPacket << 40030;		or	clPacket << iValue;	(int 넣기)
	clPacket << 1.4;		or	clPacket << fValue;	(float 넣기)


	빼기.
	clPacket >> iValue;		(int 빼기)
	clPacket >> byValue;		(BYTE 빼기)
	clPacket >> fValue;		(float 빼기)

	CPacket Packet2;

	!.	삽입되는 데이타 FIFO 순서로 관리된다.
		환형 큐는 아니므로, 넣기(<<).빼기(>>) 를 혼합해서 사용하지 않도록 한다



	* 실제 패킷 프로시저에서의 처리

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


	* 실제 메시지(패킷) 생성부에서의 처리

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
// 직렬화 버퍼에 리사이즈 기능이 필요한가??? 
// 리사이즈 기능이 탑제되면 일단 느리다. 
// 
// 또한 그냥 컨텐츠 적으로 메시지의 최대길이 ex) 인벤토리 갯수, 채팅 길이 말고, 사실 가변 길이 메시지가 어디에 필요한지 정확히 모르겠음
//		제한을 걸어두거나 혹은, 추후, 필요할 때, 수정하여 크기를 키워서 다시 빌드하는 것이 더 좋아보임
// 
// 또한 메시지 길이에 대하여 제한이 없다면, 그것 또한 하나의 공격방법이 될 수 있고, 결과적으로 메시지의 최대 길이에 대한 제한이 생기게 된다. 
//		그럼 처음부터 메시지 길이에 대해 제한을 두는것과 다를바 없다. 
// 
// 장점: 가변길이의 메시지를 만들 수 있다. 
// 
// 단점: 1. 현재 메시지를 보내는 쪽은 메시지 크기에 맞춰서 메시지를 보내지만, 받는쪽의 경우 최대 사이즈의 메모리를 할당해야함.
//		혹은 고정 크기 헤더 사이즈에서 메시지 길이를 체크하여 메모리를 할당해야함 ==> 메모리 할당 해지가 느리다 ==> 메모리 풀을 사용해야함 
//		
//		2. 구조체를 사용할 경우 메시지를 넣는 순서와 빼는 순서에 대해서 그렇게 큰 고민을 하지 않아도 됬? 하지만 직렬화 버퍼를 사용하면 
//			메시지의 순서를 조금이라도 틀리면 안된다. 
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
		eBUFFER_DEFAULT = 1400,			// 패킷의 기본 버퍼 사이즈.
		eBUFFER_ReSize_Max_Size = 10000		// 링버퍼의 기본 최대 사이즈.
	};


	virtual	~Packet();


	//////////////////////////////////////////////////////////////////////////
	// 새로 쓰기 위해서 m_chpWritePos 위치를 m_chpReadPos 위치로 변경한다
	// 
	//////////////////////////////////////////////////////////////////////////
	void	ReWrite(void) { m_chpTempPos = m_chpWritePos; m_chpWritePos = m_chpReadPos; }
	//////////////////////////////////////////////////////////////////////////
	// m_chpWritePos위치를 원래 위치로 되돌린다.
	// 
	//////////////////////////////////////////////////////////////////////////
	void	ReturnPos(void) { m_chpWritePos = m_chpTempPos; }



	//////////////////////////////////////////////////////////////////////////
	// 패킷 청소.
	//
	// Parameters: 없음.
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	void	Clear(void) { m_chpReadPos = m_chpWritePos = m_chpPayloadBeginPos; }
	//////////////////////////////////////////////////////////////////////////
	// 버퍼 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)패킷 버퍼 사이즈 얻기.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetBufferSize(void) { return m_iBufferSize; }
	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 데이타 사이즈.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetPayloadSize(void) { return m_chpWritePos - m_chpReadPos; }
	//////////////////////////////////////////////////////////////////////////
	// 현재 사용하지 않는 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용하지 않는 데이타 사이즈.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetPayloadFreeSize(void) { return m_chpEndPos - m_chpWritePos; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 헤더 + 페이로드 사이즈 반환
	// Parameters: 없음.
	// Return: (int)현재 사용중인 헤더 + 페이로드 사이즈 반환 사이즈.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetUseSize(void) { return m_chpWritePos - m_chpHeaderBeginPos; }


	//////////////////////////////////////////////////////////////////////////
	// 버퍼 데이터 읽기 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	char* GetPayloadBeginPtr(void) { return m_chpPayloadBeginPos; }

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 데이터 읽기 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	char* GetPayloadReadPtr(void) { return m_chpReadPos; }
	
	//////////////////////////////////////////////////////////////////////////
	// 버퍼 쓰기 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	char* GetPayloadWritePtr(void) { return m_chpWritePos; }

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 Pos 이동. (음수이동은 안됨)
	// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
	//
	// Parameters: (int) 이동 사이즈.
	// Return: (int) 이동된 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int			RetMoveWritePos(int iSize) { m_chpWritePos += iSize; return iSize; }
	int			RetMoveReadPos(int iSize) { m_chpReadPos += iSize; return iSize; }
	void		MoveWritePos(int iSize) { m_chpWritePos += iSize; }
	void		MoveReadPos(int iSize) { m_chpReadPos += iSize; }
	// 현재 해당 함수가 왜 사이즈를 리턴해야 하는지 정확한 이유를 모르겠다. 





	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	Packet& operator = (Packet& clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// 넣기.	각 변수 타입마다 모두 만듬.
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
	// 빼기.	각 변수 타입마다 모두 만듬.
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
	// 데이타 얻기.
	// 
	// Parameters: (char *)Dest 포인터. (int)Size.
	// Return: (int)복사한 사이즈.	데이터가 없다면 0
	//////////////////////////////////////////////////////////////////////////
	size_t		GetData(char* chpDest, size_t iSize);

	//////////////////////////////////////////////////////////////////////////
	// 데이타 삽입.
	//
	// Parameters: (char *)Src 포인터. (int)SrcSize.
	// Return: (int)복사한 사이즈.	데이터를 넣을 자리가 없다면  0
	//////////////////////////////////////////////////////////////////////////
	size_t		PutData(char* chpSrc, size_t iSrcSize);


	//////////////////////////////////////////////////////////////////////////
	// 버퍼 사이즈 리사이즈
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
	// 큐 처럼 구현되는 직렬화 버퍼에서 Read(Out), Write(In)
	// 포인터를 들고 있을 지 고민중..... 
	// 포인터를 들고 있으면 버퍼 초기값을 호출 안해도 된다. 
	// this 연산이 줄어든다.
	//------------------------------------------------------------
	char* m_chpReadPos;
	char* m_chpWritePos;
	char* m_chpTempPos;

	//------------------------------------------------------------
	// 버퍼 Begin 주소값.
	//------------------------------------------------------------
	char* m_chpPayloadBeginPos;

	//------------------------------------------------------------
	// 버퍼의 End 주소값
	//------------------------------------------------------------
	char* m_chpEndPos; 

	//------------------------------------------------------------
	// 버퍼 사이즈.
	//------------------------------------------------------------
	size_t	m_iBufferSize;

	//------------------------------------------------------------
	// 현재 버퍼에 사용중인 사이즈. 이게 꼭 잇어야 하나???
	//------------------------------------------------------------
	// size_t	m_iDataSize;

	//------------------------------------------------------------
	// 버퍼 리사이즈 최대 사이즈.
	//------------------------------------------------------------
	size_t m_iReSizeMaxSize;


private:
	template<class DATA> friend class OreoPizza::CMemoryPool;
	template<class DATA> friend struct OreoPizza::st_BLOCK_NODE;

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Return:
	//////////////////////////////////////////////////////////////////////////
	Packet();
	Packet(int iBufferSize);

	//////////////////////////////////////////////////////////////////////////
	// 현재 헤더 사이즈 반환
	// 
	// Parameters: 없음.
	// Return: (int)현재 헤더 사이즈 반환 사이즈.
	//////////////////////////////////////////////////////////////////////////
	size_t		GetHeaderSize(void) { return sizeof(st_NETWORKHEADER); }

	//////////////////////////////////////////////////////////////////////////
	// 헤더에 정보가 늘어나면 추가해야 한다.
	// 
	// Parameters: 
	// Return: 
	//////////////////////////////////////////////////////////////////////////
	size_t		PutHeader(void);

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 헤더 시작 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	char* GetHeaderBeginPtr(void) { return m_chpHeaderBeginPos; }

	//------------------------------------------------------------
	// 버퍼의 Header 주소값
	//------------------------------------------------------------
	char* m_chpHeaderBeginPos;
};



#endif // !__PACKET_H__

