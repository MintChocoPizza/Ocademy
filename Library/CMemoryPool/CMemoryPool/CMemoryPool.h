/*---------------------------------------------------------------

	OreoPizza MemoryPool.

	메모리 풀 클래스 (오브젝트 풀 / 프리리스트)
	특정 데이타(구조체,클래스,변수)를 일정량 할당 후 나눠쓴다.

	- 사용법.

	OreoPizza::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData 사용

	MemPool.Free(pData);

	할당하는 객체 내부 생성자에서 동적 할당이 있다면, 메모리를 할당할 때,
	생성자를 호출하여 내부의 동적할당 까지 마무리 한다. 
----------------------------------------------------------------*/
#ifndef __CMEMORY_POOL_H__
#define __CMEMORY_POOL_H__

#include <stdio.h>
#include <stddef.h>
#include <new>
#include <Windows.h>


namespace OreoPizza
{
	// 클래스 전방선언
	template <class DATA>
	class CMemoryPool;

	template <class DATA>
	struct st_BLOCK_NODE
	{
#ifdef _DEBUG
		void* allocationRecord;
#endif // _DEBUG
		DATA data;
		st_BLOCK_NODE* pNext;
	};

	template <class DATA>
	class CMemoryPool
	{

	public:

		//////////////////////////////////////////////////////////////////////////
		// 생성자, 파괴자.
		//
		// Parameters:	(int) 초기 블럭 개수.
		//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CMemoryPool(int iBlockNum, bool bPlacementNew = false);
		CMemoryPool(int iBlockNum, int ArrSize = 1, bool bPlacementNew = false);
		virtual	~CMemoryPool();


		//////////////////////////////////////////////////////////////////////////
		// 블럭 하나를 할당받는다.  
		// 
		// 블럭이 없다면 메모리를 할당하여 블럭을 생성 후 할당한다.
		//
		// Parameters: 없음.
		// Return: (DATA *) 데이타 블럭 포인터.
		//////////////////////////////////////////////////////////////////////////
		DATA* Alloc();

		//////////////////////////////////////////////////////////////////////////
		// 사용중이던 블럭을 해제한다.
		//
		// Parameters: (DATA *) 블럭 포인터.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free(DATA* pData);


		//////////////////////////////////////////////////////////////////////////
		// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
		//
		// Parameters: 없음.
		// Return: (int) 메모리 풀 내부 전체 개수
		//////////////////////////////////////////////////////////////////////////
		int		GetCapacityCount(void) { return m_iCapacity; }

		//////////////////////////////////////////////////////////////////////////
		// 현재 사용중인 블럭 개수를 얻는다.
		//
		// Parameters: 없음.
		// Return: (int) 사용중인 블럭 개수.
		//////////////////////////////////////////////////////////////////////////
		int		GetUseCount(void) { return m_iUseCount; }

		//////////////////////////////////////////////////////////////////////////
		// 리스트를 순회한다.
		// 
		//////////////////////////////////////////////////////////////////////////
		void	TraverseMemoryPool(void);


		// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
		// DATA 생성시 초기에 디폴트 생성자가 동작한다.
		st_BLOCK_NODE<DATA> _pFreeNode;

	private:
		int			m_iCapacity;		// 현재 리스트에 연결된 메모리 갯수
		int			m_iUseCount;		// 현재 사용되고 있는 메모리의 갯수
		int 		m_bArrSize;			// 링버퍼에서 사용하기 위하여 배열을 할당할 수 있게 만들었다.   근데 처음부터 배열로 만들면 문제가 있나?
		bool		m_bPlacementNew;
		SRWLOCK		m_srwLock;
	};



	template<class DATA>
	inline CMemoryPool<DATA>::CMemoryPool(int iBlockNum, bool bPlacementNew) :
		m_iCapacity(iBlockNum), m_iUseCount(0), m_bPlacementNew(bPlacementNew), m_bArrSize(0)
	{
		int iCnt;
		_pFreeNode.pNext = NULL;

		InitializeSRWLock(&m_srwLock);

		for (iCnt = 0; iCnt < iBlockNum; ++iCnt)
		{
			// 기존 코드: call malloc, call new 2번의 어셈블리 호출이 일어난다.
			//st_BLOCK_NODE<DATA>* pNewNode = (st_BLOCK_NODE<DATA> *)malloc(sizeof(st_BLOCK_NODE<DATA>));
			//if (pNewNode == NULL)
			//	throw;
			//// 생성자 호출
			//new(&(pNewNode->data)) DATA();
			st_BLOCK_NODE<DATA>* pNewNode = new st_BLOCK_NODE<DATA>;


			// 흔적 남기기, 주소 저장
#ifdef _DEBUG
			pNewNode->allocationRecord = (void*)this;
#endif // _DEBUG

			// 위에서 _pFreeNode.pNext = NULL; 초기화
			(*pNewNode).pNext = _pFreeNode.pNext;
			_pFreeNode.pNext = pNewNode;
		}
	}

	template<class DATA>
	inline CMemoryPool<DATA>::CMemoryPool(int iBlockNum, int ArrSize, bool bPlacementNew) :
		m_iCapacity(iBlockNum), m_iUseCount(0), m_bPlacementNew(bPlacementNew), m_bArrSize(ArrSize)

	{
		int iCnt;
		_pFreeNode.pNext = NULL;

		InitializeSRWLock(&m_srwLock);

		for (iCnt = 0; iCnt < iBlockNum; ++iCnt)
		{
			st_BLOCK_NODE<DATA>* pNewNode = new st_BLOCK_NODE<DATA>[ArrSize];

			// 흔적 남기기, 주소 저장
#ifdef _DEBUG
			pNewNode->allocationRecord = (void*)this;
#endif // _DEBUG
			(*pNewNode).pNext = _pFreeNode.pNext;
			_pFreeNode.pNext = pNewNode;
		}
	}

	template<class DATA>
	inline CMemoryPool<DATA>::~CMemoryPool()
	{
		st_BLOCK_NODE<DATA>* deleteNode;

		if (m_bArrSize == 0)
		{
			for (deleteNode = _pFreeNode.pNext; deleteNode != NULL; deleteNode = _pFreeNode.pNext)
			{
				_pFreeNode.pNext = deleteNode->pNext;

				// 기존 코드: call소멸자 call 메모리 해지 -> call이 2번 일어난다.
				//(deleteNode->data).~DATA();
				////(_pFreeNode.data).~DATA();
				//free(deleteNode);
				delete deleteNode;

				--m_iCapacity;
			}
		}
		else
		{
			for (deleteNode = _pFreeNode.pNext; deleteNode != NULL; deleteNode = _pFreeNode.pNext)
			{
				_pFreeNode.pNext = deleteNode->pNext;

				// 기존 코드: call소멸자 call 메모리 해지 -> call이 2번 일어난다.
				//(deleteNode->data).~DATA();
				////(_pFreeNode.data).~DATA();
				//free(deleteNode);
				delete[] deleteNode;

				--m_iCapacity;
			}
		}
	}

	template<class DATA>
	inline DATA* CMemoryPool<DATA>::Alloc()
	{
		st_BLOCK_NODE<DATA>* pTempNode;

		AcquireSRWLockExclusive(&m_srwLock);

		if (m_iCapacity == 0)
		{
			// 기존 코드: call malloc, call new 2번의 어셈블리 호출이 일어난다.
			//pTempNode = (st_BLOCK_NODE<DATA>*)malloc(sizeof(st_BLOCK_NODE<DATA>));
			//if (pTempNode == NULL)
			//	throw;
			//// 생성자 호출
			//new(&(pTempNode->data)) DATA();
			if (m_bArrSize == 0)
				pTempNode = new st_BLOCK_NODE<DATA>;
			else
				pTempNode = new st_BLOCK_NODE<DATA>[m_bArrSize];

#ifdef _DEBUG
			pTempNode->allocationRecord = this;
#endif // _DEBUG


			// (*pTempNode).pNext = _pFreeNode.pNext;
			// _pFreeNode.pNext = pTempNode;

			// ++m_iCapacity;
		}
		else
		{
			pTempNode = _pFreeNode.pNext;
			_pFreeNode.pNext = pTempNode->pNext;
			--m_iCapacity;

			if (m_bPlacementNew == true)
			{
				new(&(pTempNode->data)) DATA();
			}
		}

		// _pFreeNode.pNext = pTempNode->pNext;

		//if (m_bPlacementNew == true)
		//{
		//	new(&(pTempNode->data)) DATA();
		//}

		++m_iUseCount;

		ReleaseSRWLockExclusive(&m_srwLock);
		return &pTempNode->data;
	}

	template<class DATA>
	inline bool CMemoryPool<DATA>::Free(DATA* pData)
	{
		AcquireSRWLockExclusive(&m_srwLock);

#ifdef _DEBUG
		int a = offsetof(st_BLOCK_NODE<DATA>, data);
		st_BLOCK_NODE<DATA>* st_makeStruct = (st_BLOCK_NODE<DATA> *)((char*)pData - offsetof(st_BLOCK_NODE<DATA>, data));
		if (st_makeStruct->allocationRecord != this)
		{
			return false;
		}
#else
		st_BLOCK_NODE<DATA>* st_makeStruct = (st_BLOCK_NODE<DATA> *)(pData);
#endif // _DEBUG



		// 소멸자는 무조건 호출 되는게 맞다고 생각함. X
		if (m_bPlacementNew == true)
		{
			pData->~DATA();
		}
		//pData->~DATA();


		st_makeStruct->pNext = _pFreeNode.pNext;
		_pFreeNode.pNext = st_makeStruct;

		++m_iCapacity;
		--m_iUseCount;

		return true;

		ReleaseSRWLockExclusive(&m_srwLock);
	}

	template<class DATA>
	inline void CMemoryPool<DATA>::TraverseMemoryPool(void)
	{
		st_BLOCK_NODE<DATA>* curNode = _pFreeNode.pNext;
		int iCnt = 0;

		for (curNode = _pFreeNode.pNext; curNode != NULL; curNode = (*curNode).pNext)
		{
			printf_s("노드 출력: %d \n", iCnt);
			printf_s("노드의 주소 : % p, 다음 노드 주소 : % p \n", curNode, curNode->pNext);
			++iCnt;
		}

		printf_s("현재 리스트의 노드 수 %d,  사용중인 노드의 수 %d\n", m_iCapacity, m_iUseCount);

	}

}

#endif // !__CMEMORY_POOL_H__

