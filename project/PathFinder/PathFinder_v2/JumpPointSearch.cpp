
#include <map>
#include <Windows.h>

#include "resource.h"
#include "framework.h"
#include "RegisterClass.h"
#include "Initstance.h"

#include "Tile.h"

#include "WndProc.h"
#include "PathFinder_v2.h"

#include "JumpPointSearch.h"

using namespace std;

CJumpPointSearch::CJumpPointSearch() : _eY(-1), _eX(-1)
{
}

CJumpPointSearch::~CJumpPointSearch()
{
	clear();
}

void CJumpPointSearch::clear(void)
{
	std::multimap<double, st_JPS_NODE*>::iterator				iter_Open_List;
	std::map<std::pair<int, int>, st_JPS_NODE*>::iterator		iter_CloseList;

	for (iter_Open_List = _OpenList.begin(); iter_Open_List != _OpenList.end(); )
	{
		delete iter_Open_List->second;
		iter_Open_List = _OpenList.erase(iter_Open_List);
	}

	for (iter_CloseList = _CloseList.begin(); iter_CloseList != _CloseList.end();)
	{
		delete iter_CloseList->second;
		iter_CloseList = _CloseList.erase(iter_CloseList);
	}
}

bool CJumpPointSearch::find(int sY, int sX, int eY, int eX, HWND hWnd)
{
	int direction;

	double	G;
	int		H;
	double	F;

	int		curY;
	int		curX;
	int		nY;
	int		nX;

	st_JPS_NODE		*st_CurNode;
	st_JPS_NODE		*st_NextNode;

	multimap<double, st_JPS_NODE*>::iterator		OpenIter;
	map<pair<int, int>, st_JPS_NODE*>::iterator	CloseIter;


	// Ŭ���� ��� ������ eY, eX�� ������ش�.
	_eY = eY;
	_eX = eX;
	
	// �ʱ� ��带 �������ش�. 
	H = abs(sY - eY) + abs(sX - eX);
	st_CurNode = new st_JPS_NODE;
	*st_CurNode = { -1, -1, sY, sX, 0, H, (double)H * 5, Null, true };

	_OpenList.insert(
		pair<double, st_JPS_NODE*>(
			(double)H, st_CurNode
		)
	);
	_CloseList.insert(
		make_pair(
			make_pair(sY, sX), st_CurNode
		)
	);



	while (_OpenList.empty() != true)
	{
		// OpenList���� ���� Ž�� ��带 �ϳ� ������.
		OpenIter = _OpenList.begin();
		st_CurNode = OpenIter->second;

		// CloseList���� �ش� ��带 ã�Ƽ� isOpenList üũ�� �����ϰ�,
		// OpenList���� ��带 �����.
		CloseIter = _CloseList.find(make_pair(st_CurNode->_Y, st_CurNode->_X));
		(CloseIter->second)->isOpenList = false;
		_OpenList.erase(OpenIter);

		// �������: �������� �����ߴٸ� �����Ѵ�. 
		if (st_CurNode->_Y == eY && st_CurNode->_X == eX)
		{
			return true;
		}

		// 1. ù��° ���� 8������ Ž���Ѵ�. 
		// 2. �� ���� �����ʹ� �θ�� ���� �� ������ �����Ͽ� 8 ������ �ƴ� �������� Ž���� �����Ѵ�. 
		// ���õ� ���⿡�� ��带 ����ų�(�ڳʸ� ã�Ҵ�), ���� ������ ������ ������. 
		switch (st_CurNode->direction_Parents)
		{
		case UU:
			break;
		case DD:
			break;
		case LL:
			break;
		case RR:
			break;
		case LU:
			break;
		case RU:
			break;
		case LD:
			break;
		case RD:
			break;
		case Null:
			// Null �� ��� �θ� ��尡 ���� ����̴� == ù ��° ���
			// 8������ Ž���Ѵ�. 
		{
			JumpUU(st_CurNode, true);
			JumpDD(st_CurNode, true);
			JumpLL(st_CurNode, true);
			JumpRR(st_CurNode, true);

			
		}
			break;
		default:
			break;
		}
	}
	return false;
}

bool CJumpPointSearch::JumpUU(st_JPS_NODE* st_Cur_Node, bool CreateNode)
{
	map<pair<int, int>, st_JPS_NODE*>::iterator		iter_CloseList;
	std::multimap<double, st_JPS_NODE*>::iterator	iter_OpenList;

	st_JPS_NODE* st_Temp_Node;
	st_JPS_NODE* st_Next_Node;

	double	G;
	int		H;
	double	F;

	int nY;
	int nX;

	nY = st_Cur_Node->_Y;
	nX = st_Cur_Node->_X;

	while (1)
	{
		nY += _dy[0];
		nX += _dx[0];

		//----------------------------------------------
		// ���� ���
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// �� ũ�⸦ ���� �� ��� 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// ���� ���
			return false;
		}

		//----------------------------------------------
		// UU ���� �ڳ��� ���
		// 1. ������ ������, ���� �밢�� ���� �Ը����
		// 2. �������� ������, ���� �밢�� ���� �Ը����
		//----------------------------------------------
		if (!check(nY, nX - 1) && check(nY - 1, nX - 1) ||
			!check(nY, nX + 1) && check(nY - 1, nX + 1))
		{
			// ���Ѵٸ� ��带 ������ �ʰ� ������.
			if (CreateNode == false) return true;

			// ���� ��ġ�� �ڳ��̴� ==> OpenList�� Node�� ����Ѵ�.
			// ��忡 ���� G, H, F �� ���.
			G = st_Cur_Node->_G +
				sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
			H = abs(nY - _eY) + abs(nX - _eX);
			F = G + H * 5;

			// �ߺ� ��带 �˻��Ѵ�. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.
				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					UU,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;
				
				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					UU,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = UU;
			}

			return true;
		}
	}
}

bool CJumpPointSearch::JumpDD(st_JPS_NODE* st_Cur_Node, bool CreateNode)
{
	map<pair<int, int>, st_JPS_NODE*>::iterator		iter_CloseList;
	std::multimap<double, st_JPS_NODE*>::iterator	iter_OpenList;

	st_JPS_NODE* st_Temp_Node;
	st_JPS_NODE* st_Next_Node;

	double	G;
	int		H;
	double	F;

	int nY;
	int nX;

	nY = st_Cur_Node->_Y;
	nX = st_Cur_Node->_X;

	while (1)
	{
		nY += _dy[1];
		nX += _dx[1];

		//----------------------------------------------
		// ���� ���
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// �� ũ�⸦ ���� �� ��� 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// ���� ���
			return false;
		}

		//----------------------------------------------
		// DD ���� �ڳ��� ���
		// 1. ������ ������, ���� �밢�� �Ʒ��� �Ը����
		// 2. �������� ������, ���� �밢�� �Ʒ��� �Ը����
		//----------------------------------------------
		if (!check(nY, nX - 1) && check(nY + 1, nX - 1) ||
			!check(nY, nX + 1) && check(nY + 1, nX + 1))
		{
			// ���Ѵٸ� ��带 ������ �ʰ� ������.
			if (CreateNode == false) return true;

			// ���� ��ġ�� �ڳ��̴� ==> OpenList�� Node�� ����Ѵ�.
			
			// ��忡 ���� G, H, F �� ���.
			G = st_Cur_Node->_G +
				sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
			H = abs(nY - _eY) + abs(nX - _eX);
			F = G + H * 5;

			// �ߺ� ��带 �˻��Ѵ�. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.
				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					DD,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;
				
				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					DD,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = DD;
			}

			return true;
		}
	}
}

bool CJumpPointSearch::JumpLL(st_JPS_NODE* st_Cur_Node, bool CreateNode)
{
	map<pair<int, int>, st_JPS_NODE*>::iterator		iter_CloseList;
	std::multimap<double, st_JPS_NODE*>::iterator	iter_OpenList;

	st_JPS_NODE* st_Next_Node;
	st_JPS_NODE* st_Temp_Node;

	double	G;
	int		H;
	double	F;

	int nY;
	int nX;

	nY = st_Cur_Node->_Y;
	nX = st_Cur_Node->_X;

	while (1)
	{
		nY += _dy[2];
		nX += _dx[2];

		//----------------------------------------------
		// ���� ���
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// �� ũ�⸦ ���� �� ��� 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// ���� ���
			return false;
		}

		//----------------------------------------------
		// LL ���� �ڳ��� ���
		// 1. ���� ������, ���� �밢�� ���� �Ը����
		// 2. �Ʒ��� ������, ���� �밢�� �Ʒ��� �Ը����
		//----------------------------------------------
		if (!check(nY - 1, nX) && check(nY - 1, nX - 1) ||
			!check(nY + 1, nX) && check(nY + 1, nX - 1))
		{
			// ���Ѵٸ� ��带 ������ �ʰ� ������.
			if (CreateNode == false) return true;

			// ���� ��ġ�� �ڳ��̴� ==> OpenList�� Node�� ����Ѵ�.

			// ��忡 ���� G, H, F �� ���.
			G = st_Cur_Node->_G +
				sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
			H = abs(nY - _eY) + abs(nX - _eX);
			F = G + H * 5;

			// �ߺ� ��带 �˻��Ѵ�. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.
				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					LL,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					LL,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = LL;
			}

			return true;
		}
	}
}

bool CJumpPointSearch::JumpRR(st_JPS_NODE* st_Cur_Node, bool CreateNode)
{
	map<pair<int, int>, st_JPS_NODE*>::iterator		iter_CloseList;
	std::multimap<double, st_JPS_NODE*>::iterator	iter_OpenList;

	st_JPS_NODE* st_Next_Node;
	st_JPS_NODE* st_Temp_Node;

	double	G;
	int		H;
	double	F;

	int nY;
	int nX;

	nY = st_Cur_Node->_Y;
	nX = st_Cur_Node->_X;

	while (1)
	{
		nY += _dy[3];
		nX += _dx[3];

		//----------------------------------------------
		// ���� ���
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// �� ũ�⸦ ���� �� ��� 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// ���� ���
			return false;
		}

		//----------------------------------------------
		// RR ���� �ڳ��� ���
		// 1. ���� ������, ������ �밢�� ���� �Ը����
		// 2. �Ʒ��� ������, ������ �밢�� �Ʒ��� �Ը����
		//----------------------------------------------
		if (!check(nY - 1, nX) && check(nY - 1, nX + 1) ||
			!check(nY + 1, nX) && check(nY + 1, nX + 1))
		{
			// ���Ѵٸ� ��带 ������ �ʰ� ������.
			if (CreateNode == false) return true;

			// ���� ��ġ�� �ڳ��̴� ==> OpenList�� Node�� ����Ѵ�.

			// ��忡 ���� G, H, F �� ���.
			G = st_Cur_Node->_G +
				sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
			H = abs(nY - _eY) + abs(nX - _eX);
			F = G + H * 5;

			// �ߺ� ��带 �˻��Ѵ�. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.
				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					RR,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					RR,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = RR;
			}

			return true;
		}
	}
}


bool CJumpPointSearch::JumpLU(st_JPS_NODE* st_Cur_Node, bool CreateNode)
{
	map<pair<int, int>, st_JPS_NODE*>::iterator iter_CloseList;
	multimap<double, st_JPS_NODE*>::iterator	iter_OpenList;


	st_JPS_NODE* st_Next_Node;
	st_JPS_NODE* st_Temp_Node;
	st_JPS_NODE st_Node;

	double	G;
	int		H;
	double	F;

	int nY;
	int nX;

	nY = st_Cur_Node->_Y;
	nX = st_Cur_Node->_X;

	while (1)
	{
		nY += _dy[4];
		nX += _dx[4];

		//----------------------------------------------
		// ���� ���
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// �� ũ�⸦ ���� �� ��� 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// ���� ���
			return false;
		}

		//----------------------------------------------
		// �밢�� �̵��� ��带 ����� �ߴ��ϴ� ���
		// 
		// 1. �ڳʸ� ���� ���
		// 2. �밢���� �¿츦 Ž���� �ǹ� -> �ű⼭ �ڳ� ���� ���
		// 
		//----------------------------------------------

		// ��忡 ���� G, H, F �� ���.
		G = st_Cur_Node->_G +
			sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
		H = abs(nY - _eY) + abs(nX - _eX);
		F = G + H * 5;

		//----------------------------------------------
		// LU ���� �ڳ��� ���
		// 1. �Ʒ��� ������, ���� �밢�� �Ʒ��� �Ը����
		// 2. �������� ������, ������ �밢�� ���� �Ը����
		//----------------------------------------------
		if (!check(nY + 1, nX) && check(nY + 1, nX + 1) ||
			!check(nY, nX - 1) && check(nY - 1, nX - 1))
		{
			// ���Ѵٸ� ��带 ������ �ʰ� ������.
			if (CreateNode == false) return true;

			// ���� ��ġ�� �ڳ��̴� ==> OpenList�� Node�� ����Ѵ�.
			// 1. �ߺ� ��带 �˻��Ѵ�. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.

				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					LU,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					LU,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = LU;
			}
			
			return true;
		}

		//----------------------------------------------
		// 2. �밢���� �¿츦 ���Ǹ� ���� �Ѵ�.
		// 
		// LU�� ��� ��, �� �� ���Ǹ� ������ �ǹ��� �ִ�.
		//----------------------------------------------
		st_Node = {
			st_Cur_Node->_Y, st_Cur_Node->_X,
			nY, nX,
			G,H,F,
			LU,
			true
		};
		if (JumpUU(&st_Node, false) || JumpLL(&st_Node, false))
		{
			// �ǹ��� �����ϰ�, �ش� ���⿡ �ڳʰ� �ִٸ�, ���� �ڸ��� ��带 �����Ѵ�. 
			// ���� ���� �ߺ� �˻縦 �Ѵ�. 
			
			// ���Ѵٸ� ��带 ������ �ʰ� ������. 
			if (CreateNode == false) return true;

			// �ߺ��˻縦 �Ѵ�.
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.

				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					LU,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					LU,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = LU;
			}
		}
	}
}

bool CJumpPointSearch::JumpRU(st_JPS_NODE* st_Cur_Node, bool CreateNode)
{
	map<pair<int, int>, st_JPS_NODE*>::iterator iter_CloseList;
	multimap<double, st_JPS_NODE*>::iterator	iter_OpenList;


	st_JPS_NODE* st_Next_Node;
	st_JPS_NODE* st_Temp_Node;
	st_JPS_NODE st_Node;

	double	G;
	int		H;
	double	F;

	int nY;
	int nX;

	nY = st_Cur_Node->_Y;
	nX = st_Cur_Node->_X;

	while (1)
	{
		nY += _dy[5];
		nX += _dx[5];

		//----------------------------------------------
		// ���� ���
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// �� ũ�⸦ ���� �� ��� 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// ���� ���
			return false;
		}

		//----------------------------------------------
		// �밢�� �̵��� ��带 ����� �ߴ��ϴ� ���
		// 
		// 1. �ڳʸ� ���� ���
		// 2. �밢���� �¿츦 Ž���� �ǹ� -> �ű⼭ �ڳ� ���� ���
		// 
		//----------------------------------------------

		// ��忡 ���� G, H, F �� ���.
		G = st_Cur_Node->_G +
			sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
		H = abs(nY - _eY) + abs(nX - _eX);
		F = G + H * 5;

		//----------------------------------------------
		// RU ���� �ڳ��� ���
		// 1. �Ʒ��� ������, ������ �밢�� �Ʒ��� �Ը����
		// 2. ������ ������, ���� �밢�� ���� �Ը����
		//----------------------------------------------
		if (!check(nY + 1, nX) && check(nY + 1, nX + 1) ||
			!check(nY, nX - 1) && check(nY - 1, nX - 1))
		{
			// ���Ѵٸ� ��带 ������ �ʰ� ������.
			if (CreateNode == false) return true;

			// ���� ��ġ�� �ڳ��̴� ==> OpenList�� Node�� ����Ѵ�.
			// 1. �ߺ� ��带 �˻��Ѵ�. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.

				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					RU,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					RU,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = RU;
			}
			
			return true;
		}

		//----------------------------------------------
		// 2. �밢���� �¿츦 ���Ǹ� ���� �Ѵ�.
		// 
		// RU�� ��� ��, �� �� ���Ǹ� ������ �ǹ��� �ִ�.
		//----------------------------------------------
		st_Node = {
			st_Cur_Node->_Y, st_Cur_Node->_X,
			nY, nX,
			G,H,F,
			RU,
			true
		};
		if (JumpUU(&st_Node, false) || JumpRR(&st_Node, false))
		{
			// �ǹ��� �����ϰ�, �ش� ���⿡ �ڳʰ� �ִٸ�, ���� �ڸ��� ��带 �����Ѵ�. 
			// ���� ���� �ߺ� �˻縦 �Ѵ�. 
			
			// ���Ѵٸ� ��带 ������ �ʰ� ������. 
			if (CreateNode == false) return true;

			// �ߺ��˻縦 �Ѵ�.
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.

				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					RU,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					RU,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = RU;
			}
		}
	}
}

bool CJumpPointSearch::JumpLD(st_JPS_NODE* st_Cur_Node, bool CreateNode)
{
	map<pair<int, int>, st_JPS_NODE*>::iterator iter_CloseList;
	multimap<double, st_JPS_NODE*>::iterator	iter_OpenList;


	st_JPS_NODE* st_Next_Node;
	st_JPS_NODE* st_Temp_Node;
	st_JPS_NODE st_Node;

	double	G;
	int		H;
	double	F;

	int nY;
	int nX;

	nY = st_Cur_Node->_Y;
	nX = st_Cur_Node->_X;

	while (1)
	{
		nY += _dy[6];
		nX += _dx[6];

		//----------------------------------------------
		// ���� ���
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// �� ũ�⸦ ���� �� ��� 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// ���� ���
			return false;
		}

		//----------------------------------------------
		// �밢�� �̵��� ��带 ����� �ߴ��ϴ� ���
		// 
		// 1. �ڳʸ� ���� ���
		// 2. �밢���� �¿츦 Ž���� �ǹ� -> �ű⼭ �ڳ� ���� ���
		// 
		//----------------------------------------------

		// ��忡 ���� G, H, F �� ���.
		G = st_Cur_Node->_G +
			sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
		H = abs(nY - _eY) + abs(nX - _eX);
		F = G + H * 5;

		//----------------------------------------------
		// LD���� �ڳ��� ���
		// 1. ���� ������, ���� �밢�� ���� �Ը����
		// 2. �������� ������, ������ �밢�� �Ʒ��� �Ը����
		//----------------------------------------------
		if (!check(nY - 1, nX) && check(nY - 1, nX - 1) ||
			!check(nY, nX + 1) && check(nY + 1, nX + 1))
		{
			// ���Ѵٸ� ��带 ������ �ʰ� ������.
			if (CreateNode == false) return true;

			// ���� ��ġ�� �ڳ��̴� ==> OpenList�� Node�� ����Ѵ�.
			// 1. �ߺ� ��带 �˻��Ѵ�. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.

				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					LD,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					LD,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = LD;
			}

			return true;
		}

		//----------------------------------------------
		// 2. �밢���� �¿츦 ���Ǹ� ���� �Ѵ�.
		// 
		// LD�� ��� ��, �� �� ���Ǹ� ������ �ǹ��� �ִ�.
		//----------------------------------------------
		st_Node = {
			st_Cur_Node->_Y, st_Cur_Node->_X,
			nY, nX,
			G,H,F,
			LD,
			true
		};
		if (JumpDD(&st_Node, false) || JumpLL(&st_Node, false))
		{
			// �ǹ��� �����ϰ�, �ش� ���⿡ �ڳʰ� �ִٸ�, ���� �ڸ��� ��带 �����Ѵ�. 
			// ���� ���� �ߺ� �˻縦 �Ѵ�. 

			// ���Ѵٸ� ��带 ������ �ʰ� ������. 
			if (CreateNode == false) return true;

			// �ߺ��˻縦 �Ѵ�.
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.

				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					LD,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					LD,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = LD;
			}
		}
	}
}

bool CJumpPointSearch::JumpRD(st_JPS_NODE* st_Cur_Node, bool CreateNode)
{
	map<pair<int, int>, st_JPS_NODE*>::iterator iter_CloseList;
	multimap<double, st_JPS_NODE*>::iterator	iter_OpenList;


	st_JPS_NODE* st_Next_Node;
	st_JPS_NODE* st_Temp_Node;
	st_JPS_NODE st_Node;

	double	G;
	int		H;
	double	F;

	int nY;
	int nX;

	nY = st_Cur_Node->_Y;
	nX = st_Cur_Node->_X;

	while (1)
	{
		nY += _dy[6];
		nX += _dx[6];

		//----------------------------------------------
		// ���� ���
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// �� ũ�⸦ ���� �� ��� 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// ���� ���
			return false;
		}

		//----------------------------------------------
		// �밢�� �̵��� ��带 ����� �ߴ��ϴ� ���
		// 
		// 1. �ڳʸ� ���� ���
		// 2. �밢���� �¿츦 Ž���� �ǹ� -> �ű⼭ �ڳ� ���� ���
		// 
		//----------------------------------------------

		// ��忡 ���� G, H, F �� ���.
		G = st_Cur_Node->_G +
			sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
		H = abs(nY - _eY) + abs(nX - _eX);
		F = G + H * 5;

		//----------------------------------------------
		// LD���� �ڳ��� ���
		// 1. ���� ������, ���� �밢�� ���� �Ը����
		// 2. �������� ������, ������ �밢�� �Ʒ��� �Ը����
		//----------------------------------------------
		if (!check(nY - 1, nX) && check(nY - 1, nX - 1) ||
			!check(nY, nX + 1) && check(nY + 1, nX + 1))
		{
			// ���Ѵٸ� ��带 ������ �ʰ� ������.
			if (CreateNode == false) return true;

			// ���� ��ġ�� �ڳ��̴� ==> OpenList�� Node�� ����Ѵ�.
			// 1. �ߺ� ��带 �˻��Ѵ�. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.

				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					LD,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					LD,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = LD;
			}

			return true;
		}

		//----------------------------------------------
		// 2. �밢���� �¿츦 ���Ǹ� ���� �Ѵ�.
		// 
		// LD�� ��� ��, �� �� ���Ǹ� ������ �ǹ��� �ִ�.
		//----------------------------------------------
		st_Node = {
			st_Cur_Node->_Y, st_Cur_Node->_X,
			nY, nX,
			G,H,F,
			LD,
			true
		};
		if (JumpDD(&st_Node, false) || JumpLL(&st_Node, false))
		{
			// �ǹ��� �����ϰ�, �ش� ���⿡ �ڳʰ� �ִٸ�, ���� �ڸ��� ��带 �����Ѵ�. 
			// ���� ���� �ߺ� �˻縦 �Ѵ�. 

			// ���Ѵٸ� ��带 ������ �ʰ� ������. 
			if (CreateNode == false) return true;

			// �ߺ��˻縦 �Ѵ�.
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// �ߺ��� �ƴϴ�.

				// ��� ����
				st_Next_Node = new st_JPS_NODE;
				*st_Next_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G,H,F,
					LD,
					true
				};

				_OpenList.insert(
					pair<double, st_JPS_NODE*>(F, st_Next_Node)
				);

				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_Next_Node
					)
				);
			}
			else
			{
				// �ߺ��̴�.
				// A* �� ���������� ������ ������ �� ���ٸ� ���� ������ �ٲ۴�.
				st_Next_Node = iter_CloseList->second;

				// �̹� �湮�Ͽ��ٸ� F���� �翬�� �� ���� ���̴�. 
				// ���� ����� F ���� �� ũ�� ������ ���߿� �湮�ϰ� �Ǿ���.
				if (st_Next_Node->isOpenList == false)
					return true;

				// ���� ����� �Ǵ� F ���� �� ũ�ų� ������, ���� ������ ������ ����.
				if (F >= st_Next_Node->_F)
					return true;


				st_Temp_Node = new st_JPS_NODE;
				*st_Temp_Node = {
					st_Cur_Node->_Y, st_Cur_Node->_X,
					nY, nX,
					G, H, F,
					LD,
					true
				};

				// �ߺ�Ű�� ������ multimap ���� Ű���� �������� ��ȸ�� �Ѵ�. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// ������ �ִ� ��带 �����Ѵ�. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// ���ο� ��带 �����Ѵ�.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList�� �ִ� ���� �����Ѵ�. 
				// ��� ���� OpenList ���� ���� ������ �����ϴ�.
				st_Next_Node->_F = F;
				st_Next_Node->_G = G;
				st_Next_Node->_H = H;
				st_Next_Node->_pY = st_Cur_Node->_Y;
				st_Next_Node->_pX = st_Cur_Node->_X;
				st_Next_Node->direction_Parents = LD;
			}
		}
	}
}


bool CJumpPointSearch::check(int y, int x)
{
	if (g_CTile.st_GRID[y][x] == 1) return false;
	else return true;
}

