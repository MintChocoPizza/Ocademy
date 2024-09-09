
#ifndef __SESSION_H__
#define __SESSION_H__

#define dfMAX_SESSION 8001

struct st_SECTOR_AROUND;
extern DWORD g_dwPreTime;

//-------------------------------------------------------------------------------
// ��Ʈ��ũ 1���� �����ؾ��� 
//-------------------------------------------------------------------------------
struct st_SESSION
{
	SOCKET				Socket;				// �� ������ TCP ���� 
	
	DWORD				dwSessionID;		// �������� ���� ���� ID
	
	C_RING_BUFFER*		RecvQ;				// ���� ť;
	C_RING_BUFFER*		SendQ;				// �۽� ť; 

	DWORD				dwLastRecvTime;		// �޽��� ���� üũ�� ���� �ð� (Ÿ�Ӿƿ���)

	//bool				Disconnect;			// ���� ���� üũ

#ifdef DISCONNECT_SERVER
	DWORD				dwCreateTime;		// ���� ������ ������ �ð�
	DWORD				dwLastSelectSetTime;// ����Ʈ�� ���� �� �ð�
	DWORD				dwLastFD_ISSETTime;	// ����Ʈ �� ������ �ð�
#endif // DISCONNECT_SERVER


	st_SESSION();
	~st_SESSION();
	//st_SESSION(SOCKET New_Socket, DWORD dw_New_SessionID);
	void Init(SOCKET New_Socket, DWORD dw_New_SessionID);


};

void ForwardDecl(int DestID, SerializeBuffer* sb);

void SetSession(void);

st_SESSION* FindSession(DWORD dwSessionID);
void DeleteSession(DWORD dwSessionID);

void netStartUp(void);
void netCleanUp(void);
void netIOProcess(void);
void netProc_Accept(void);
int netProc_Recv(st_SESSION* pSession);
void netProc_Send(st_SESSION* pSession);
void netHeartbeat(DWORD CurTime);

//----------------------------------------------------------------
// Ư�� 1���� Ŭ���̾�Ʈ ���� �޽��� ������
void SendPacket_Unicast(st_SESSION* pSession, SerializeBuffer* pPacket);

bool PacketProc(st_SESSION* pSession, unsigned char byPacketType, SerializeBuffer* pPacket);
bool netPacketProc_Movestart(st_SESSION* pSession, SerializeBuffer* pPacket);
bool netPacketProc_MoveStop(st_SESSION* pSession, SerializeBuffer* pPacket);
bool netPacketProc_Attack1(st_SESSION* pSession, SerializeBuffer* pPacket);
bool netPacketProc_Attack2(st_SESSION* pSession, SerializeBuffer* pPacket);
bool netPacketProc_Attack3(st_SESSION* pSession, SerializeBuffer* pPacket);
bool netPacketProc_Echo(st_SESSION* pSession, SerializeBuffer* pPacket);

void mpSync(SerializeBuffer* pPacket, DWORD dwSessionID, short shX, short shY);
void mpMoveStart(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY);
void mpMoveStop(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY);
void mpCreateMyCharacter(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY, char HP);
void mpCreateOtherCharacter(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY, char HP);
void mpEcho(SerializeBuffer* pPacket, DWORD Time);
void mpAttack1(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY);
void mpAttack2(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY);
void mpAttack3(SerializeBuffer* pPacket, DWORD dwSessionID, char byDirection, short shX, short shY);
void mpDamge(SerializeBuffer* pPacket, DWORD dwAttackID, DWORD dwDamageID, char DamageHP);
void mpDeleteCharacter(SerializeBuffer* pPacket, DWORD dwSessionID);


#if C_SESSION == 1
class C_Session
{
	enum
	{
		dfNETWORK_PORT = 20000
	};

public:
	static C_Session* GetInstance(void);
	void netIOProcess(void);					// �� �Լ��� ���ο��� ȣ���Ͽ� ���ο��� Send, Recv�� �� ��ģ��. 
	void netProc_Accept(void);
	void netProc_Send(st_SESSION* pSession);
	void netProc_Recv(st_SESSION* pSession);


	//----------------------------------------------------------------
	// Ư�� ���� 1���� �ִ� Ŭ���̾�Ʈ�� ���� �޽��� ������,
	// ������ �Ű����� �����ϰ�
	void SendPacket_SectorOne(int iSectorX, int iSectorY, SerializeBuffer* pPacket, st_SESSION* pExceptSession);

	//----------------------------------------------------------------
	// Ư�� 1���� Ŭ���̾�Ʈ ���� �޽��� ������
	void SendPacket_Unicast(st_SESSION* pSession, SerializeBuffer* pPacket);

	//----------------------------------------------------------------
	// Ŭ���̾�Ʈ ���� �ֺ� ���Ϳ� �޽��� ������ (�ִ� 9�� ����)
	void SendPacket_Around(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around, bool bSendMe = false);

	//----------------------------------------------------------------
	// ���� ��ε� ĳ���� (�ý������� �޽��� �ܿ��� ������� ����)
	void SendPacket_Broadcast(st_SESSION* pSession, SerializeBuffer* pPacket);

	//----------------------------------------------------------------
	// ��Ŷ ó���� �������� ����� ó���Ѵ�. 
	bool PacketProc(st_SESSION* pSession, unsigned char byPacketType, SerializeBuffer* pPacket);

	//----------------------------------------------------------------
	// ���� ��Ŷ ó��
	bool netPacketProc_MoveStart(st_SESSION* pSession, SerializeBuffer* pPacket);
	bool netPacketProc_MoveStop(st_SESSION* pSession, SerializeBuffer* pPacket);
	bool netPacketProc_Attack1(st_SESSION* pSession, SerializeBuffer* pPacket);
	bool netPacketProc_Attack2(st_SESSION* pSession, SerializeBuffer* pPacket);
	bool netPacketProc_Attack3(st_SESSION* pSession, SerializeBuffer* pPacket);




private:
	C_Session(void);
	~C_Session(void);

private:
	static C_Session _C_Session;

	// �����ڿ��� �ʱ�ȭ �Ͽ� 0���� ����� �� ���� 1�� ������. 
	DWORD _SessionID;

public:
	// Key: UserID, Value: Session
	//std::map<DWORD, st_SESSION*> _Session;

	// Key: SessionID, Value: Session
	std::unordered_map< DWORD, st_SESSION*> _Session_Hash;

public:
	SOCKET _Listen_Socket;
	WSADATA _WsaData;

};
#endif

#endif // !__SESSION_H__
