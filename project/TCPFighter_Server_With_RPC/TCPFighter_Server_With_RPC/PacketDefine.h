
/////////////////////////////////////////////////////////////////////
// www.gamecodi.com						이주행 master@gamecodi.com
//
//
/////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------

패킷데이터 정의.


자신의 캐릭터에 대한 패킷을 서버에게 보낼 때, 모두 자신이 먼저
액션을 취함과 동시에 패킷을 서버로 보내주도록 한다.

- 이동 키 입력 시 이동동작을 취함과 동시에 이동 패킷을 보내도록 한다.
- 공격키 입력 시 공격 동작을 취하면서 패킷을 보낸다.
- 충돌 처리 및 데미지에 대한 정보는 서버에서 처리 후 통보하게 된다.


---------------------------------------------------------------*/

#ifndef __PACKET_DEFINE_H__
#define __PACKET_DEFINE_H__

constexpr int dfNETWORK_PACKET_CODE = 0x89;

//---------------------------------------------------------------
// 공격범위.
//---------------------------------------------------------------
constexpr int dfATTACK1_RANGE_X = 80;
constexpr int dfATTACK2_RANGE_X = 90;
constexpr int dfATTACK3_RANGE_X = 100;
constexpr int dfATTACK1_RANGE_Y = 10;
constexpr int dfATTACK2_RANGE_Y = 10;
constexpr int dfATTACK3_RANGE_Y = 20;

//---------------------------------------------------------------
// 공격력.
//---------------------------------------------------------------
constexpr int dfATTACK1_DAMAGE = 5;
constexpr int dfATTACK2_DAMAGE = 8;
constexpr int dfATTACK3_DAMAGE = 10;




#pragma pack(push, 1)
//---------------------------------------------------------------
// 패킷헤더.
//
//---------------------------------------------------------------
struct st_PACKET_HEADER
{
	unsigned char	byCode;			// 패킷코드 0x89 고정.
	unsigned char	bySize;			// 패킷 사이즈.
	unsigned char	byType;			// 패킷타입.
};


#define	dfPACKET_SC_CREATE_MY_CHARACTER			0
//---------------------------------------------------------------
// 클라이언트 자신의 캐릭터 할당		Server -> Client
//
// 서버에 접속시 최초로 받게되는 패킷으로 자신이 할당받은 ID 와
// 자신의 최초 위치, HP 를 받게 된다. (처음에 한번 받게 됨)
// 
// 이 패킷을 받으면 자신의 ID,X,Y,HP 를 저장하고 캐릭터를 생성시켜야 한다.
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
// 다른 클라이언트의 캐릭터 생성 패킷		Server -> Client
//
// 처음 서버에 접속시 이미 접속되어 있던 캐릭터들의 정보
// 또는 게임중에 접속된 클라이언트들의 생성용 정보.
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
// 캐릭터 삭제 패킷						Server -> Client
//
// 캐릭터의 접속해제 또는 캐릭터가 죽었을때 전송됨.
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
// 캐릭터 이동시작 패킷						Client -> Server
//
// 자신의 캐릭터 이동시작시 이 패킷을 보낸다.
// 이동 중에는 본 패킷을 보내지 않으며, 키 입력이 변경되었을 경우에만
// 보내줘야 한다.
//
// (왼쪽 이동중 위로 이동 / 왼쪽 이동중 왼쪽 위로 이동... 등등)
//
//	1	-	Direction	( 방향 디파인 값 8방향 사용 )
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
// 캐릭터 이동시작 패킷						Server -> Client
//
// 다른 유저의 캐릭터 이동시 본 패킷을 받는다.
// 패킷 수신시 해당 캐릭터를 찾아 이동처리를 해주도록 한다.
// 
// 패킷 수신 시 해당 키가 계속해서 눌린것으로 생각하고
// 해당 방향으로 계속 이동을 하고 있어야만 한다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값 8방향 )
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
// 캐릭터 이동중지 패킷						Client -> Server
//
// 이동중 키보드 입력이 없어서 정지되었을 때, 이 패킷을 서버에 보내준다.
// 이동중 방향 전환시에는 스탑을 보내지 않는다.
//
//	1	-	Direction	( 방향 디파인 값 좌/우만 사용 )
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
// 캐릭터 이동중지 패킷						Server -> Client
//
// ID 에 해당하는 캐릭터가 이동을 멈춘것이므로 
// 캐릭터를 찾아서 방향과, 좌표를 입력해주고 멈추도록 처리한다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
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
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
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
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격1번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
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
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
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
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격2번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
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
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
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
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격3번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
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
// 캐릭터 데미지 패킷							Server -> Client
//
// 공격에 맞은 캐릭터의 정보를 보냄.
//
//	4	-	AttackID	( 공격자 ID )
//	4	-	DamageID	( 피해자 ID )
//	1	-	DamageHP	( 피해자 HP )
//
//---------------------------------------------------------------
struct st_PACKET_SC_DAMAGE
{
	__int32		AttackID;	// ( 공격자 ID )
	__int32		DamageID;	// ( 피해자 ID )
	char		DamageHP;	// ( 피해자 HP )
};








// 사용안함...
#define	dfPACKET_CS_SYNC						250
//---------------------------------------------------------------
// 동기화를 위한 패킷					Client -> Server
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
// 동기화를 위한 패킷					Server -> Client
//
// 서버로부터 동기화 패킷을 받으면 해당 캐릭터를 찾아서
// 캐릭터 좌표를 보정해준다.
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
