//----------------------------------------------------------------------------------------------
// ����
// 
// CList<T> ������;
// 
// 
//----------------------------------------------------------------------------------------------


#ifndef __CLIST_H__
#define __CLIST_H__

#include <new>
#include "CMemoryPool.h"

//////////////////////////////////////////////////////////////////////////////////////////
// ���ø� Ŭ������ ��� ���Ǻο� ����θ� �и��� �� ����. 
// 
// �ϳ��� ���Ͽ� �ۼ��ؾ� ��
// 
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class CList
{
public:
	struct Node
	{
		T _data;
		Node* _Prev;
		Node* _Next;
	};


	//////////////////////////////////////////////////////////////////////////////////////////
	// iter�� Node�� �ּҸ� ������ ����.
	// iter�� new�� ���Ͽ� �Ҵ���� ����. 
	// �׳� ���ο� Node�� �ּҸ� ������ ����. 
	// 
	// ��������� �Ҹ��� ȣ���� ���� �� �ʿ䰡 ����. 
	// 
	// ���� �޸� �������� �ʿ� ����. 
	// 
	// Node�� �޸𸮸� �����ϸ� �ȴ�.
	// 
	// delete iter._node 
	//////////////////////////////////////////////////////////////////////////////////////////
	class iterator
	{
	public:
		Node* _node;
	public:
		iterator(Node* node = nullptr)
		{
			// ���ڷ� ���� Node �����͸� ����
			_node = node;
		}


		inline iterator operator ++(int);
		inline iterator& operator ++();
		inline iterator operator --(int);
		inline iterator& operator --();
		inline bool operator ==(const iterator& other);
		inline bool operator !=(const iterator& other);
		inline iterator& operator += (int num);
		inline T& operator *();


	};


private:
	OreoPizza::CMemoryPool<Node> _MemPool;
	int _size = 0;
	Node _head;
	Node _tail;

public:
	CList();
	~CList();

	// ��� ��Ҹ� ��ȸ�ϸ� ����Ѵ�.
	void printAll(void);

	//-----------------------------------------------------
	// 4�� �ʱ�ȭ�� 3���� ���Ҹ� �Ҵ��Ѵ�.
	// 
	// ex)
	// it.assign(3,4);
	// 
	//-----------------------------------------------------
	void assign(int count, T data);

	// �� ���� ���Ҹ� ��ȯ(return), ���� �Ѵ�.
	T& front(void);
	// �� ���� ���Ҹ� ��ȯ(return), ���� �Ѵ�.
	T& back(void);

	//-----------------------------------------------------
	// �� ���� ���Ҹ� ����Ű�� iterator�� ��ȯ�Ѵ�. 
	// 
	// ex) list<int>::iterator iter;
	// iter = lt.begin();
	// 
	//-----------------------------------------------------
	inline iterator begin(void);

	//-----------------------------------------------------
	// �� �������� ������ ������ ����Ű�� iterator�� ��ȯ�Ѵ�. 
	// 
	// ex) list<int>::iterator iter;
	// iter = It.end();
	// 
	//-----------------------------------------------------
	inline iterator end(void);

	//-----------------------------------------------------
	// �� �������� ������ ������ ����Ű�� iterator�� ��ȯ�Ѵ�. 
	// 
	// ex) list<int>::iterator iter;
	// iter = It.end();
	// 
	//-----------------------------------------------------

	// �������� ���� data�� �����Ѵ�.
	inline void push_front(const T& data);
	// �������� ���� data�� �����Ѵ�.
	inline void push_back(const T& data);
	// �� ������ ���Ҹ� �����Ѵ�.
	inline void pop_back(void);
	// �� ù��° ���Ҹ� �����Ѵ�.
	inline void pop_front(void);

	// ���Ұ� ����ִ��� Ȯ�� 
	inline bool empty(void);
	//-----------------------------------------------------
	// iter�� �������� ������ ����
	// erase�� iterator._node �� �����Ѵ�.  
	// ��� _node._data�� �̸� ���� �ؾ���.
	// 
	// ��뿹��:
	//  CList<aaa*> cl;
	//	CList<aaa*>::iterator iter;
	//
	//	for (int i = 0; i < 10; ++i)
	//	{
	//		aaa* temp = new aaa(i);
	//
	//		cl.push_back(temp);
	//	}
	//
	//
	//
	//	for (iter = cl.begin(); iter != cl.end();)
	//	{
	//		aaa* temp = *iter;
	//
	//		delete temp;
	//
	//		iter = cl.erase(iter);
	//	}
	// 
	//-----------------------------------------------------
	inline iterator erase(const iterator _Where);
	// T Date�� �������� ��ġ�ϴ� ��� ������ ����
	inline void remove(const T& data);
};





//////////////////////////////////////////////////////////////////////////////////////////
// iterator Ŭ���� 
// 
// ������ �����ε�
// 
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
inline typename CList<T>::iterator CList<T>::iterator::operator++ (int)		// ���� ����
{
	CList<T>::iterator iter(this->_node);
	this->_node = this->_node->_Next;

	return iter;
}

template <typename T>
inline typename CList<T>::iterator& CList<T>::iterator::operator++ ()	// ���� ����
{
	this->_node = this->_node->_Next;
	return *this;
}

template <typename T>
inline typename CList<T>::iterator CList<T>::iterator::operator-- (int)
{
	CList<T>::iterator iter(this->_node);
	this->_node = this->_node->_Prev;

	return iter;
}

template <typename T>
inline typename CList<T>::iterator& CList<T>::iterator::operator--()
{
	this->_node = this->_node->_Prev;
	return *this;
}

template<typename T>
inline bool CList<T>::iterator::operator==(const iterator& other)
{
	if (this->_node == other._node)
		return true;
	else
		return false;
}

template<typename T>
inline bool CList<T>::iterator::operator!=(const iterator& other)
{
	if (this->_node == other._node)
		return false;
	else
		return true;
}

template<typename T>
inline typename CList<T>::iterator& CList<T>::iterator::operator+=(int num)
{
	// TODO: ���⿡ return ���� �����մϴ�.
	int iCnt;
	for (iCnt = 0; iCnt < 2; ++iCnt)
	{
		_node = _node->_Next;
	}
	return *this;
}

template<typename T>
inline T& CList<T>::iterator::operator*()
{
	// TODO: ���⿡ return ���� �����մϴ�.
	return this->_node->_data;
}















//////////////////////////////////////////////////////////////////////////////////////////
// ���� ���Ͽ� ���Ǻ� �ۼ�
// 
// CList Ŭ����
//////////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------------
// ������, �Ҹ���
// 
// �����ڿ� �Ҹ��ڴ� Ŭ������ ���ÿ� �Ҵ�Ǿ� �����ǰų�, new �� ����Ͽ� �������� �Ҵ��� ��
// �ڵ����� �����ڰ� �����Ѵ�. 
// 
// ��, malloc, free �� ����Ͽ� ���� �Ҵ��� �� ��� �����ڿ� �Ҹ��ڰ� �������� �ʴ´�.
//----------------------------------------------------------------------------------------
template <typename T>
CList<T>::CList() : _size(0), _MemPool{0,false}
{
	_head._Prev = NULL;
	_head._Next = &_tail;

	_tail._Next = NULL;
	_tail._Prev = &_head;
}

template <typename T>
inline CList<T>::~CList()
{
	// ������ �ȿ��� ����ϰ�, �������� ����� �Ҵ�� ��� �����͸� ������ �Ѵ�.
	// �ƴϸ� �Ҵ�� �޸𸮸� �����ϴ°� ���� ������� ��?

	// ���� list�� ��� �Ҹ��ڸ� ȣ���Ű�� ���� 
	// -> �Ҹ� ��Ű�� ���� ������� ��

	// ���� ������ T�� ���Ͽ� ���� �������̴�. 

	// malloc���� �Ҵ�� ���Ḯ��Ʈ�� ������ �޸𸮸� �����Ͽ��� �Ѵ�.

	Node* temp = _head._Next;
	Node* Prev;
	Node* Next;

	while (temp != &_tail)
	{
		Prev = temp->_Prev;
		Next = temp->_Next;

		Prev->_Next = Next;
		Next->_Prev = Prev;

		free(temp);
		temp = Next;
	}

	// printf_s("�Ҹ��� ������? \n");
	// �����Ѵ�.
}

//----------------------------------------------------------------------------------------
// Ŭ������ �Լ�
// 
//----------------------------------------------------------------------------------------
#ifdef _DEBUG
#include <iostream>
template <typename T>
void CList<T>::printAll(void)
{
	Node* curNode;

	for (curNode = _head._Next; curNode != &_tail; curNode = curNode->_Next)
	{
		std::cout << curNode->_data << ',';
	}

	std::cout << "\n reverse \n";

	for (curNode = _tail._Prev; curNode != &_head; curNode = curNode->_Prev)
	{
		std::cout << curNode->_data << ',';
	}

	std::cout << std::endl;

}
#endif // _DEBUG



template<typename T>
inline T& CList<T>::front(void)
{
	// TODO: ���⿡ return ���� �����մϴ�.
	return _head._Next->_data;
}

template<typename T>
inline T& CList<T>::back(void)
{
	// TODO: ���⿡ return ���� �����մϴ�.
	return _tail._Prev->_data;

}

//////////////////////////////////////////////////////////////////////////////////////////
// �� �Լ����� ���� list������ �ζ������� ����Ǿ� �ִ�. 
// 
// ���Ǹ� �̷��� �ۼ��� �� ������ �ζ��� ��ġ�� �ű��.
// 
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
typename CList<T>::iterator CList<T>::begin(void)
{
	// �ƹ��͵� ������ &_tail�� ��ȯ��
	CList<T>::iterator iter(_head._Next);
	return iter;
}

template <typename T>
typename CList<T>::iterator CList<T>::end(void)
{
	CList<T>::iterator iter(&_tail);
	return iter;
}

template <typename T>
inline void CList<T>::push_front(const T& data)
{
	Node* pNewNode = _MemPool.Alloc();

	pNewNode->_data = data;
	pNewNode->_Next = _head._Next;
	pNewNode->_Prev = &_head;

	_head._Next->_Prev = pNewNode;
	_head._Next = pNewNode;

	++_size;
}

template <typename T>
void CList<T>::push_back(const T& data)
{
	Node* pNewNode = _MemPool.Alloc();

	pNewNode->_data = data;
	pNewNode->_Next = &_tail;
	pNewNode->_Prev = _tail._Prev;

	_tail._Prev->_Next = pNewNode;
	_tail._Prev = pNewNode;

	// ++_size;
	++this->_size;
}

template <typename T>
void CList<T>::pop_back(void)
{
	if (empty())
		return;

	Node* removeNode = _tail._Prev;
	Node* prevNode = removeNode->_Prev;

	prevNode->_Next = &_tail;
	_tail._Prev = prevNode;

	--_size;

	_MemPool.Free(removeNode);
}

template <typename T>
void CList<T>::pop_front(void)
{
	if (empty())
		return;

	Node* removeNode = _head._Next;
	Node* nextNode = removeNode->_Next;

	nextNode->_Prev = &_head;
	_head._Next = nextNode;

	--_size;
	_MemPool.Free(removeNode);
}

template<typename T>
inline bool CList<T>::empty(void)
{
	return !_size;
}

template<typename T>
typename CList<T>::iterator CList<T>::erase(const iterator _Where)
{
	Node* CurNode = _Where._node;
	Node* NextNode = _Where._node->_Next;
	Node* PreNode = _Where._node->_Prev;

	if (CurNode == &_head)
		return CurNode->_Next;

	if (CurNode == &_tail)
		return NULL;

	PreNode->_Next = NextNode;
	NextNode->_Prev = PreNode;

	--_size;

	//---------------------------------------------
	//delete CurNode;
	// push�� malloc�ε�
	// erase�� delete....
	// ������ ���� ���µ� 
	// �޸� ����Ž�� new �����ε�����
	// �޸� ������ ������ �̻��� ���� ����
	//---------------------------------------------
	_MemPool.Free(CurNode);
	return CList<T>::iterator(NextNode);
}

template<typename T>
inline void CList<T>::remove(const T& data)
{
	CList<T>::iterator iter;
	for (iter = begin(); iter != end(); )
	{
		if ((*iter) == data)
		{
			iter = erase(iter);
		}
		else
			++iter;
	}
}








#endif // !__CLIST_H__

