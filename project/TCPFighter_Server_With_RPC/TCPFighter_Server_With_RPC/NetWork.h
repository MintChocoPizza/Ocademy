
#ifndef __NETWORK_H__
#define __NETWORK_H__



//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
constexpr int dfERROR_RANGE = 50;


struct st_SESSION
{
	SOCKET			Socket;				// 현 접속의 TCP 소켓
	DWORD			dwSessionID;		// 접속자의 고유 세션 ID
	C_RING_BUFFER	RecvQ;				// 수신 큐;
	C_RING_BUFFER	SendQ;				// 송신 큐;

	DWORD			dwAction;			// 
	BYTE			byDirection;		//	

	short			shX;
	short			shY;

	char			chHP;

	bool			Disconnect;

	st_SESSION() :Socket(INVALID_SOCKET), dwSessionID(0), RecvQ(1500), SendQ(1500), dwAction(-1), byDirection(-1), shX(-1), shY(-1), chHP(-1), Disconnect(false)
	{

	}
};

extern std::map<DWORD, st_SESSION*> g_Session_List;
extern CList<st_SESSION*> g_Disconnect_List;




bool init_Listen_Socket(void* Listen_Socket, void* wsa_Data);
void netIOProcess(void);
void netProc_Accept(void);
void init_Session(SOCKET Client_Socket, st_SESSION* st_New_Player);

void netProc_Recv(st_SESSION* pSession);
void netProc_Send(st_SESSION* pSession);

#ifdef SERIALIZEBUFFER
void SerializeBuffer_netSendUnicast(st_SESSION* pSession, SerializeBuffer* clpPacket);
void SerializeBuffer_netSendBroadcast(st_SESSION* pSession, SerializeBuffer* clpPacket);
bool SerializeBuffer_PacketProc(st_SESSION* pSession, BYTE byPacketType, SerializeBuffer* clpPacket);
#else
void netSendUnicast(st_SESSION* pSession, char* header, char* packet, int Packet_Len);
void netSendBroadcast(st_SESSION* pSession, char* header, char* packet, int Packet_Len);
bool PacketProc(st_SESSION* pSession, BYTE byPacketType, char* pPacket);
#endif // SERIALIZEBUFFER

void PushDisconnectList(st_SESSION* pSession);
void Disconnect(void);


#endif // !__NETWORK_H__
