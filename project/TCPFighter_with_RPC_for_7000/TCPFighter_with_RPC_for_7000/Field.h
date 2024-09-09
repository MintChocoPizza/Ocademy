
//--------------------------------------------------------------------------------------------------------------------
// �ʵ� Ŭ������ ���� ����:
// 
// �������� ��ġ�� �ʵ� ������ �����ϴ� ���:
//	1. ��� �������� x,y ��ǥ�� �׸��� ������ ������ �ش��ϴ� �׸��带 ��ġ�� ���س���.  
//		����: ��Ŷ�� ���� �� ����, ��� �������� ������� x,y ��ǥ�� �������� �����Ͽ� �׸��� ��ġ�� ���س��� �Ѵ�. 
// 
//	2. ���� �̵�, ��� �÷��̾��� �������� ���� �� ����, �ش� �׸��忡 ���ԵǴ� �÷��̾���� �����صд� .
//		�÷��̾���� ��ġ�� �ʿ��� �� ����, �׳� �ش� �׸��带 Ž���ϸ� �ش� ��ġ�� �ִ� �÷��̾ �ѹ��� �� �� �ִ�. 
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
	// Ư�� ���� ��ǥ ���� �ֺ� ����� ���� ���
	// SectorX, SectorY�� ������ǥ���� �ֺ� �ִ� 9���� ���� ��ǥ�� ��� ���� �Լ�
	void GetSectorAround(int iSectorX, int iSectorY, st_SECTOR_AROUND* pSectorAound);

	//---------------------------------------------------------------------------------------------
	// �־��� ��ǥ�� ���� ������ �������� ���� ������ �ִ� ���� ��� 
	void GetAttackSectorAround(short shX, short shY, char byDirection, int i_Attack_Range_X, int i_Attack_Range_Y, st_SECTOR_AROUND* pSectorAound);

	//---------------------------------------------------------------------------------------------
	// ĳ���Ͱ� ���͸� �̵� �� �ڿ� ����ǿ��� ���� ����, ���� �߰��� ������ ���� ���ϴ� �Լ�
	void GetUpdateSectorAround(st_PLAYER* pCharacter, st_SECTOR_AROUND* pRemoveSector, st_SECTOR_AROUND* pAddSector);

	//---------------------------------------------------------------------------------------------
	// ���Ͱ� ����� ���� Ȯ���Ѵ�. �׸��� ����Ǿ��ٸ� g_Sector_CList�� �ݿ��Ѵ�.
	bool Sector_UpdateCharacter(st_PLAYER* pPlayer);

	//---------------------------------------------------------------------------------------------
	// ����� ���Ϳ� ���ؼ� �޽����� ������. 
	void CharacterSectorUpdatePacket(st_PLAYER* pPlayer);

	//---------------------------------------------------------------------------------------------
	// g_Sector_CList ���� �÷��̾ �����. 
	void removeUserFromSector(st_PLAYER* pPlayer);

	//---------------------------------------------------------------------------------------------
	// Ư�� ���� 1���� �ִ� Ŭ���̾�Ʈ�� ���� �޽��� ������,
	// ������ �Ű����� ������ �����ϰ�
	void SendPacket_SectorOne(int iSectorX, int iSectorY, SerializeBuffer* pPacket, st_SESSION* pExceptSession);

	//----------------------------------------------------------------
	// Ŭ���̾�Ʈ ���� �ֺ� ���Ϳ� �޽��� ������ (�ִ� 9�� ����)
	void SendPacket_Around(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around, bool bSendMe = false);

	//----------------------------------------------------------------
	// Ŭ���̾�Ʈ ���� �ֺ� ���Ϳ��� Ŭ���̾�Ʈ���� �÷��̾� ���� �޽��� ������(�ִ� 9�� ����)
	void SendPacket_Around_To_Session_CreateCharacter(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around);

	//----------------------------------------------------------------
	// Ŭ���̾�Ʈ ���� �ֺ� ���Ϳ��� Ŭ���̾�Ʈ���� �÷��̾� ���� �޽��� ������ (�ִ� 9�� ����)
	void SendPacket_Around_To_Session_DeleteCharater(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around);

	//----------------------------------------------------------------
	// ���Ϳ� ���� ���� ���� ó���� �Ѵ�. 
	bool Check_Sector_CoordinateRange(int iSectorX, int iSectorY);

	//----------------------------------------------------------------
	// �ش� ���Ϳ� ���� �÷��̾���� ����� ��ȯ�Ѵ�. 
	CList<st_PLAYER*>* GetPlayerInSectorCList(int iSectorX, int iSectorY);

	//----------------------------------------------------------------
	// ó�� ������ �÷��̾ ���Ϳ� ����Ѵ�. 
	void AddPlayerToSector(st_PLAYER* pPlayer);

private:
	C_Field();
	~C_Field();

private:
	static C_Field _C_Field;

public:

	//----------------------------------------------------------------------------------------------
	// C_Field()�� �Ҹ��ڰ� �����ؾ� CList() �Ҹ��ڰ� �����ϰ�,
	// �׷��� �Ҵ���� �޸𸮸� �޸� Ǯ�� �����ش�. 
	// �׷��ٸ� ������ CList()�� �Ҹ��ڰ� ���� �����ؾ� �Ѵ�. 
	OreoPizza::CMemoryPool<CList<st_PLAYER*>::Node> _MemPool;

	// Key: <GridY, GridX>, Value: SessionID
	// std::unordered_multimap<std::pair<int, int>, DWORD> _Grid;
	// �׳� �����Ҵ� �迭�� ����Ͽ� ���ʹ� �������� �����Ѵ�. 
	//std::list<st_PLAYER*>*** _Sector;
	//std::list<st_PLAYER*> g_Sector_List[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

	// ������ ��, ��ȸ�ϱ� �Ⱦ �ؽ� ���
	// �߸��Ǿ���.......
	// std::unordered_map<DWORD, st_PLAYER*> g_Sector_Hash[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

	//----------------------------------------------------------------------------------------------
	// ����
	// �÷��̾ �̵��Կ� ���� �ؽ����̺� ĳ���͸� ���� �� ���� �����Ҵ��� �߻��ϰ� �ȴ�. 
	// �̿����� ������尡 �� Ŭ������ �Ǵܵ�[
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
	// st_SECTOR_POS �ʱ�ȭ
	void Init_SECTOR_POS(int Y, int X);
};

struct st_SECTOR_AROUND
{
	int iCount;
	st_SECTOR_POS Around[9];
};


