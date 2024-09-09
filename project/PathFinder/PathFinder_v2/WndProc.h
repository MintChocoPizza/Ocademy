

#ifndef __WNDPROC_H__
#define __WNDPROC_H__


struct st_Position
{
	bool _bErase;
	int _y;
	int _x;
};


// extern int g_Tile[100][100];
//extern int GRID_SIZE;
//extern const int GRID_WIDTH;
//extern const int GRID_HEIGHT;




LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



#endif // !__WNDPROC_H__


