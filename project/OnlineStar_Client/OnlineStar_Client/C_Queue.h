
#ifndef __C_QUEUE_H__
#define __C_QUEUE_H__

#include <stdlib.h>
#include <malloc.h>

namespace OreoPizza
{
	template <class DATA>
	class C_Queue
	{
	public:
		inline C_Queue(int size);
		inline ~C_Queue();

		inline void enqueue(const DATA& data);
		inline void dequeue(void);
		inline bool dequeue(int Data_Size);
		inline DATA back(void);
		inline bool back(void* Buff, int Data_Size);
		inline bool isQueueEmpty(void);
		inline bool isQueueFull(void);

		inline bool find(const DATA& data);

		int _In;
		int _Out;
		int _Size;
		int _Data_Size;
		// int _Empty_Size // 필요한가?? 사실 _Size - _Data_Size 랑 같은데, 함수 호출될 때 마다 호출되어 값을 갱신하기에는 매번 어셈블리 3개씩 들어간다. 
		DATA* _queue;
	};

	template<class DATA>
	inline C_Queue<DATA>::C_Queue(int size) : _In(0), _Out(0), _Size(size), _Data_Size(0)
	{
		// _queue = (DATA*)malloc(sizeof(DATA) * size);
		_queue = new DATA[size];
	}

	template<class DATA>
	inline C_Queue<DATA>::~C_Queue()
	{
		// free(_queue);
		delete[] _queue;
	}

	template<class DATA>
	inline void C_Queue<DATA>::enqueue(const DATA& data)
	{
		if (isQueueFull())
			return;

		_queue[_In] = data;
		_In = (_In + 1) % _Size;

		++_Data_Size;
	}

	template<class DATA>
	inline void C_Queue<DATA>::dequeue(void)
	{
		if (isQueueEmpty())
			return;
		_Out = (_Out + 1) % _Size;
		
		--_Data_Size;
	}

	template<class DATA>
	inline bool C_Queue<DATA>::dequeue(int Data_Size)
	{
		if (_Data_Size < Data_Size)
			return false;

		_Out = (_Out + Data_Size) % _Size;

		_Data_Size -= Data_Size;
		return true;
	}

	template<class DATA>
	inline DATA C_Queue<DATA>::back(void)
	{
		return _queue[_Out];
	}

	template<class DATA>
	inline bool C_Queue<DATA>::back(void* Buff, int Data_Size)
	{
		// 1byte 씩 데이터를 읽어 전달하기 때문에 char* 로 버퍼를 받는다.
		// Buff를 char* 로 받으면 구조체의 형변환이 안된다. 
		int i_Cnt;
		int Out = _Out;

		if (_Data_Size < Data_Size)
			return false;

		for (i_Cnt = 0; i_Cnt < Data_Size; ++i_Cnt)
		{

			*((char*)Buff + i_Cnt) = _queue[(Out+i_Cnt) % _Size];
		}

		return true;
	}

	template<class DATA>
	inline bool C_Queue<DATA>::isQueueEmpty(void)
	{
		return _In == _Out;
	}

	template<class DATA>
	inline bool C_Queue<DATA>::isQueueFull(void)
	{
		return (_In + 1) % _Size == _Out;
	}

	template<class DATA>
	inline bool C_Queue<DATA>::find(const DATA& data)
	{
		int in = _In;
		int out = _Out;
		int size = _Size;

		while (out != in)
		{
			if (_queue[out] == data)
				return true;

			out = (out + 1) % size;
		}

		return false;
	}


	
}
#endif // !__C_QUEUE_H__


