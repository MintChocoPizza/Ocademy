
#include <stdlib.h>
#include <Windows.h>
#include <windowsx.h>


#include "resource.h"
#include "framework.h"
#include "RegisterClass.h"
#include "Initstance.h"

#include "Tile.h"

#include "WndProc.h"
#include "PathFinder_v2.h"
#include "A_STAR.h"


A_STAR::A_STAR()
{
}

A_STAR::~A_STAR()
{
}

bool A_STAR::find(int sY, int sX, int eY, int eX, HWND hWnd)
{
	double	G;
	int		H;
	double	F;
	
	int		nY;
	int		nX;

	st_A_START_NODE st_CurNode;
	st_A_START_NODE st_NextNode;

	multimap<double, st_A_START_NODE>::iterator		OpenIter;
	map<pair<int, int>, st_A_START_NODE>::iterator	CloseIter;

	_OpenList.clear();
	_CloseList.clear();


	_eY = eY;
	_eX = eX;

	H = abs(sY - eY) + abs(sX - eX);
	st_CurNode = { -1, -1, sY, sX, 0, H, (double)H * 5, true };
	_OpenList.insert(
		pair<double, st_A_START_NODE>(
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


		OpenIter = _OpenList.begin();
		st_CurNode = OpenIter->second;


		CloseIter = _CloseList.find(make_pair(st_CurNode._Y, st_CurNode._X));
		(CloseIter->second).isOpenList = false;
		_OpenList.erase(OpenIter);



		if (st_CurNode._Y == eY && st_CurNode._X == eX)
		{
			_PathIter = CloseIter;
			return true;
		}

		for (int iCnt = 0; iCnt < 8; ++iCnt)
		{
			nY = st_CurNode._Y + _dy[iCnt];
			nX = st_CurNode._X + _dx[iCnt];


			//----------------------------------------------
			// 예외 처리
			// 
			//----------------------------------------------
			if (nY < 0 || nX < 0 || g_GRID_HEIGHT <= nY || g_GRID_WIDTH <= nX)
			{
				// 맵 크기를 벗어 난 경우 
				continue;
			}
			if (g_CTile.st_GRID[nY][nX] == 1)
			{
				// 벽인 경우
				continue;
			}


			//----------------------------------------------
			// nY, nX 좌표의 G, H, F값 계산
			//----------------------------------------------
			G = st_CurNode._G +
				sqrt(pow(st_CurNode._Y - nY, 2) + pow(st_CurNode._X - nX, 2));
			H = abs(nY - eY) + abs(nX - eX);
			F = G + H * 5;


			//----------------------------------------------
			// 경로의 중복 검사 
			// 
			//----------------------------------------------
			CloseIter = _CloseList.find(make_pair(nY, nX));
			if (CloseIter != _CloseList.end())
			{
				// 중복
				st_NextNode = CloseIter->second;

				if (st_NextNode.isOpenList == false)
					continue;

				if (F >= st_NextNode._F)
					continue;
				//if (st_NextNode.isOpenList == false)
				//{
				//	//continue;
				//	MessageBox(NULL, L"F값이 더 작은데 CloseList에 있음", NULL, NULL);

				//	_eY = st_CurNode._Y;
				//	_eX = st_CurNode._X;
				//	//_eY = nY;
				//	//_eX = nX;
				//	_PathIter = CloseIter;
				//	return true;
				//}
				

				st_A_START_NODE st_temp_Node = {
					st_CurNode._Y, st_CurNode._X,
					nY, nX,
					G, H, F,
					true
				};
				for (OpenIter = _OpenList.lower_bound(st_NextNode._F);
					OpenIter != _OpenList.upper_bound(st_NextNode._F);
					++OpenIter)
				{
					if ((OpenIter->second)._Y == nY && (OpenIter->second)._X == nX)
					{
						_OpenList.erase(OpenIter);
						break;
					}
				}

				_OpenList.insert(
					pair<double, st_A_START_NODE>(
						F, st_temp_Node
					)
				);
				(CloseIter->second)._F = F;
				(CloseIter->second)._G = G;
				(CloseIter->second)._H = H;
				(CloseIter->second)._pY = st_CurNode._Y;
				(CloseIter->second)._pX = st_CurNode._X;
				
			}
			else
			{
				// 중복 X
				st_NextNode = {
					st_CurNode._Y, st_CurNode._X,
					nY, nX,
					G, H, F,
					true
				};

				_OpenList.insert(
					pair<double, st_A_START_NODE>(
						F, st_NextNode
					)
				);
				_CloseList.insert(
					make_pair(
						make_pair(nY, nX), st_NextNode
					)
				);
				
			}
		}


	}
	return false;
}

st_A_START_NODE* A_STAR::GetNextPosition(void)
{
	st_A_START_NODE* temp = &(_PathIter->second);

	if (temp->_pY == -1 && temp->_pX == -1)
	{
		_PathIter = _CloseList.find(make_pair(_eY, _eX));
		return NULL;
	}

	_PathIter = _CloseList.find(make_pair(temp->_pY, temp->_pX));

	return temp;
}


map<pair<int, int>, st_A_START_NODE>::iterator A_STAR::BeginIter(void)
{
	return _CloseList.begin();
}


map<pair<int, int>, st_A_START_NODE>::iterator A_STAR::EndIter(void)
{
	return _CloseList.end();
}
