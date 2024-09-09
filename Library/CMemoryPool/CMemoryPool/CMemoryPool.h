/*---------------------------------------------------------------

	OreoPizza MemoryPool.

	�޸� Ǯ Ŭ���� (������Ʈ Ǯ / ��������Ʈ)
	Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

	- ����.

	OreoPizza::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData ���

	MemPool.Free(pData);

	�Ҵ��ϴ� ��ü ���� �����ڿ��� ���� �Ҵ��� �ִٸ�, �޸𸮸� �Ҵ��� ��,
	�����ڸ� ȣ���Ͽ� ������ �����Ҵ� ���� ������ �Ѵ�. 
----------------------------------------------------------------*/
#ifndef __CMEMORY_POOL_H__
#define __CMEMORY_POOL_H__

#include <stdio.h>
#include <stddef.h>
#include <new>
#include <Windows.h>


namespace OreoPizza
{
	// Ŭ���� ���漱��
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
		// ������, �ı���.
		//
		// Parameters:	(int) �ʱ� �� ����.
		//				(bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CMemoryPool(int iBlockNum, bool bPlacementNew = false);
		CMemoryPool(int iBlockNum, int ArrSize = 1, bool bPlacementNew = false);
		virtual	~CMemoryPool();


		//////////////////////////////////////////////////////////////////////////
		// �� �ϳ��� �Ҵ�޴´�.  
		// 
		// ���� ���ٸ� �޸𸮸� �Ҵ��Ͽ� ���� ���� �� �Ҵ��Ѵ�.
		//
		// Parameters: ����.
		// Return: (DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////////////////////////
		DATA* Alloc();

		//////////////////////////////////////////////////////////////////////////
		// ������̴� ���� �����Ѵ�.
		//
		// Parameters: (DATA *) �� ������.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free(DATA* pData);


		//////////////////////////////////////////////////////////////////////////
		// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
		//
		// Parameters: ����.
		// Return: (int) �޸� Ǯ ���� ��ü ����
		//////////////////////////////////////////////////////////////////////////
		int		GetCapacityCount(void) { return m_iCapacity; }

		//////////////////////////////////////////////////////////////////////////
		// ���� ������� �� ������ ��´�.
		//
		// Parameters: ����.
		// Return: (int) ������� �� ����.
		//////////////////////////////////////////////////////////////////////////
		int		GetUseCount(void) { return m_iUseCount; }

		//////////////////////////////////////////////////////////////////////////
		// ����Ʈ�� ��ȸ�Ѵ�.
		// 
		//////////////////////////////////////////////////////////////////////////
		void	TraverseMemoryPool(void);


		// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.
		// DATA ������ �ʱ⿡ ����Ʈ �����ڰ� �����Ѵ�.
		st_BLOCK_NODE<DATA> _pFreeNode;

	private:
		int			m_iCapacity;		// ���� ����Ʈ�� ����� �޸� ����
		int			m_iUseCount;		// ���� ���ǰ� �ִ� �޸��� ����
		int 		m_bArrSize;			// �����ۿ��� ����ϱ� ���Ͽ� �迭�� �Ҵ��� �� �ְ� �������.   �ٵ� ó������ �迭�� ����� ������ �ֳ�?
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
			// ���� �ڵ�: call malloc, call new 2���� ����� ȣ���� �Ͼ��.
			//st_BLOCK_NODE<DATA>* pNewNode = (st_BLOCK_NODE<DATA> *)malloc(sizeof(st_BLOCK_NODE<DATA>));
			//if (pNewNode == NULL)
			//	throw;
			//// ������ ȣ��
			//new(&(pNewNode->data)) DATA();
			st_BLOCK_NODE<DATA>* pNewNode = new st_BLOCK_NODE<DATA>;


			// ���� �����, �ּ� ����
#ifdef _DEBUG
			pNewNode->allocationRecord = (void*)this;
#endif // _DEBUG

			// ������ _pFreeNode.pNext = NULL; �ʱ�ȭ
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

			// ���� �����, �ּ� ����
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

				// ���� �ڵ�: call�Ҹ��� call �޸� ���� -> call�� 2�� �Ͼ��.
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

				// ���� �ڵ�: call�Ҹ��� call �޸� ���� -> call�� 2�� �Ͼ��.
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
			// ���� �ڵ�: call malloc, call new 2���� ����� ȣ���� �Ͼ��.
			//pTempNode = (st_BLOCK_NODE<DATA>*)malloc(sizeof(st_BLOCK_NODE<DATA>));
			//if (pTempNode == NULL)
			//	throw;
			//// ������ ȣ��
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



		// �Ҹ��ڴ� ������ ȣ�� �Ǵ°� �´ٰ� ������. X
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
			printf_s("��� ���: %d \n", iCnt);
			printf_s("����� �ּ� : % p, ���� ��� �ּ� : % p \n", curNode, curNode->pNext);
			++iCnt;
		}

		printf_s("���� ����Ʈ�� ��� �� %d,  ������� ����� �� %d\n", m_iCapacity, m_iUseCount);

	}

}

#endif // !__CMEMORY_POOL_H__

