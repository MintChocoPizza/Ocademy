
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


	// 클래스 멤버 변수로 eY, eX를 등록해준다.
	_eY = eY;
	_eX = eX;
	
	// 초기 노드를 세팅해준다. 
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
		// OpenList에서 다음 탐색 노드를 하나 꺼낸다.
		OpenIter = _OpenList.begin();
		st_CurNode = OpenIter->second;

		// CloseList에서 해당 노드를 찾아서 isOpenList 체크를 해지하고,
		// OpenList에서 노드를 지운다.
		CloseIter = _CloseList.find(make_pair(st_CurNode->_Y, st_CurNode->_X));
		(CloseIter->second)->isOpenList = false;
		_OpenList.erase(OpenIter);

		// 기저사례: 목적지에 도착했다면 종료한다. 
		if (st_CurNode->_Y == eY && st_CurNode->_X == eX)
		{
			return true;
		}

		// 1. 첫번째 노드는 8방향을 탐색한다. 
		// 2. 그 다음 노드부터는 부모로 부터 온 방향을 생각하여 8 방향이 아닌 제한적인 탐색을 진행한다. 
		// 선택된 방향에서 노드를 만들거나(코너를 찾았다), 길이 막히면 할일이 끝난다. 
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
			// Null 인 경우 부모 노드가 없는 경우이다 == 첫 번째 노드
			// 8방향을 탐색한다. 
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
		// 기저 사례
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// 맵 크기를 벗어 난 경우 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// 벽인 경우
			return false;
		}

		//----------------------------------------------
		// UU 에서 코너인 경우
		// 1. 왼쪽이 막히고, 왼쪽 대각선 위가 뚤린경우
		// 2. 오른쪽이 막히고, 왼쪽 대각선 위가 뚤린경우
		//----------------------------------------------
		if (!check(nY, nX - 1) && check(nY - 1, nX - 1) ||
			!check(nY, nX + 1) && check(nY - 1, nX + 1))
		{
			// 원한다면 노드를 만들지 않고 끝낸다.
			if (CreateNode == false) return true;

			// 현재 위치는 코너이다 ==> OpenList에 Node를 등록한다.
			// 노드에 대한 G, H, F 값 계산.
			G = st_Cur_Node->_G +
				sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
			H = abs(nY - _eY) + abs(nX - _eX);
			F = G + H * 5;

			// 중복 노드를 검색한다. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.
				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;
				
				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 기저 사례
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// 맵 크기를 벗어 난 경우 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// 벽인 경우
			return false;
		}

		//----------------------------------------------
		// DD 에서 코너인 경우
		// 1. 왼쪽이 막히고, 왼쪽 대각선 아래가 뚤린경우
		// 2. 오른쪽이 막히고, 왼쪽 대각선 아래가 뚤린경우
		//----------------------------------------------
		if (!check(nY, nX - 1) && check(nY + 1, nX - 1) ||
			!check(nY, nX + 1) && check(nY + 1, nX + 1))
		{
			// 원한다면 노드를 만들지 않고 끝낸다.
			if (CreateNode == false) return true;

			// 현재 위치는 코너이다 ==> OpenList에 Node를 등록한다.
			
			// 노드에 대한 G, H, F 값 계산.
			G = st_Cur_Node->_G +
				sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
			H = abs(nY - _eY) + abs(nX - _eX);
			F = G + H * 5;

			// 중복 노드를 검색한다. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.
				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;
				
				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 기저 사례
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// 맵 크기를 벗어 난 경우 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// 벽인 경우
			return false;
		}

		//----------------------------------------------
		// LL 에서 코너인 경우
		// 1. 위가 막히고, 왼쪽 대각선 위가 뚤린경우
		// 2. 아래가 막히고, 왼쪽 대각선 아래가 뚤린경우
		//----------------------------------------------
		if (!check(nY - 1, nX) && check(nY - 1, nX - 1) ||
			!check(nY + 1, nX) && check(nY + 1, nX - 1))
		{
			// 원한다면 노드를 만들지 않고 끝낸다.
			if (CreateNode == false) return true;

			// 현재 위치는 코너이다 ==> OpenList에 Node를 등록한다.

			// 노드에 대한 G, H, F 값 계산.
			G = st_Cur_Node->_G +
				sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
			H = abs(nY - _eY) + abs(nX - _eX);
			F = G + H * 5;

			// 중복 노드를 검색한다. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.
				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 기저 사례
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// 맵 크기를 벗어 난 경우 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// 벽인 경우
			return false;
		}

		//----------------------------------------------
		// RR 에서 코너인 경우
		// 1. 위가 막히고, 오른쪽 대각선 위가 뚤린경우
		// 2. 아래가 막히고, 오른쪽 대각선 아래가 뚤린경우
		//----------------------------------------------
		if (!check(nY - 1, nX) && check(nY - 1, nX + 1) ||
			!check(nY + 1, nX) && check(nY + 1, nX + 1))
		{
			// 원한다면 노드를 만들지 않고 끝낸다.
			if (CreateNode == false) return true;

			// 현재 위치는 코너이다 ==> OpenList에 Node를 등록한다.

			// 노드에 대한 G, H, F 값 계산.
			G = st_Cur_Node->_G +
				sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
			H = abs(nY - _eY) + abs(nX - _eX);
			F = G + H * 5;

			// 중복 노드를 검색한다. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.
				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 기저 사례
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// 맵 크기를 벗어 난 경우 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// 벽인 경우
			return false;
		}

		//----------------------------------------------
		// 대각선 이동이 노드를 만들고 중단하는 경우
		// 
		// 1. 코너를 만난 경우
		// 2. 대각선은 좌우를 탐색할 의무 -> 거기서 코너 만난 경우
		// 
		//----------------------------------------------

		// 노드에 대한 G, H, F 값 계산.
		G = st_Cur_Node->_G +
			sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
		H = abs(nY - _eY) + abs(nX - _eX);
		F = G + H * 5;

		//----------------------------------------------
		// LU 에서 코너인 경우
		// 1. 아래가 막히고, 왼쪽 대각선 아래가 뚤린경우
		// 2. 오른쪽이 막히고, 오른쪽 대각선 위가 뚤린경우
		//----------------------------------------------
		if (!check(nY + 1, nX) && check(nY + 1, nX + 1) ||
			!check(nY, nX - 1) && check(nY - 1, nX - 1))
		{
			// 원한다면 노드를 만들지 않고 끝낸다.
			if (CreateNode == false) return true;

			// 현재 위치는 코너이다 ==> OpenList에 Node를 등록한다.
			// 1. 중복 노드를 검색한다. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.

				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 2. 대각선은 좌우를 살피며 가야 한다.
		// 
		// LU의 경우 상, 좌 를 살피며 가야할 의무가 있다.
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
			// 의무를 실행하고, 해당 방향에 코너가 있다면, 현재 자리에 노드를 생성한다. 
			// 여기 또한 중복 검사를 한다. 
			
			// 원한다면 노드를 만들지 않고 끝낸다. 
			if (CreateNode == false) return true;

			// 중복검사를 한다.
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.

				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 기저 사례
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// 맵 크기를 벗어 난 경우 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// 벽인 경우
			return false;
		}

		//----------------------------------------------
		// 대각선 이동이 노드를 만들고 중단하는 경우
		// 
		// 1. 코너를 만난 경우
		// 2. 대각선은 좌우를 탐색할 의무 -> 거기서 코너 만난 경우
		// 
		//----------------------------------------------

		// 노드에 대한 G, H, F 값 계산.
		G = st_Cur_Node->_G +
			sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
		H = abs(nY - _eY) + abs(nX - _eX);
		F = G + H * 5;

		//----------------------------------------------
		// RU 에서 코너인 경우
		// 1. 아래가 막히고, 오른쪽 대각선 아래가 뚤린경우
		// 2. 왼쪽이 막히고, 왼쪽 대각선 위가 뚤린경우
		//----------------------------------------------
		if (!check(nY + 1, nX) && check(nY + 1, nX + 1) ||
			!check(nY, nX - 1) && check(nY - 1, nX - 1))
		{
			// 원한다면 노드를 만들지 않고 끝낸다.
			if (CreateNode == false) return true;

			// 현재 위치는 코너이다 ==> OpenList에 Node를 등록한다.
			// 1. 중복 노드를 검색한다. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.

				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 2. 대각선은 좌우를 살피며 가야 한다.
		// 
		// RU의 경우 상, 좌 를 살피며 가야할 의무가 있다.
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
			// 의무를 실행하고, 해당 방향에 코너가 있다면, 현재 자리에 노드를 생성한다. 
			// 여기 또한 중복 검사를 한다. 
			
			// 원한다면 노드를 만들지 않고 끝낸다. 
			if (CreateNode == false) return true;

			// 중복검사를 한다.
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.

				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 기저 사례
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// 맵 크기를 벗어 난 경우 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// 벽인 경우
			return false;
		}

		//----------------------------------------------
		// 대각선 이동이 노드를 만들고 중단하는 경우
		// 
		// 1. 코너를 만난 경우
		// 2. 대각선은 좌우를 탐색할 의무 -> 거기서 코너 만난 경우
		// 
		//----------------------------------------------

		// 노드에 대한 G, H, F 값 계산.
		G = st_Cur_Node->_G +
			sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
		H = abs(nY - _eY) + abs(nX - _eX);
		F = G + H * 5;

		//----------------------------------------------
		// LD에서 코너인 경우
		// 1. 위가 막히고, 왼쪽 대각선 위가 뚤린경우
		// 2. 오른쪽이 막히고, 오른쪽 대각선 아래가 뚤린경우
		//----------------------------------------------
		if (!check(nY - 1, nX) && check(nY - 1, nX - 1) ||
			!check(nY, nX + 1) && check(nY + 1, nX + 1))
		{
			// 원한다면 노드를 만들지 않고 끝낸다.
			if (CreateNode == false) return true;

			// 현재 위치는 코너이다 ==> OpenList에 Node를 등록한다.
			// 1. 중복 노드를 검색한다. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.

				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 2. 대각선은 좌우를 살피며 가야 한다.
		// 
		// LD의 경우 하, 좌 를 살피며 가야할 의무가 있다.
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
			// 의무를 실행하고, 해당 방향에 코너가 있다면, 현재 자리에 노드를 생성한다. 
			// 여기 또한 중복 검사를 한다. 

			// 원한다면 노드를 만들지 않고 끝낸다. 
			if (CreateNode == false) return true;

			// 중복검사를 한다.
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.

				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 기저 사례
		//----------------------------------------------
		if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
		{
			// 맵 크기를 벗어 난 경우 
			return false;
		}
		if (g_CTile.st_GRID[nY][nX] == 1)
		{
			// 벽인 경우
			return false;
		}

		//----------------------------------------------
		// 대각선 이동이 노드를 만들고 중단하는 경우
		// 
		// 1. 코너를 만난 경우
		// 2. 대각선은 좌우를 탐색할 의무 -> 거기서 코너 만난 경우
		// 
		//----------------------------------------------

		// 노드에 대한 G, H, F 값 계산.
		G = st_Cur_Node->_G +
			sqrt(pow(st_Cur_Node->_Y - nY, 2) + pow(st_Cur_Node->_X - nX, 2));
		H = abs(nY - _eY) + abs(nX - _eX);
		F = G + H * 5;

		//----------------------------------------------
		// LD에서 코너인 경우
		// 1. 위가 막히고, 왼쪽 대각선 위가 뚤린경우
		// 2. 오른쪽이 막히고, 오른쪽 대각선 아래가 뚤린경우
		//----------------------------------------------
		if (!check(nY - 1, nX) && check(nY - 1, nX - 1) ||
			!check(nY, nX + 1) && check(nY + 1, nX + 1))
		{
			// 원한다면 노드를 만들지 않고 끝낸다.
			if (CreateNode == false) return true;

			// 현재 위치는 코너이다 ==> OpenList에 Node를 등록한다.
			// 1. 중복 노드를 검색한다. 
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.

				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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
		// 2. 대각선은 좌우를 살피며 가야 한다.
		// 
		// LD의 경우 하, 좌 를 살피며 가야할 의무가 있다.
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
			// 의무를 실행하고, 해당 방향에 코너가 있다면, 현재 자리에 노드를 생성한다. 
			// 여기 또한 중복 검사를 한다. 

			// 원한다면 노드를 만들지 않고 끝낸다. 
			if (CreateNode == false) return true;

			// 중복검사를 한다.
			iter_CloseList = _CloseList.find(make_pair(nY, nX));
			if (iter_CloseList == _CloseList.end())
			{
				// 중복이 아니다.

				// 노드 생성
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
				// 중복이다.
				// A* 와 마찬가지로 현재의 연결이 더 좋다면 연결 정보를 바꾼다.
				st_Next_Node = iter_CloseList->second;

				// 이미 방문하였다면 F값이 당연히 더 작을 것이다. 
				// 현재 노드의 F 값이 더 크기 때문에 나중에 방문하게 되었다.
				if (st_Next_Node->isOpenList == false)
					return true;

				// 새로 만들게 되는 F 값이 더 크거나 같으면, 굳이 변경할 이유가 없다.
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

				// 중복키가 가능한 multimap 에서 키값을 기준으로 순회를 한다. 
				for (iter_OpenList = _OpenList.lower_bound(st_Next_Node->_F);
					iter_OpenList != _OpenList.upper_bound(st_Next_Node->_F);
					++iter_OpenList)
				{
					// 기존에 있던 노드를 삭제한다. 
					if (iter_OpenList->second->_Y == nY && iter_OpenList->second->_X == nX)
					{
						//delete iter_OpenList->second;
						_OpenList.erase(iter_OpenList);
						break;
					}
				}

				// 새로운 노드를 저장한다.
				_OpenList.insert(
					pair<double, st_JPS_NODE*>(
						F, st_Temp_Node
					)
				);

				// CloseList에 있는 값을 변경한다. 
				// 사실 위의 OpenList 또한 값만 변경이 가능하다.
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

