
#include <windowsx.h>
#include <map>

#include "resource.h"
#include "framework.h"
#include "PathFinder_v2.h"
#include "RegisterClass.h"
#include "Initstance.h"

#include "Tile.h"

#include "Cellular_Automata.h"
#include "WndProc.h"
#include "A_STAR.h"


//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//

//int GRID_SIZE = 16;
//const int GRID_WIDTH = 100;
//const int GRID_HEIGHT = 100;

int g_xPos;
int g_yPos;

//int g_Tile[100][100];
bool g_bErase = false;
bool g_bDrag = false;
bool findPath = false;  // 길찾기 성공 실패

HPEN g_hGridPen;
HPEN g_hPathPen;
HPEN g_hToParentsPen;

HBRUSH g_hTileBrush;
HBRUSH g_hTileStartBrush;
HBRUSH g_hTileEndBrush;
HBRUSH g_hTileOpenListBrush;
HBRUSH g_hTileCloseListBrush;

HBITMAP g_hMemDCBitmap;
HBITMAP g_hMemDCBitmap_old;

HDC     g_hMemDC;

RECT    g_MemDCRect;

A_STAR g_A_STAR;
st_Position st_Start;
st_Position st_End;
st_Position st_RenderStart;


void RenderGrid(HDC hdc);
void RenderObstacle(HDC hdc);
void RendefUI(HDC hdc);
void RenderPath(HDC hdc);
void RenderList(HDC hdc);

void TestStart(HDC hdc);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {

    case WM_PAINT:
    {
        //// 메모리 DC를 클리어 하고
        //PatBlt(g_hMemDC, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, WHITENESS);

        //// RenderObstacle, RenderGrid를 메모리 DC에 출력
        //RenderObstacle(g_hMemDC);


        //if (findPath == true)
        //{
        //    RenderList(g_hMemDC);
        //    RenderPath(g_hMemDC);
        //}
        //RenderGrid(g_hMemDC);
        //RendefUI(g_hMemDC);



        //// 메모리 DC에 랜더링이 끝나면, 메모리 DC -> 윈도우 DC로의 출력
        //hdc = BeginPaint(hWnd, &ps);
        //BitBlt(hdc, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, g_hMemDC, 0, 0, SRCCOPY);
        //EndPaint(hWnd, &ps);


        //더블버퍼링 없애기
        InvalidateRect(hWnd, NULL, true);
        hdc = BeginPaint(hWnd, &ps);

        if (findPath == true)
        {
            RenderList(hdc);
            RenderPath(hdc);
        }

        RenderObstacle(hdc);
        RenderGrid(hdc);
        RendefUI(hdc);

        EndPaint(hWnd, &ps);

    }
    break;

    case WM_LBUTTONDBLCLK:
    {
        int xPos = GET_X_LPARAM(lParam) - g_CTile._Render_StartX;
        int yPos = GET_Y_LPARAM(lParam) - g_CTile._Render_StartY;
        int iTileX = xPos / g_CTile._GRID_SIZE;
        int iTileY = yPos / g_CTile._GRID_SIZE;


        // 그려진적 없다면 새로 그린다.
        if(g_CTile._b_Starting_Erase = false)
        {
            g_CTile.st_GRID[iTileY][iTileX] = 2;

            g_CTile._Starting_Y = iTileY;
            g_CTile._Render_StartX = iTileX;
            g_CTile._b_Starting_Erase = true;
        }
        else
        {
            // 이미 그려져 있을 때
            // 같은 위치라면, 그림을 지운다.
            if(g_CTile._Starting_Y == iTileY && g_CTile._Starting_X == iTileX)
            {
                g_CTile._b_Starting_Erase = false;
                g_CTile.st_GRID[iTileY][iTileX] = 0;
            }
            else
            {
                // 다른위치인 경우
                g_CTile.st_GRID[g_CTile._Starting_Y][g_CTile._Starting_X] = 0;
                g_CTile.st_GRID[iTileY][iTileX] = 2;
                g_CTile._Starting_Y = iTileY;
                g_CTile._Starting_X = iTileX;
            }
        }

        // 화면을 다시 그린다.
        InvalidateRect(hWnd, NULL, false);
    }
    break;

    case WM_RBUTTONDBLCLK:
    {
        int xPos = GET_X_LPARAM(lParam) - g_CTile._Render_StartX;
        int yPos = GET_Y_LPARAM(lParam) - g_CTile._Render_StartY;
        int iTileX = xPos / g_CTile._GRID_SIZE;
        int iTileY = yPos / g_CTile._GRID_SIZE;



        // 그려진적 없다면 새로 그린다.
        if (g_CTile._b_Ending_Erase == false)
        {
            g_CTile.st_GRID[iTileY][iTileX] = 3;

            g_CTile._Ending_Y = iTileY;
            g_CTile._Ending_X = iTileX;
            g_CTile._b_Ending_Erase = true;
        }
        else
        {
            // 이미 그려져 있을 때
            // 같은 위치라면, 그림을 지운다.
            if(g_CTile._Ending_Y == iTileY && g_CTile._Ending_X == iTileX)
            {
                g_CTile._b_Ending_Erase = false;
                g_CTile.st_GRID[iTileY][iTileX] = 0;
            }
            else
            {
                // 다른위치인 경우
                g_CTile.st_GRID[g_CTile._Ending_Y][g_CTile._Ending_X] = 0;
                g_CTile.st_GRID[iTileY][iTileX] = 3;
                g_CTile._Ending_Y = iTileY;
                g_CTile._Ending_X = iTileX;
            }
        }

        // 화면을 다시 그린다.
        InvalidateRect(hWnd, NULL, false);
    }
    break;

    case WM_LBUTTONDOWN:
        g_bDrag = true;
        {
            int xPos = GET_X_LPARAM(lParam) - g_CTile._Render_StartX;
            int yPos = GET_Y_LPARAM(lParam) - g_CTile._Render_StartY;
            int iTileX = xPos / g_CTile._GRID_SIZE;
            int iTileY = yPos / g_CTile._GRID_SIZE;


            // 첫 선택 타일이 장애물이면 지우기 모드 아니면 장애물 넣기 모드
            //if (g_Tile[iTileY][iTileX] == 1)
            if(g_CTile.st_GRID[iTileY][iTileX] == 1)
                g_CTile._bErase = true;
            else
                g_CTile._bErase = false;
        }
        break;

    case WM_LBUTTONUP:
        g_bDrag = false;
        break;

    case WM_MOUSEMOVE:
    {
        int xPos = GET_X_LPARAM(lParam) - g_CTile._Render_StartX;
        int yPos = GET_Y_LPARAM(lParam) - g_CTile._Render_StartY;


        if (g_bDrag)
        {
            int iTileX = xPos / g_CTile._GRID_SIZE;
            int iTileY = yPos / g_CTile._GRID_SIZE;

            //g_Tile[iTileY][iTileX] = !g_bErase;
            g_CTile.st_GRID[iTileY][iTileX] = !g_CTile._bErase;
        }
        InvalidateRect(hWnd, NULL, false);
    }
    break;

    case WM_MOUSEWHEEL:
    {
        short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        
        // 마우스 커서의 현재 화면 좌표를 가져온다. 
        ScreenToClient(hWnd, &pt);

        // 마우스 포인터가 그리드를 넘어가면 동작 안함
        //if (pt.x >= (st_RenderStart._x + GRID_WIDTH * GRID_SIZE) || pt.y >= (st_RenderStart._y + GRID_HEIGHT * GRID_SIZE))
        if(pt.x >= (g_CTile._Render_StartX + g_GRID_WIDTH * g_CTile._GRID_SIZE) ||
            pt.y >= (g_CTile._Render_StartY + g_GRID_HEIGHT * g_CTile._GRID_SIZE))
        {
            break;
        }

        // 현재 사각형의 가로, 세로 길이
        int curWidth = g_GRID_WIDTH * g_CTile._GRID_SIZE;
        int curHeight = g_GRID_HEIGHT * g_CTile._GRID_SIZE;



        // 마우스 좌표와 그리드 시작 좌표로 그린 삼각형의 밑변의 길이
        //int xcurbottom = abs(st_renderstart._x - pt.x);
        int xCurBottom = abs(g_CTile._Render_StartX - pt.x);





        // 그리드 사이즈 변경 후 사각형의 가로, 세로 길이
        int ChangeWidth;
        int ChargeHeight;

        // 그리드 사이즈 변경 비율, 그리는 좌표는 정수이기 때문에 나머지는 과감히 자른다. 
        double Percentage_Changed;

        // 변경된 밑변의 길이
        double xChangedBottom;

        // 변경된 사각형의 시작 좌표
        int xChangedStartPos;
        int yChangedStartPos;

        // 현재 윈도우 스크린 사이즈
        int ScreenSizeX = g_MemDCRect.right;
        int ScreenSizeY = g_MemDCRect.bottom;

        // 그리드 사이즈 변경
        if (zDelta > 0)
        {
            g_CTile._GRID_SIZE += 5;
            ChangeWidth = g_GRID_WIDTH * g_CTile._GRID_SIZE;
            ChargeHeight = g_GRID_HEIGHT * g_CTile._GRID_SIZE;

            // 얼마나 커졌는지 확인한다. 
            Percentage_Changed = (double)ChangeWidth / (double)curWidth;

            // 변경된 비율만큼 늘어난 밑변의 길이를 구한다. 
            xChangedBottom = (double)xCurBottom * (double)Percentage_Changed;

            // 현재 마우스의 x좌표를 기준으로 변경된 밑변을 빼서 사각형의 시작 x 좌표를 구한다.
            xChangedStartPos = (int)(pt.x - xChangedBottom);

            // 직선의 방정식으로 y좌표도 구한다.
            //yChangedStartPos = (double)(pt.y - st_RenderStart._y) / (double)(pt.x - st_RenderStart._x) 
            // * (xChangedStartPos - st_RenderStart._x) + st_RenderStart._y;
            yChangedStartPos = (double)(pt.y - g_CTile._Render_StartY) / (double)(pt.x - g_CTile._Render_StartX) 
                * (xChangedStartPos - g_CTile._Render_StartX) + g_CTile._Render_StartY;

        }
        else
        {
            g_CTile._GRID_SIZE -= 5;
            g_CTile._GRID_SIZE = max(g_CTile._GRID_SIZE, 6);


            ChangeWidth = g_GRID_WIDTH * g_CTile._GRID_SIZE;
            ChargeHeight = g_GRID_HEIGHT * g_CTile._GRID_SIZE;

            // 얼마나 작아졌는지 확인한다. 
            Percentage_Changed = (double)curWidth / (double)ChangeWidth;

            // 변경된 비율만큼 줄어든 밑변의 길이를 구한다. 
            xChangedBottom = (double)xCurBottom / Percentage_Changed;

            // 현재 마우스의 x좌표를 기준으로 변경된 밑변을 빼서 사각형의 시작 x 좌표를 구한다.
            xChangedStartPos = pt.x - xChangedBottom;

            // 직선의 방정식으로 y좌표도 구한다.
            //yChangedStartPos = (int)((double)(pt.y - st_RenderStart._y) / (double)(pt.x - st_RenderStart._x) * 
            //    (xChangedStartPos - st_RenderStart._x) + st_RenderStart._y);
            

            yChangedStartPos = (int)((double)(pt.y - g_CTile._Render_StartY) / (double)(pt.x - g_CTile._Render_StartX) * 
                (xChangedStartPos - g_CTile._Render_StartX) + g_CTile._Render_StartY);
            
        }


  


        g_CTile._Render_StartY = min(yChangedStartPos, 0);
        g_CTile._Render_StartX = min(xChangedStartPos, 0);

        if (ChangeWidth <= ScreenSizeX || ChargeHeight <= ScreenSizeY)
        {
            g_CTile._Render_StartY = 0;
            g_CTile._Render_StartX = 0;
        }
        
        InvalidateRect(hWnd, NULL, false);
    }
    break;

    case WM_KEYDOWN:
    {
        if (wParam == VK_SPACE)
        {
            findPath = false;
            findPath = g_A_STAR.find(g_CTile._Starting_Y, g_CTile._Starting_X, g_CTile._Ending_Y, g_CTile._Ending_X, hWnd);
        }
        else if (wParam == 'W')
        {
            // 현재 윈도우 스크린 세로 사이즈
            int ScreenSizeY = g_MemDCRect.bottom;
            // 현재 사각형의 세로 길이
            int curHeight = g_GRID_HEIGHT * g_CTile._GRID_SIZE;

            if (curHeight < ScreenSizeY)
                break;

            g_CTile._Render_StartY += g_CTile._GRID_SIZE;
            if (g_CTile._Render_StartY > 0)
                g_CTile._Render_StartY = 0;

          }
        else if (wParam == 'S')
        {
            // 현재 윈도우 스크린 세로 사이즈
            int ScreenSizeY = g_MemDCRect.bottom;
            // 현재 사각형의 세로 길이
            int curHeight = g_GRID_HEIGHT * g_CTile._GRID_SIZE;

            if (curHeight < ScreenSizeY)
                break;
            
            //if (st_RenderStart._y + curHeight + GRID_SIZE < ScreenSizeY)
            if(g_CTile._Render_StartY + curHeight + g_CTile._GRID_SIZE < ScreenSizeY)
                break;

            g_CTile._Render_StartY -= g_CTile._GRID_SIZE;

        }
        else if (wParam == 'A')
        {
            // 현재 윈도우 스크린 가로 사이즈
            int ScreenSizeX = g_MemDCRect.right;
            // 현재 사각형의 가로 길이
            int curWidth = g_GRID_WIDTH * g_CTile._GRID_SIZE;

            if (curWidth < ScreenSizeX)
                break;


            g_CTile._Render_StartX += g_CTile._GRID_SIZE;
            if (g_CTile._Render_StartX > 0)
                g_CTile._Render_StartX = 0;
        }
        else if (wParam == 'D')
        {
            // 현재 윈도우 스크린 가로 사이즈
            int ScreenSizeX = g_MemDCRect.right;
            // 현재 사각형의 가로 길이
            int curWidth = g_GRID_WIDTH * g_CTile._GRID_SIZE;

            if (curWidth < ScreenSizeX)
                break;

            if (g_CTile._Render_StartX + curWidth + g_CTile._GRID_SIZE < ScreenSizeX)
                break;

            g_CTile._Render_StartX -= g_CTile._GRID_SIZE;
        }
        else if (wParam == 'R')
        {
            findPath = false;
            g_CTile.Reset();
        }
        else if (wParam == 'M')
        {
            g_Cellular_Automata.Update();
        }
        else if (wParam == 'T')
        {
            
        }

        InvalidateRect(hWnd, NULL, false);
    }
        break;
    case WM_CREATE:
    {
        g_hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        g_hPathPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
        g_hToParentsPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));


        g_hTileBrush = CreateSolidBrush(RGB(100, 100, 100));
        g_hTileStartBrush = CreateSolidBrush(RGB(0, 255, 0));
        g_hTileEndBrush = CreateSolidBrush(RGB(255, 0, 0));
        g_hTileOpenListBrush = CreateSolidBrush(RGB(0,0,255));
        g_hTileCloseListBrush = CreateSolidBrush(RGB(255,255,0));

        st_RenderStart = { 0, 0 ,0};

        // 윈도우 생성시 현 윈도우 크기와 동일한 메모리 DC 생성
        HDC hdc = GetDC(hWnd);
        GetClientRect(hWnd, &g_MemDCRect);
        g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_MemDCRect.right, g_MemDCRect.bottom);
        g_hMemDC = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);
        g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);
    }
    break;

    case WM_SIZE:
    {
        SelectObject(g_hMemDC, g_hMemDCBitmap_old);
        DeleteObject(g_hMemDC);
        DeleteObject(g_hMemDCBitmap);

        HDC hdc = GetDC(hWnd);

        GetClientRect(hWnd, &g_MemDCRect);
        g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_MemDCRect.right, g_MemDCRect.bottom);
        g_hMemDC = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);

        g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_DESTROY:
        SelectObject(g_hMemDC, g_hMemDCBitmap_old);
        DeleteObject(g_hTileBrush);
        DeleteObject(g_hGridPen);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void RenderGrid(HDC hdc)
{
    int iX = g_CTile._Render_StartX;
    int iY = g_CTile._Render_StartY;
    HPEN hOldPen = (HPEN)SelectObject(hdc, g_hGridPen);

    for (int iCntW = 0; iCntW <= g_GRID_WIDTH; iCntW++)
    {
        MoveToEx(hdc, iX, g_CTile._Render_StartY, NULL);
        LineTo(hdc, iX, g_CTile._Render_StartY + g_GRID_HEIGHT * g_CTile._GRID_SIZE);

        iX += g_CTile._GRID_SIZE;
    }

    for (int iCntH = 0; iCntH <= g_GRID_HEIGHT; ++iCntH)
    {
        MoveToEx(hdc, g_CTile._Render_StartX, iY, NULL);
        LineTo(hdc, g_CTile._Render_StartX + g_GRID_WIDTH * g_CTile._GRID_SIZE, iY);
        iY += g_CTile._GRID_SIZE;
    }
    SelectObject(hdc, hOldPen);
}

void RenderObstacle(HDC hdc)
{
	int iX = 0;
	int iY = 0;
	HBRUSH hOldBrush;
	//HPEN hOldPen;



    hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));	// 뭔가 테두리가 사라짐

	for (int iCntW = 0; iCntW < g_GRID_WIDTH; iCntW++)
	{
		for (int iCntH = 0; iCntH < g_GRID_HEIGHT; ++iCntH)
		{
            // 색깔이 정해진다면.
            if(g_CTile.st_GRID[iCntH][iCntW] == 1)
            {
                // 필기구를 선택하고
                hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileBrush);
                SelectObject(hdc, GetStockObject(NULL_PEN));	// 뭔가 테두리가 사라짐

                // x좌표와 y좌표를 계산한다.
                iX = iCntW * g_CTile._GRID_SIZE + g_CTile._Render_StartX;
                iY = iCntH * g_CTile._GRID_SIZE + g_CTile._Render_StartY;

                // 테두리 크기가 있으므로 +2 한다.
                Rectangle(hdc, iX, iY, iX + g_CTile._GRID_SIZE + 2, iY + g_CTile._GRID_SIZE + 2);
                SelectObject(hdc, hOldBrush);
            }
            else if(g_CTile.st_GRID[iCntH][iCntW] == 2)
            {
                hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileStartBrush);
                SelectObject(hdc, GetStockObject(NULL_PEN));	// 뭔가 테두리가 사라짐

                iX = iCntW * g_CTile._GRID_SIZE + g_CTile._Render_StartX;
                iY = iCntH * g_CTile._GRID_SIZE + g_CTile._Render_StartY;
                // 테두리 크기가 있으므로 +2 한다.
                Rectangle(hdc, iX, iY, iX + g_CTile._GRID_SIZE + 2, iY + g_CTile._GRID_SIZE + 2);
                SelectObject(hdc, hOldBrush);
            }
            else if(g_CTile.st_GRID[iCntH][iCntW] == 3)
            {
                hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileEndBrush);
                SelectObject(hdc, GetStockObject(NULL_PEN));	// 뭔가 테두리가 사라짐

                iX = iCntW * g_CTile._GRID_SIZE + g_CTile._Render_StartX;
                iY = iCntH * g_CTile._GRID_SIZE + g_CTile._Render_StartY;
                // 테두리 크기가 있으므로 +2 한다.
                Rectangle(hdc, iX, iY, iX + g_CTile._GRID_SIZE + 2, iY + g_CTile._GRID_SIZE + 2);
                SelectObject(hdc, hOldBrush);
            }
		}
	}
    SelectObject(hdc, hOldBrush);
}

void RendefUI(HDC hdc)
{
    // 폰트 크기 +16
    HFONT hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PALETTE | FF_DONTCARE, L"Tahoma");
    SelectObject(hdc, hFont);
    SetTextColor(hdc, RGB(0, 0, 0));
    //SetBkMode(hdc, )
    TextOut(hdc, 10, 10, L"시작: 왼더블클릭 / 끝: 오른더블클릭 / 장애물: 드래그 / 시작: 스페이스", lstrlen(L"시작: 왼더블클릭 / 끝: 오른더블클릭 / 장애물: 드래그 / 시작: 스페이스"));
    TextOut(hdc, 10, 26, L"AWSD: 스크롤 / 휠: 확대축소", lstrlen(L"AWSD: 스크롤 / 휠: 확대축소"));
    TextOut(hdc, 10, 42, L"M: 랜덤 맵 생성", lstrlen(L"M 랜덤 맵 생성"));
    TextOut(hdc, 10, 58, L"R: 새로고침", lstrlen(L"R: 새로고침"));
    TextOut(hdc, 10, 74, L"T: 테스트 시작", lstrlen(L"T: 테스트 시작"));
    TextOut(hdc, 10, 74, L"", lstrlen(L""));
}

void RenderPath(HDC hdc)
{
    st_A_START_NODE* temp = g_A_STAR.GetNextPosition();
    int sY;
    int sX;
    int eY;
    int eX;
    HPEN hOldPen = (HPEN)SelectObject(hdc, g_hPathPen);

    while (temp != NULL)
    {
        sY = temp->_Y * g_CTile._GRID_SIZE + g_CTile._GRID_SIZE / 2 + g_CTile._Render_StartY;
        sX = temp->_X * g_CTile._GRID_SIZE + g_CTile._GRID_SIZE / 2 + g_CTile._Render_StartX;
        eY = temp->_pY * g_CTile._GRID_SIZE + g_CTile._GRID_SIZE / 2 + g_CTile._Render_StartY;
        eX = temp->_pX * g_CTile._GRID_SIZE + g_CTile._GRID_SIZE / 2 + g_CTile._Render_StartX;
        MoveToEx(hdc, sX, sY, NULL);
        LineTo(hdc, eX, eY);

        temp = g_A_STAR.GetNextPosition();
    }

    SelectObject(hdc, hOldPen);
}

void RenderList(HDC hdc)
{
    std::map<pair<int, int>, st_A_START_NODE>::iterator iter;
    st_A_START_NODE tempNode;
    HBRUSH hOldBrush;
    HPEN hOldPen;
    HFONT hFont;
    int iX;
    int iY;
    int sX;
    int sY;
    int eX;
    int eY;
    wchar_t Buff[50];

    hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PALETTE | FF_DONTCARE, L"Tahoma");
    hOldPen = (HPEN)SelectObject(hdc, g_hToParentsPen);
    SelectObject(hdc, hFont);
    SetTextColor(hdc, RGB(0, 0, 0));

    for (iter = g_A_STAR.BeginIter(); iter != g_A_STAR.EndIter(); ++iter)
    {
        tempNode = iter->second;

        if (tempNode._X == g_CTile._Starting_X && tempNode._Y == g_CTile._Starting_Y)
            continue;
        if (tempNode._X == g_CTile._Ending_X && tempNode._Y == g_CTile._Ending_Y)
            continue;

        // 타일 색깔 칠하기
        if (tempNode.isOpenList == true)
        {
            hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileOpenListBrush);
            iX = tempNode._X * g_CTile._GRID_SIZE + g_CTile._Render_StartX;
            iY = tempNode._Y * g_CTile._GRID_SIZE + g_CTile._Render_StartY;
            Rectangle(hdc, iX, iY, iX + g_CTile._GRID_SIZE + 2, iY + g_CTile._GRID_SIZE + 2);
            SelectObject(hdc, hOldBrush);
        }
        else
        {
            hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileCloseListBrush);
            iX = tempNode._X * g_CTile._GRID_SIZE + g_CTile._Render_StartX;
            iY = tempNode._Y * g_CTile._GRID_SIZE + g_CTile._Render_StartY;
            Rectangle(hdc, iX, iY, iX + g_CTile._GRID_SIZE + 2, iY + g_CTile._GRID_SIZE + 2);
            SelectObject(hdc, hOldBrush);
        }

        // 부모노드쪽으로 가르키기
        if (tempNode._pX != -1 || tempNode._pY != -1)
        {
            sX = (tempNode._X * g_CTile._GRID_SIZE + g_CTile._Render_StartX) + g_CTile._GRID_SIZE / 2;
            sY = (tempNode._Y * g_CTile._GRID_SIZE + g_CTile._Render_StartY) + g_CTile._GRID_SIZE / 2;
            eX = (tempNode._pX * g_CTile._GRID_SIZE + g_CTile._Render_StartX) + g_CTile._GRID_SIZE / 2;
            eY = (tempNode._pY * g_CTile._GRID_SIZE + g_CTile._Render_StartY) + g_CTile._GRID_SIZE / 2;

            if (eX < tempNode._X * g_CTile._GRID_SIZE + g_CTile._Render_StartX)
                eX = tempNode._X * g_CTile._GRID_SIZE + g_CTile._Render_StartX;
            else if (eX >= tempNode._X * g_CTile._GRID_SIZE + g_CTile._Render_StartX + g_CTile._GRID_SIZE)
                eX = tempNode._X * g_CTile._GRID_SIZE + g_CTile._Render_StartX + g_CTile._GRID_SIZE;

            if (eY < tempNode._Y * g_CTile._GRID_SIZE + g_CTile._Render_StartY)
                eY = tempNode._Y * g_CTile._GRID_SIZE + g_CTile._Render_StartY;
            else if (eY > tempNode._Y * g_CTile._GRID_SIZE + g_CTile._Render_StartY + g_CTile._GRID_SIZE)
                eY = tempNode._Y * g_CTile._GRID_SIZE + g_CTile._Render_StartY + g_CTile._GRID_SIZE;

            MoveToEx(hdc, sX, sY, NULL);
            LineTo(hdc, eX, eY);
        }


        if (g_CTile._GRID_SIZE > 46)
        {
            swprintf_s(Buff, L"G: %0.1lf", tempNode._G);
            TextOut(hdc, iX + 1, iY + 1, Buff, lstrlen(Buff));

            swprintf_s(Buff, L"H: %d", tempNode._H);
            TextOut(hdc, iX + 1, iY + 14, Buff, lstrlen(Buff));

            swprintf_s(Buff, L"F: %0.1lf", tempNode._F);
            TextOut(hdc, iX + 1, iY + 27, Buff, lstrlen(Buff));
        }
    }
    SelectObject(hdc, hOldPen);
}

void TestStart(HDC hdc)
{

}

