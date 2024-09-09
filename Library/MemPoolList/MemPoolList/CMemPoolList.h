//----------------------------------------------------------------------------------------------
// 사용법
// 
// CList<T> 변수명;
// 
// 
//----------------------------------------------------------------------------------------------


#ifndef __CLIST_H__
#define __CLIST_H__

#include <new>
#include "CMemoryPool.h"

//////////////////////////////////////////////////////////////////////////////////////////
// 템플릿 클래스의 경우 정의부와 선언부를 분리할 수 없다. 
// 
// 하나의 파일에 작성해야 함
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
	// iter은 Node의 주소를 가지고 있음.
	// iter은 new를 통하여 할당받지 않음. 
	// 그냥 내부에 Node의 주소를 가지고 있음. 
	// 
	// 결과적으로 소멸자 호출을 굳이 할 필요가 없음. 
	// 
	// 또한 메모리 해지또한 필요 없음. 
	// 
	// Node의 메모리만 해지하면 된다.
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
			// 인자로 들어온 Node 포인터를 저장
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

	// 모든 요소를 순회하면 출력한다.
	void printAll(void);

	//-----------------------------------------------------
	// 4로 초기화된 3개의 원소를 할당한다.
	// 
	// ex)
	// it.assign(3,4);
	// 
	//-----------------------------------------------------
	void assign(int count, T data);

	// 맨 앞의 원소를 반환(return), 참조 한다.
	T& front(void);
	// 맨 뒤의 원소를 반환(return), 참조 한다.
	T& back(void);

	//-----------------------------------------------------
	// 맨 앞의 원소를 가리키는 iterator를 반환한다. 
	// 
	// ex) list<int>::iterator iter;
	// iter = lt.begin();
	// 
	//-----------------------------------------------------
	inline iterator begin(void);

	//-----------------------------------------------------
	// 맨 마지막의 원소의 다음을 가리키는 iterator를 반환한다. 
	// 
	// ex) list<int>::iterator iter;
	// iter = It.end();
	// 
	//-----------------------------------------------------
	inline iterator end(void);

	//-----------------------------------------------------
	// 맨 마지막의 원소의 다음을 가리키는 iterator를 반환한다. 
	// 
	// ex) list<int>::iterator iter;
	// iter = It.end();
	// 
	//-----------------------------------------------------

	// 앞쪽으로 원소 data를 삽입한다.
	inline void push_front(const T& data);
	// 뒤쪽으로 원소 data를 삽입한다.
	inline void push_back(const T& data);
	// 맨 마지막 원소를 제거한다.
	inline void pop_back(void);
	// 맨 첫번째 원소를 제거한다.
	inline void pop_front(void);

	// 원소가 비어있는지 확인 
	inline bool empty(void);
	//-----------------------------------------------------
	// iter를 기준으로 데이터 삭제
	// erase는 iterator._node 를 삭제한다.  
	// 고로 _node._data는 미리 삭제 해야함.
	// 
	// 사용예시:
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
	// T Date를 기준으로 일치하는 모든 데이터 삭제
	inline void remove(const T& data);
};





//////////////////////////////////////////////////////////////////////////////////////////
// iterator 클래스 
// 
// 연산자 오버로딩
// 
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
inline typename CList<T>::iterator CList<T>::iterator::operator++ (int)		// 후위 증가
{
	CList<T>::iterator iter(this->_node);
	this->_node = this->_node->_Next;

	return iter;
}

template <typename T>
inline typename CList<T>::iterator& CList<T>::iterator::operator++ ()	// 전위 증가
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
	// TODO: 여기에 return 문을 삽입합니다.
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
	// TODO: 여기에 return 문을 삽입합니다.
	return this->_node->_data;
}















//////////////////////////////////////////////////////////////////////////////////////////
// 같은 파일에 정의부 작성
// 
// CList 클래스
//////////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------------
// 생성자, 소멸자
// 
// 생성자와 소멸자는 클래스가 스택에 할당되어 생성되거나, new 를 사용하여 동적으로 할당할 때
// 자동으로 생성자가 동작한다. 
// 
// 단, malloc, free 를 사용하여 동적 할당을 할 경우 생성자와 소멸자가 동작하지 않는다.
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
	// 스코프 안에서 사용하고, 스코프를 벗어나면 할당된 모든 데이터를 날려야 한다.
	// 아니면 할당된 메모리를 해제하는것 또한 사용자의 몫?

	// 원래 list의 경우 소멸자를 호출시키지 않음 
	// -> 소멸 시키는 것은 사용자의 몫

	// 위의 모든것은 T를 동하여 들어온 데이터이다. 

	// malloc으로 할당된 연결리스트의 노드들은 메모리를 해지하여야 한다.

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

	// printf_s("소멸자 동작함? \n");
	// 동작한다.
}

//----------------------------------------------------------------------------------------
// 클래스의 함수
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
	// TODO: 여기에 return 문을 삽입합니다.
	return _head._Next->_data;
}

template<typename T>
inline T& CList<T>::back(void)
{
	// TODO: 여기에 return 문을 삽입합니다.
	return _tail._Prev->_data;

}

//////////////////////////////////////////////////////////////////////////////////////////
// 이 함수들은 원래 list에서도 인라인으로 선언되어 있다. 
// 
// 정의를 이렇게 작성할 수 있지만 인라인 위치로 옮긴다.
// 
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
typename CList<T>::iterator CList<T>::begin(void)
{
	// 아무것도 없으면 &_tail을 반환함
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
	// push는 malloc인데
	// erase는 delete....
	// 동작은 문제 없는데 
	// 메모리 누수탐지 new 오버로딩에서
	// 메모리 관리를 실패해 이상한 값이 나옴
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

