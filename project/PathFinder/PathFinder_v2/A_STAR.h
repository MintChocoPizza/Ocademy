

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

	// ����ġ
	double	_G;		// ��������� ������ �̵� �Ÿ�, ��Ŭ����(�밢��)
	int		_H;		// ���������� �Ÿ�, ����ư(���� + ����)
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


	// �����¿� �»�, ���, ����, ����
	int _dy[8] = { -1, 1, 0 ,0, -1, -1, 1, 1 };
	int _dx[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };


	//-------------------------------------------------------------------------
	//  ��¿�
	//-------------------------------------------------------------------------
	int _eY;
	int _eX;
	map<pair<int, int>, st_A_START_NODE>::iterator	_PathIter;
};


#endif // !__A_STAR_H__
