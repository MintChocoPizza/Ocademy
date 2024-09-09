
/////////////////////////////////////////////////////////////////////
// www.gamecodi.com						������ master@gamecodi.com
//
//
/////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------

��Ŷ������ ����.


�ڽ��� ĳ���Ϳ� ���� ��Ŷ�� �������� ���� ��, ��� �ڽ��� ����
�׼��� ���԰� ���ÿ� ��Ŷ�� ������ �����ֵ��� �Ѵ�.

- �̵� Ű �Է� �� �̵������� ���԰� ���ÿ� �̵� ��Ŷ�� �������� �Ѵ�.
- ����Ű �Է� �� ���� ������ ���ϸ鼭 ��Ŷ�� ������.
- �浹 ó�� �� �������� ���� ������ �������� ó�� �� �뺸�ϰ� �ȴ�.


---------------------------------------------------------------*/

#ifndef __PACKET_DEFINE_H__
#define __PACKET_DEFINE_H__

constexpr int dfNETWORK_PACKET_CODE = 0x89;

//---------------------------------------------------------------
// ���ݹ���.
//---------------------------------------------------------------
constexpr int dfATTACK1_RANGE_X = 80;
constexpr int dfATTACK2_RANGE_X = 90;
constexpr int dfATTACK3_RANGE_X = 100;
constexpr int dfATTACK1_RANGE_Y = 10;
constexpr int dfATTACK2_RANGE_Y = 10;
constexpr int dfATTACK3_RANGE_Y = 20;

//---------------------------------------------------------------
// ���ݷ�.
//---------------------------------------------------------------
constexpr int dfATTACK1_DAMAGE = 5;
constexpr int dfATTACK2_DAMAGE = 8;
constexpr int dfATTACK3_DAMAGE = 10;




#pragma pack(push, 1)
//---------------------------------------------------------------
// ��Ŷ���.
//
//---------------------------------------------------------------
struct st_PACKET_HEADER
{
	unsigned char	byCode;			// ��Ŷ�ڵ� 0x89 ����.
	unsigned char	bySize;			// ��Ŷ ������.
	unsigned char	byType;			// ��ŶŸ��.
};


#define	dfPACKET_SC_CREATE_MY_CHARACTER			0
//---------------------------------------------------------------
// Ŭ���̾�Ʈ �ڽ��� ĳ���� �Ҵ�		Server -> Client
//
// ������ ���ӽ� ���ʷ� �ްԵǴ� ��Ŷ���� �ڽ��� �Ҵ���� ID ��
// �ڽ��� ���� ��ġ, HP �� �ް� �ȴ�. (ó���� �ѹ� �ް� ��)
// 
// �� ��Ŷ�� ������ �ڽ��� ID,X,Y,HP �� �����ϰ� ĳ���͸� �������Ѿ� �Ѵ�.
//
//	4	-	ID
//	1	-	Direction	(LL / RR)
//	2	-	X
//	2	-	Y
//	1	-	HP
//
//---------------------------------------------------------------
struct st_PACKET_SC_CREATE_MY_CHARACTER
{
	__int32 ID;
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
	char	HP;
};


#define	dfPACKET_SC_CREATE_OTHER_CHARACTER		1
//---------------------------------------------------------------
// �ٸ� Ŭ���̾�Ʈ�� ĳ���� ���� ��Ŷ		Server -> Client
//
// ó�� ������ ���ӽ� �̹� ���ӵǾ� �ִ� ĳ���͵��� ����
// �Ǵ� �����߿� ���ӵ� Ŭ���̾�Ʈ���� ������ ����.
//
//
//	4	-	ID
//	1	-	Direction	(LL / RR)
//	2	-	X
//	2	-	Y
//	1	-	HP
//
//---------------------------------------------------------------
struct st_PACKET_SC_CREATE_OTHER_CHARACTER
{
	__int32 ID;
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
	char	HP;
};


#define	dfPACKET_SC_DELETE_CHARACTER			2
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ						Server -> Client
//
// ĳ������ �������� �Ǵ� ĳ���Ͱ� �׾����� ���۵�.
//
//	4	-	ID
//
//---------------------------------------------------------------
struct st_PACKET_SC_DELETE_CHARACTER
{
	__int32		ID;
};



#define	dfPACKET_CS_MOVE_START					10
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Client -> Server
//
// �ڽ��� ĳ���� �̵����۽� �� ��Ŷ�� ������.
// �̵� �߿��� �� ��Ŷ�� ������ ������, Ű �Է��� ����Ǿ��� ��쿡��
// ������� �Ѵ�.
//
// (���� �̵��� ���� �̵� / ���� �̵��� ���� ���� �̵�... ���)
//
//	1	-	Direction	( ���� ������ �� 8���� ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7

struct st_PACKET_CS_MOVE_START
{
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};




#define	dfPACKET_SC_MOVE_START					11
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Server -> Client
//
// �ٸ� ������ ĳ���� �̵��� �� ��Ŷ�� �޴´�.
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�� �̵�ó���� ���ֵ��� �Ѵ�.
// 
// ��Ŷ ���� �� �ش� Ű�� ����ؼ� ���������� �����ϰ�
// �ش� �������� ��� �̵��� �ϰ� �־�߸� �Ѵ�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ �� 8���� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_SC_MOVE_START
{
	__int32 ID;
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};




#define	dfPACKET_CS_MOVE_STOP					12
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Client -> Server
//
// �̵��� Ű���� �Է��� ��� �����Ǿ��� ��, �� ��Ŷ�� ������ �����ش�.
// �̵��� ���� ��ȯ�ÿ��� ��ž�� ������ �ʴ´�.
//
//	1	-	Direction	( ���� ������ �� ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_CS_MOVE_STOP
{
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};


#define	dfPACKET_SC_MOVE_STOP					13
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Server -> Client
//
// ID �� �ش��ϴ� ĳ���Ͱ� �̵��� ������̹Ƿ� 
// ĳ���͸� ã�Ƽ� �����, ��ǥ�� �Է����ְ� ���ߵ��� ó���Ѵ�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_SC_MOVE_STOP
{
	__int32 ID;
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};



#define	dfPACKET_CS_ATTACK1						20
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y	
//
//---------------------------------------------------------------
struct st_PACKET_CS_ATTACK1
{
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};

#define	dfPACKET_SC_ATTACK1						21
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����1�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_SC_ATTACK1
{
	__int32 ID;
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};



#define	dfPACKET_CS_ATTACK2						22
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_CS_ATTACK2
{
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};

#define	dfPACKET_SC_ATTACK2						23
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����2�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_SC_ATTACK2
{
	__int32 ID;
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};

#define	dfPACKET_CS_ATTACK3						24
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_CS_ATTACK3
{
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};

#define	dfPACKET_SC_ATTACK3						25
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����3�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_SC_ATTACK3
{
	__int32 ID;
	char	Direction;	// (LL / RR)
	short	X;
	short	Y;
};





#define	dfPACKET_SC_DAMAGE						30
//---------------------------------------------------------------
// ĳ���� ������ ��Ŷ							Server -> Client
//
// ���ݿ� ���� ĳ������ ������ ����.
//
//	4	-	AttackID	( ������ ID )
//	4	-	DamageID	( ������ ID )
//	1	-	DamageHP	( ������ HP )
//
//---------------------------------------------------------------
struct st_PACKET_SC_DAMAGE
{
	__int32		AttackID;	// ( ������ ID )
	__int32		DamageID;	// ( ������ ID )
	char		DamageHP;	// ( ������ HP )
};








// ������...
#define	dfPACKET_CS_SYNC						250
//---------------------------------------------------------------
// ����ȭ�� ���� ��Ŷ					Client -> Server
//
//
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_CS_SYNC
{
	short X;
	short Y;
};


#define	dfPACKET_SC_SYNC						251
//---------------------------------------------------------------
// ����ȭ�� ���� ��Ŷ					Server -> Client
//
// �����κ��� ����ȭ ��Ŷ�� ������ �ش� ĳ���͸� ã�Ƽ�
// ĳ���� ��ǥ�� �������ش�.
//
//	4	-	ID
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct st_PACKET_SC_SYNC
{
	__int32		ID;
	short		X;
	short		Y;
};
#pragma pack(pop)

#ifdef SERIALIZEBUFFER
bool SerializeBuffer_netPacketProc_SC_CREATE_MY_CHARACTER(st_SESSION* p_Session, SerializeBuffer* clpPacket);
bool SerializeBuffer_netPacketProc_SC_CREATE_OTHER_CHARACTER(st_SESSION* p_Session, SerializeBuffer* clpPacket);

bool SerializeBuffer_netPacketProc_SC_DELETE_CHARACTER(st_SESSION* p_Session, SerializeBuffer* clpPacket);

bool SerializeBuffer_netPacketProc_CS_MOVE_START(st_SESSION* p_Session, SerializeBuffer* clpPacket);
bool SerializeBuffer_netPacketProc_SC_MOVE_START(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y);

bool SerializeBuffer_netPacketProc_CS_MOVE_STOP(st_SESSION* p_Session, SerializeBuffer* clpPacket);
bool SerializeBuffer_netPacketProc_SC_MOVE_STOP(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y);

bool SerializeBuffer_netPacketProc_CS_ATTACK1(st_SESSION* p_Session, SerializeBuffer* clpPacket);
bool SerializeBuffer_netPacketProc_SC_ATTACK1(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y);

bool SerializeBuffer_netPacketProc_CS_ATTACK2(st_SESSION* p_Session, SerializeBuffer* clpPacket);
bool SerializeBuffer_netPacketProc_SC_ATTACK2(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y);

bool SerializeBuffer_netPacketProc_CS_ATTACK3(st_SESSION* p_Session, SerializeBuffer* clpPacket);
bool SerializeBuffer_netPacketProc_SC_ATTACK3(SerializeBuffer* clpPacket, char Direction, __int32 ID, short X, short Y);

bool  SerializeBuffer_netPacketProc_SC_DAMAGE(SerializeBuffer* clpPacket, __int32 AttackID, __int32 DamageID, char HP);

#else
bool netPacketProc_SC_CREATE_MY_CHARACTER(st_SESSION* p_Session, char* pHeader, char* pPacket);
bool netPacketProc_SC_CREATE_OTHER_CHARACTER(st_SESSION* p_Session, char* pHeader, char* pPacket);

bool netPacketProc_SC_DELETE_CHARACTER(st_SESSION* p_Session, char* pHeader, char* pPacket);

bool netPacketProc_CS_MOVE_START(st_SESSION* p_Session, char* pPacket);
bool netPacketProc_SC_MOVE_START(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y);

bool netPacketProc_CS_MOVE_STOP(st_SESSION* p_Session, char* pPacket);
bool netPacketProc_SC_MOVE_STOP(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y);

bool netPacketProc_CS_ATTACK1(st_SESSION* p_Session, char* pPacket);
bool netPacketProc_SC_ATTACK1(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y);

bool netPacketProc_CS_ATTACK2(st_SESSION* p_Session, char* pPacket);
bool netPacketProc_SC_ATTACK2(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y);

bool netPacketProc_CS_ATTACK3(st_SESSION* p_Session, char* pPacket);
bool netPacketProc_SC_ATTACK3(char* pHeader, char* pPacket, char Direction, __int32 ID, short X, short Y);

bool netPacketProc_SC_DAMAGE(char* pHeader, char* pPacket, __int32 AttackID, __int32 DamageID, char HP);

bool netPacketProc_CS_SYNC(st_SESSION* p_Session, char* pPacket);
bool netPacketProc_SC_SYNC(st_SESSION* p_Session, char* pHeader, char* pPacket);
#endif // SERIALIZEBUFFER








#endif // !__PACKET_DEFINE_H__
