

#ifndef __A_STAR_H__
#define __A_STAR_H__

#include <map>

using namespace std;

struct st_A_START_NODE
{
	int _pY;
	int _pX;

	int _Y;
	int _X;

	// 가중치
	double	_G;		// 출발점으로 부터의 이동 거리, 유클리드(대각선)
	int		_H;		// 목적지와의 거리, 맨헤튼(가로 + 세로)
	double	_F;		// G + H

	bool isOpenList;
};

class A_STAR
{
public:
	A_STAR();
	~A_STAR();

	bool find(int sY, int sX, int eY, int eX, HWND hWnd);
	st_A_START_NODE* GetNextPosition(void);

	map<pair<int, int>, st_A_START_NODE>::iterator BeginIter(void);
	map<pair<int, int>, st_A_START_NODE>::iterator EndIter(void);

private:


	// Key: F ... Value: Node
	multimap<double, st_A_START_NODE>		_OpenList;

	// Key: <y, x> ... Value: Node
	map<pair<int, int>, st_A_START_NODE>	_CloseList;


	// 상하좌우 좌상, 우상, 좌하, 우하
	int _dy[8] = { -1, 1, 0 ,0, -1, -1, 1, 1 };
	int _dx[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };


	//-------------------------------------------------------------------------
	//  출력용
	//-------------------------------------------------------------------------
	int _eY;
	int _eX;
	map<pair<int, int>, st_A_START_NODE>::iterator	_PathIter;
};


#endif // !__A_STAR_H__
