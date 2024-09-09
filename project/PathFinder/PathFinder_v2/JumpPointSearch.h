
//----------------------------------------------------------------------------------------------
// 
// ���� ������ ���ؼ���� �Լ��� ���� ������ �ʿ䰡 �ִ�.
// 
//----------------------------------------------------------------------------------------------

#ifndef __JUMPPOINTSEARCH_H__
#define __JUMPPOINTSEARCH_H__

enum Direction
{
	UU = 0,
	DD,
	LL,
	RR,

	LU,
	RU,
	LD,
	RD,
	Null
};

struct st_JPS_NODE
{
	int _pY;
	int _pX;

	int _Y;
	int _X;

	// ����ġ 
	double	_G;		// ��������� ������ �̵� �Ÿ�, ��Ŭ����(�밢��)
	int		_H;		// ���������� �Ÿ�, ����ư(���� + ����)
	double	_F;		// G + H

	Direction direction_Parents;

	// ���� ���� OpenList & CloseList ���ʿ� ����ִ�. 
	// �ش� ��尡 OpenList�� ������� ������ ǥ���Ѵ�.
	bool isOpenList;
};

class CJumpPointSearch
{
public:
	CJumpPointSearch();
	~CJumpPointSearch();

	//---------------------------------------------------------------
	// Ŭ������ ����.
	// ����Ʈ ���� �� �� ����.
	//---------------------------------------------------------------
	void clear(void);


	//---------------------------------------------------------------
	// ��ã�⸦ �����Ѵ�. 
	//---------------------------------------------------------------
	bool find(int sY, int sX, int eY, int eX, HWND hWnd);

	//---------------------------------------------------------------
	//	�־��� ���⿡ ���Ͽ� Ž���ϰ�, ��带 �����Ѵ�. 
	//---------------------------------------------------------------
	bool JumpUU(st_JPS_NODE* st_Cur_Node, bool CreateNode);
	bool JumpDD(st_JPS_NODE* st_Cur_Node, bool CreateNode);
	bool JumpLL(st_JPS_NODE* st_Cur_Node, bool CreateNode);
	bool JumpRR(st_JPS_NODE* st_Cur_Node, bool CreateNode);
	bool JumpLU(st_JPS_NODE* st_Cur_Node, bool CreateNode);
	bool JumpRU(st_JPS_NODE* st_Cur_Node, bool CreateNode);
	bool JumpLD(st_JPS_NODE* st_Cur_Node, bool CreateNode);
	bool JumpRD(st_JPS_NODE* st_Cur_Node, bool CreateNode);


	//---------------------------------------------------------------
	// �ڳʸ� �˻��Ѵ�.
	//---------------------------------------------------------------
	bool check(int y, int x);



public:
	// Key: F	... Value: Node
	std::multimap<double, st_JPS_NODE*>			_OpenList;

	// Key: <y,x> ... Value: Node
	std::map<std::pair<int, int>, st_JPS_NODE*>	_CloseList;


	// �����¿� �»�, ���, ����, ����
	int _dy[8] = { -1, 1, 0 ,0, -1, -1, 1, 1 };
	int _dx[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };

	int _eY;
	int _eX;

};
#endif // !__JUMPPOINTSEARCH_H__
