
#ifndef __RED_BLACK_TREE_H__
#define __RED_BLACK_TREE_H__

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <stack>

namespace OreoPizza
{
	enum NODE_COLOR
	{
		BLACK = 0,
		RED = 1
	};
	struct st_NODE
	{
		st_NODE* _pParent;
		st_NODE* _pLeft;
		st_NODE* _pRight;

		NODE_COLOR _Color;

		int _key;

		st_NODE() 
		{
			_pParent = nullptr;
			_pLeft = nullptr;
			_pRight = nullptr;

			_Color = RED;

			_key = NULL;
		}

		st_NODE(int key, st_NODE* pNode)
		{
			_pParent = pNode;
			_pLeft = pNode;
			_pRight = pNode;

			_Color = RED;

			_key = key;
		}
	};

	class C_RED_BLACK_TREE
	{
	public:
		C_RED_BLACK_TREE();
		~C_RED_BLACK_TREE();

		void insert(const int key);
		bool find(const int key);
		void traverse(int Choice);
		bool erase(const int key);

		void leftRotate(st_NODE* cur_Node);
		void rightRotate(st_NODE* cur_Node);
		void RBInsertFixup(st_NODE* cur_Node);

		void inorder();
		void inorderHelper(st_NODE* node, int deep);


		st_NODE* _st_Root;
		st_NODE _Nil;
	};

	OreoPizza::C_RED_BLACK_TREE::C_RED_BLACK_TREE() :_st_Root(&_Nil)
	{
		_Nil._pParent = &_Nil;
		_Nil._pLeft = &_Nil;
		_Nil._pRight = &_Nil;
		_Nil._Color = BLACK;
	}

	OreoPizza::C_RED_BLACK_TREE::~C_RED_BLACK_TREE()
	{
	}

	inline void C_RED_BLACK_TREE::insert(const int key)
	{
		st_NODE* parents = &_Nil;
		st_NODE* current = _st_Root;
		st_NODE* New_Node = new st_NODE(key, &_Nil);

		while (current != &_Nil)
		{
			parents = current;
			if (key < current->_key)
				current = current->_pLeft;
			else
				current = current->_pRight;
		}

		New_Node->_pParent = parents;

		if (parents == &_Nil)
		{
			_st_Root = New_Node;
		}
		else if(key < parents->_key)
		{
			parents->_pLeft = New_Node;
		}
		else
		{
			parents->_pRight = New_Node;
		}

		RBInsertFixup(New_Node);
	}

	inline void C_RED_BLACK_TREE::leftRotate(st_NODE* x)
	{
		st_NODE* y = x->_pRight;
		x->_pRight = y->_pLeft;

		// y가 nil인 경우? 아니면 Null은 nil인가?
		// 큰 의미 없다. 
		if (y->_pLeft != &_Nil)
		{
			y->_pLeft->_pParent = x;
		}

		y->_pParent = x->_pParent;

		if (x->_pParent == &_Nil)	// x is root 
			_st_Root = y;
		else if (x == x->_pParent->_pLeft)				// x is left child
			x->_pParent->_pLeft = y;
		else											// x is right child 
			x->_pParent->_pRight = y;

		y->_pLeft = x;
		x->_pParent = y;
	}

	inline void C_RED_BLACK_TREE::rightRotate(st_NODE* x)
	{
		st_NODE* y = x->_pLeft;
		x->_pLeft = y->_pRight;

		if (y->_pRight != &_Nil)
		{
			y->_pRight->_pParent = x;
		}

		y->_pParent = x->_pParent;
		if (x->_pParent == &_Nil)
			_st_Root = y;
		else if (x == x->_pParent->_pRight)
			x->_pParent->_pRight= y;
		else
			x->_pParent->_pLeft = y;

		y->_pRight = x;
		x->_pParent = y;
	}

	inline void C_RED_BLACK_TREE::RBInsertFixup(st_NODE* z)
	{
		st_NODE* y;

		while (z->_pParent->_Color == RED) 
		{
			if (z->_pParent == z->_pParent->_pParent->_pLeft) //z.parent is the left child
			{ 

				 y = z->_pParent->_pParent->_pRight; //uncle of z

				if (y->_Color == RED)				//case 1
				{ 
					z->_pParent->_Color = BLACK;
					y->_Color = BLACK;
					z->_pParent->_pParent->_Color = RED;
					z = z->_pParent->_pParent;
				}
				else
				{
					if (z == z->_pParent->_pRight)	//case2
					{
						z = z->_pParent; //marked z.parent as new z
						leftRotate(z);
					}
					//case3
					z->_pParent->_Color = BLACK; //made parent black
					z->_pParent->_pParent->_Color = RED; //made parent red
					rightRotate(z->_pParent->_pParent);
				}
			}
			else	//z.parent is the right child
			{
				y = z->_pParent->_pParent->_pLeft; //uncle of z

				if (y->_Color == RED)
				{
					z->_pParent->_Color = BLACK;
					y->_Color = BLACK;
					z->_pParent->_pParent->_Color = RED;
					z = z->_pParent->_pParent;
				}
				else 
				{
					if (z == z->_pParent->_pLeft)
					{
						z = z->_pParent; //marked z.parent as new z
						rightRotate(z);
					}
					z->_pParent->_Color = BLACK; //made parent black
					z->_pParent->_pParent->_Color = RED; //made parent red
					leftRotate(z->_pParent->_pParent);
				}
			}
		}
		_st_Root->_Color = BLACK;
	}



	inline void OreoPizza::C_RED_BLACK_TREE::inorder()
	{
		inorderHelper(_st_Root, 0);
	}

	inline void OreoPizza::C_RED_BLACK_TREE::inorderHelper(st_NODE* node, int deep)
	{
		if (node == nullptr)
		{
			printf("nullptr로 종료합니다. \n");
			return;
		}

		if (node == &_Nil)
		{
			printf("level: %d / _Nil로 종료합니다 / Color: %s \n", deep, node->_Color == RED ? "RED" : "BLACK");
			return;
		}


		inorderHelper(node->_pLeft, deep + 1);
		printf("level: %d / key: %d / Color: %s ", deep, node->_key, node->_Color == RED ? "RED" : "BLACK");

		if (node == _st_Root)
			printf("/ Root 입니다. \n");
		else
			printf("\n");

		inorderHelper(node->_pRight, deep + 1);
	}
}





#endif // !__RED_BLACK_TREE_H__

