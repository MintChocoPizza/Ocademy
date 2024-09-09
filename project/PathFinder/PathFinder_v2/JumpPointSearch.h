
//----------------------------------------------------------------------------------------------
// 
// 추후 성능을 위해서라면 함수를 조금 정리할 필요가 있다.
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

	// 가중치 
	double	_G;		// 출발점으로 부터의 이동 거리, 유클리드(대각선)
	int		_H;		// 목적지와의 거리, 맨헤튼(가로 + 세로)
	double	_F;		// G + H

	Direction direction_Parents;

	// 현재 노드는 OpenList & CloseList 양쪽에 들어있다. 
	// 해당 노드가 OpenList에 들어있지 않음을 표시한다.
	bool isOpenList;
};

class CJumpPointSearch
{
public:
	CJumpPointSearch();
	~CJumpPointSearch();

	//---------------------------------------------------------------
	// 클래스를 비운다.
	// 리스트 들을 싹 다 비운다.
	//---------------------------------------------------------------
	void clear(void);


	//---------------------------------------------------------------
	// 길찾기를 시작한다. 
	//---------------------------------------------------------------
	bool find(int sY, int sX, int eY, int eX, HWND hWnd);

	//---------------------------------------------------------------
	//	주어진 방향에 대하여 탐색하고, 노드를 저장한다. 
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
	// 코너를 검사한다.
	//---------------------------------------------------------------
	bool check(int y, int x);



public:
	// Key: F	... Value: Node
	std::multimap<double, st_JPS_NODE*>			_OpenList;

	// Key: <y,x> ... Value: Node
	std::map<std::pair<int, int>, st_JPS_NODE*>	_CloseList;


	// 상하좌우 좌상, 우상, 좌하, 우하
	int _dy[8] = { -1, 1, 0 ,0, -1, -1, 1, 1 };
	int _dx[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };

	int _eY;
	int _eX;

};
#endif // !__JUMPPOINTSEARCH_H__
