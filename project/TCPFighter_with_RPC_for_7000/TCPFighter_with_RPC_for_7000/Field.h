
//--------------------------------------------------------------------------------------------------------------------
// 필드 클래스를 만든 이유:
// 
// 유저들의 위치를 필드 단위로 설정하는 방법:
//	1. 모든 유저들의 x,y 좌표를 그리드 단위로 나누어 해당하는 그리드를 위치를 구해낸다.  
//		단점: 패킷을 보낼 때 마다, 모든 유저들을 대상으로 x,y 좌표를 기준으로 연산하여 그리드 위치를 구해내야 한다. 
// 
//	2. 생성 이동, 등등 플레이어의 움직임이 있을 때 마다, 해당 그리드에 포함되는 플레이어들을 저장해둔다 .
//		플레이어들의 위치가 필요할 때 마다, 그냥 해당 그리드를 탐색하면 해당 위치에 있는 플레이어를 한번에 알 수 있다. 
//--------------------------------------------------------------------------------------------------------------------

#ifndef __FIELD_H__
#define __FIELD_H__

using namespace std;

//#define dfGRID_X_SIZE 50
//#define dfGRID_Y_SIZE 50
#define dfGRID_X_SIZE 160
#define dfGRID_Y_SIZE 120

//#define dfSECTOR_MAX_Y dfRANGE_MOVE_BOTTOM / dfGRID_Y_SIZE
//#define dfSECTOR_MAX_X dfRANGE_MOVE_RIGHT  / dfGRID_X_SIZE
//constexpr int dfSECTOR_MAX_Y = dfRANGE_MOVE_BOTTOM / dfGRID_Y_SIZE;
//constexpr int dfSECTOR_MAX_X = dfRANGE_MOVE_RIGHT / dfGRID_X_SIZE;

struct st_SECTOR_POS;
struct st_SECTOR_AROUND;
struct st_PLAYER;

extern int dY[];
extern int dX[];

class C_Field
{
public:
	enum
	{
		//Grid_X_Size = 100,
		//Grid_Y_Size = 100
		dfSECTOR_MAX_Y = (dfRANGE_MOVE_BOTTOM / dfGRID_Y_SIZE) + 1,
		dfSECTOR_MAX_X = (dfRANGE_MOVE_RIGHT / dfGRID_X_SIZE) + 1
	};

public:
	

public:
	static C_Field* GetInstance(void);

	//---------------------------------------------------------------------------------------------
	// 특정 섹터 좌표 기준 주변 영향권 섹터 얻기
	// SectorX, SectorY의 섹터좌표에서 주변 최대 9개의 섹터 좌표를 얻어 오는 함수
	void GetSectorAround(int iSectorX, int iSectorY, st_SECTOR_AROUND* pSectorAound);

	//---------------------------------------------------------------------------------------------
	// 주어진 좌표와 공격 범위를 기준으로 공격 범위에 있는 섹터 얻기 
	void GetAttackSectorAround(short shX, short shY, char byDirection, int i_Attack_Range_X, int i_Attack_Range_Y, st_SECTOR_AROUND* pSectorAound);

	//---------------------------------------------------------------------------------------------
	// 캐릭터가 섹터를 이동 한 뒤에 영향권에서 빠진 섹터, 새로 추가된 섹터의 정보 구하는 함수
	void GetUpdateSectorAround(st_PLAYER* pCharacter, st_SECTOR_AROUND* pRemoveSector, st_SECTOR_AROUND* pAddSector);

	//---------------------------------------------------------------------------------------------
	// 섹터가 변경된 것을 확인한다. 그리고 변경되었다면 g_Sector_CList에 반영한다.
	bool Sector_UpdateCharacter(st_PLAYER* pPlayer);

	//---------------------------------------------------------------------------------------------
	// 변경된 섹터에 대해서 메시지를 보낸다. 
	void CharacterSectorUpdatePacket(st_PLAYER* pPlayer);

	//---------------------------------------------------------------------------------------------
	// g_Sector_CList 에서 플레이어를 지운다. 
	void removeUserFromSector(st_PLAYER* pPlayer);

	//---------------------------------------------------------------------------------------------
	// 특정 섹터 1개에 있는 클라이언트들 에게 메시지 보내기,
	// 마지막 매개변수 세션을 제외하고
	void SendPacket_SectorOne(int iSectorX, int iSectorY, SerializeBuffer* pPacket, st_SESSION* pExceptSession);

	//----------------------------------------------------------------
	// 클라이언트 기준 주변 섹터에 메시지 보내기 (최대 9개 영역)
	void SendPacket_Around(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around, bool bSendMe = false);

	//----------------------------------------------------------------
	// 클라이언트 기준 주변 섹터에서 클라이언트에게 플레이어 생성 메시지 보내기(최대 9개 영역)
	void SendPacket_Around_To_Session_CreateCharacter(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around);

	//----------------------------------------------------------------
	// 클라이언트 기준 주변 섹터에서 클라이언트에게 플레이어 삭제 메시지 보내기 (최대 9개 영역)
	void SendPacket_Around_To_Session_DeleteCharater(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around);

	//----------------------------------------------------------------
	// 섹터에 대한 범위 예외 처리를 한다. 
	bool Check_Sector_CoordinateRange(int iSectorX, int iSectorY);

	//----------------------------------------------------------------
	// 해당 섹터에 대한 플레이어들의 목록을 반환한다. 
	CList<st_PLAYER*>* GetPlayerInSectorCList(int iSectorX, int iSectorY);

	//----------------------------------------------------------------
	// 처음 생성한 플레이어를 섹터에 등록한다. 
	void AddPlayerToSector(st_PLAYER* pPlayer);

private:
	C_Field();
	~C_Field();

private:
	static C_Field _C_Field;

public:

	//----------------------------------------------------------------------------------------------
	// C_Field()의 소멸자가 동작해야 CList() 소멸자가 동작하고,
	// 그래야 할당받은 메모리를 메모리 풀에 돌려준다. 
	// 그렇다면 무조건 CList()의 소멸자가 먼저 동작해야 한다. 
	OreoPizza::CMemoryPool<CList<st_PLAYER*>::Node> _MemPool;

	// Key: <GridY, GridX>, Value: SessionID
	// std::unordered_multimap<std::pair<int, int>, DWORD> _Grid;
	// 그냥 동적할당 배열을 사용하여 섹터당 유저들을 저장한다. 
	//std::list<st_PLAYER*>*** _Sector;
	//std::list<st_PLAYER*> g_Sector_List[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

	// 삭제할 때, 순회하기 싫어서 해쉬 사용
	// 잘못되었다.......
	// std::unordered_map<DWORD, st_PLAYER*> g_Sector_Hash[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

	//----------------------------------------------------------------------------------------------
	// 최종
	// 플레이어가 이동함에 따라 해쉬테이블에 캐릭터를 넣을 때 마다 동적할당이 발생하게 된다. 
	// 이에대한 오버헤드가 더 클것으로 판단됨[
	CList<st_PLAYER*> g_Sector_CList[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

};

#endif // !__FIELD_H__

struct st_SECTOR_POS
{
	int iX;
	int iY;

	//st_SECTOR_POS();
	//st_SECTOR_POS(int X, int Y);
	//----------------------------------------------------------------
	// st_SECTOR_POS 초기화
	void Init_SECTOR_POS(int Y, int X);
};

struct st_SECTOR_AROUND
{
	int iCount;
	st_SECTOR_POS Around[9];
};


