
#ifndef __C_RING_BUFFER_H__
#define __C_RING_BUFFER_H__




// TMI: 클래스 포인터 멤버 변수 -> 지역변수에 저장하여 접근하는 방식 
//	직접 접근: 매 순간 어셈블리 2줄 + 값 저장하는 1줄 == 총 3줄 
//	지역 변수: 지역 변수에 저장 3줄 + 값 저장하는 1줄 == 총 4줄 
//		But!!!!!
//		최종적인 저장 == 직접 접근하여 변수를 저장 총 3줄이 추가된다. 
// ==> 3 + 3 + 3 = 9
// ==> 4 + 1 + 3 = 8
// 단순하게 계산하면 3번 이상 접근하여 저장한다면 지역 변수에 저장하여 접근하는 것이 이득이다. 

constexpr size_t df_C_RING_BUFFER_DEFAULT_LEN = 10000;

class C_RING_BUFFER
{
private:
	char* _Buffer;

	size_t _In;
	size_t _Out;

	//-----------------------------------------------------------------------
	// idx 초기값이 0이 아니라면, 관리하기가 어렵다.
	//char* _In;
	//char* _Out;
	// const char* _Buffer_End;

	//-----------------------------------------------------------------------
	// 고민!!!! 사용중인 공간이랑 빈 공간은 사실 하나로 만들 수 있음
	// 이걸 기록하는 과정 -> 데이터 넣고 뺄때 마다 연산해야 함. 
	// 연산량이 많아질거 같음
	// 구현하다 적게 쓰는것을 메인으로 쓰기로 함.
	size_t _Full_Size;	// 현재 링버퍼의 최대 사이즈
	// size_t _Use_Size;	// 현재 링버퍼가 사용하고 있는 사이즈
	// int _Free_Size;	// 이거 매번 저장하는게 더 느리다.

public:
	/////////////////////////////////////////////////////////////////////////
	// 기본 사이즈를 1만 Byte로 할당한다.
	// 넣고 -> 증가 == 마지막 1칸을 사용하지 못한다. 
	// 고로 입력 사이즈 + 1이 실제 사이즈
	/////////////////////////////////////////////////////////////////////////
	C_RING_BUFFER(void);
	C_RING_BUFFER(int i_Buffer_Size);

	virtual ~C_RING_BUFFER();

	size_t GetBufferSize(void) { return _Full_Size; }

	/////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 용량 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 용량.
	/////////////////////////////////////////////////////////////////////////
	size_t	GetUseSize(void) 
	{
		// unsigned 변수를 - 연산 함부로 하면 안된다.
		if (_In >= _Out)
		{
			return _In - _Out;
		}
		else
		{
			return _Full_Size - (_Out - _In);
		}
	}
private:
	size_t	GetUseSize(size_t In, size_t Out)
	{
		if (In >= Out)
		{
			return In - Out;
		}
		else
		{
			return _Full_Size - (Out - In);
		}
	}
public:

	/////////////////////////////////////////////////////////////////////////
	// 현재 버퍼에 남은 용량 얻기. 
	//
	// Parameters: 없음.
	// Return: (int)남은용량.
	/////////////////////////////////////////////////////////////////////////
	size_t	GetFreeSize(void) 
	{
		return _Full_Size - GetUseSize();

		size_t i = _Full_Size - GetUseSize(_In, _Out);

		return i;
	}

	/////////////////////////////////////////////////////////////////////////
	// WritePos 에 데이타 넣음.
	// 남은 버퍼의 크기가 iSize보다 작은 경우 데이터를 넣지 않고 0을 반환한다.
	//
	// Parameters: (char *)데이타 포인터. (int)크기. 
	// Return: (int)넣은 크기.
	/////////////////////////////////////////////////////////////////////////
	size_t Enqueue(const char* chpData, size_t iSize);
	
	/////////////////////////////////////////////////////////////////////////
	// ReadPos 에서 데이타 가져옴. ReadPos 이동.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)가져온 크기.
	/////////////////////////////////////////////////////////////////////////
	size_t	Dequeue(char* chpDest, size_t iSize);

	/////////////////////////////////////////////////////////////////////////
	// ReadPos 에서 데이타 읽어옴. ReadPos 고정.
	//
	// flag	== true: if(iSize > _Use_Size ) return 0;
	// 
	// Parameters: (char *)데이타 포인터. (int)크기. (bool)defualt = false
	// Return: (int)가져온 크기.
	/////////////////////////////////////////////////////////////////////////
	size_t	Peek(char* chpDest, size_t iSize, bool flag = false);

	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 모든 데이타 삭제.
	//
	// Parameters: 없음.
	// Return: 없음.
	/////////////////////////////////////////////////////////////////////////
	void ClearBuffer(void) { _In = _Out = 0; }



	size_t	DirectEnqueueSize(void);

	size_t	DirectDequeueSize(void);

	size_t	MoveIn(size_t iSize);
	size_t	MoveOut(size_t iSize);
	char* GetOutBufferPtr(void);
	char* GetInBufferPtr(void);

	char* GetBeginBufferPtr(void);


	/////////////////////////////////////////////////////////////////////////
	// friend class CLASS 
	// 를 사용하여 private 함수를 사용할 수 있다.
	/////////////////////////////////////////////////////////////////////////
//private:
public:
	/////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이.
	// (끊기지 않은 길이)
	//
	// 원형 큐의 구조상 버퍼의 끝단에 있는 데이터는 끝 -> 처음으로 돌아가서
	// 2번에 데이터를 얻거나 넣을 수 있음. 이 부분에서 끊어지지 않은 길이를 의미
	//
	// Parameters: 없음.
	// Return: (int)사용가능 용량.
	////////////////////////////////////////////////////////////////////////
	//size_t	DirectEnqueueSize(void)
	//{
	//	size_t Full_Size = _Full_Size;
	//	size_t In = _In;
	//	size_t Out = _Out;
	//	// Enqueue의 경우 _In 바로 다음이 _Out인 경우 꽉 찬 경우이다. 
	//	//if (_Use_Size == 0) return 0;

	//	if ((In + 1) % Full_Size == Out)
	//		return 0;

	//	if (In <= ((Out + Full_Size - 1) % Full_Size))
	//		return ((Out + Full_Size - 1) % Full_Size) - In;
	//	else if (In >= Out)
	//		return Full_Size - In;

	//	return 0;
	//}
	//size_t	DirectDequeueSize(void)
	//{
	//	size_t In = _In;
	//	size_t Out = _Out;

	//	

	//	if (In >= Out)
	//	{
	//		return In - Out;
	//	}
	//	else
	//	{
	//		return _Full_Size - Out;
	//	}

	//	//if (_In >= _Out)
	//	//{
	//	//	return _In - _Out;
	//	//}
	//	//else
	//	//{
	//	//	return _Full_Size - _Out;
	//	//}
	//}

	///////////////////////////////////////////////////////////////////////////
	//// 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
	////
	//// Parameters: 없음.
	//// Return: (int)이동크기
	///////////////////////////////////////////////////////////////////////////
	//size_t	MoveIn(size_t iSize)
	//{
	//	_In = (_In + iSize) % _Full_Size;
	//	return _In;
	//}
	//size_t	MoveOut(size_t iSize)
	//{
	//	_Out = (_Out + iSize) % _Full_Size;
	//	return _Out;
	//}



	///////////////////////////////////////////////////////////////////////////
	//// 버퍼의 Front, _Out 포인터 얻음.
	////
	//// Parameters: 없음.
	//// Return: (char *) 버퍼 포인터.
	///////////////////////////////////////////////////////////////////////////
	//char* GetOutBufferPtr(void)
	//{
	//	return _Buffer + _Out;
	//}


	///////////////////////////////////////////////////////////////////////////
	//// 버퍼의 RearPos, _In 포인터 얻음.
	////
	//// Parameters: 없음.
	//// Return: (char *) 버퍼 포인터.
	///////////////////////////////////////////////////////////////////////////
	//char* GetInBufferPtr(void)
	//{
	//	return _Buffer + _In;
	//}

	///////////////////////////////////////////////////////////////////////////
	//// 버퍼의 _Begin 포인터 얻음.
	////
	//// Parameters: 없음.
	//// Return: (char *) 버퍼 시작 포인터.
	///////////////////////////////////////////////////////////////////////////
	//char* GetBeginBufferPtr(void)
	//{
	//	return _Buffer;
	//}



};


#endif // !__C_RING_BUFFER_H__

