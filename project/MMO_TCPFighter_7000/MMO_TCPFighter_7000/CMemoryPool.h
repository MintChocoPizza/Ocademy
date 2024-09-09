/*---------------------------------------------------------------

	OreoPizza MemoryPool.

	�޸� Ǯ Ŭ���� (������Ʈ Ǯ / ��������Ʈ)
	Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

	- ����.

	OreoPizza::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData ���

	MemPool.Free(pData);


----------------------------------------------------------------*/
#ifndef __CMEMORY_POOL_H__
#define __CMEMORY_POOL_H__

#include <stdio.h>
#include <stddef.h>

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
		int		m_iCapacity;		// ���� ����Ʈ�� ����� �޸� ����
		int		m_iUseCount;		// ���� ���ǰ� �ִ� �޸��� ����
		bool	m_bPlacementNew;
	};



	template<class DATA>
	inline CMemoryPool<DATA>::CMemoryPool(int iBlockNum, bool bPlacementNew) :
		m_iCapacity(iBlockNum), m_iUseCount(0), m_bPlacementNew(bPlacementNew)
	{
		int iCnt;
		_pFreeNode.pNext = NULL;

		for (iCnt = 0; iCnt < iBlockNum; ++iCnt)
		{
			st_BLOCK_NODE<DATA>* pNewNode = new st_BLOCK_NODE<DATA>;


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

		for (deleteNode = _pFreeNode.pNext; deleteNode != NULL; deleteNode = _pFreeNode.pNext)
		{
			_pFreeNode.pNext = deleteNode->pNext;

			delete deleteNode;

			--m_iCapacity;
		}
	}

	template<class DATA>
	inline DATA* CMemoryPool<DATA>::Alloc()
	{
		st_BLOCK_NODE<DATA>* pTempNode;

		if (m_iCapacity == 0)
		{

			pTempNode = new st_BLOCK_NODE<DATA>;

#ifdef _DEBUG
			pTempNode->allocationRecord = this;
#endif // _DEBUG

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


		++m_iUseCount;

		return &pTempNode->data;
	}

	template<class DATA>
	inline bool CMemoryPool<DATA>::Free(DATA* pData)
	{
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



		pData->~DATA();


		st_makeStruct->pNext = _pFreeNode.pNext;
		_pFreeNode.pNext = st_makeStruct;

		++m_iCapacity;
		--m_iUseCount;

		return true;
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

