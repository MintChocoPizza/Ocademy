#pragma once


//#define MEM_DEUBG


#define MAX_LOG_CNT 1000


// 현재 함수
enum CurrentFunc
{
	enRecvProc = 1,
	enRecvPost = 2,
	enSendProc = 3,
	enSendPost = 4
};

// SendPost 함수 반환 이유
enum RetSendPost
{
	enNone = 0,
	enSucces = 1,
	enFlag_True = 2,
	enUseSizeZero = 3,

};

struct st_Debug
{
	DWORD64 _Count;

	DWORD	_ThreadID;
	DWORD64 _sessionID;

	CurrentFunc _CurrentFunc;

	size_t _SendQ_in;
	size_t _SendQ_out;
	size_t _RecvQ_in;
	size_t _RecvQ_out;
	
	RetSendPost _RetSendPost;

	//DWORD64 _Payload;
	DWORD	_DataSize;

	void MemLog(DWORD64 Count, DWORD ThreadID, DWORD64 sessionID, CurrentFunc CurrentFunc, size_t SendQ_in, size_t SendQ_out, size_t RecvQ_in, size_t RecvQ_out, RetSendPost retSendPost, DWORD DataSize)
	{
		_Count = Count;

		_ThreadID = ThreadID;
		_sessionID = sessionID;

		_CurrentFunc = CurrentFunc;

		_SendQ_in = SendQ_in;
		_SendQ_out = SendQ_out;
		_RecvQ_in = RecvQ_in;
		_RecvQ_out = RecvQ_out;

		_RetSendPost = retSendPost;

		//_Payload = Payload;
		_DataSize = DataSize;
	}
};
