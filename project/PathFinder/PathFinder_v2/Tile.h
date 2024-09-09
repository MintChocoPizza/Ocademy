
#ifndef __TILE_H__
#define __TILE_H__


//extern const int g_GRID_WIDTH;
//extern const int g_GRID_HEIGHT;
#define g_GRID_WIDTH 190
#define g_GRID_HEIGHT 100

#define ROAD 0
#define WALL 1
#define StART_POINT 2
#define END_POINT 3



class Tile
{
public:
	Tile();
	~Tile();

	void Reset();

	struct GRID
	{
		int state;
	};

	int _GRID_SIZE;

	int _Render_StartY;
	int _Render_StartX;

	bool _bErase;


	int _Starting_Y;
	int _Starting_X;
	bool _b_Starting_Erase;

	int _Ending_Y;
	int _Ending_X;
	bool _b_Ending_Erase;

	int st_GRID[g_GRID_HEIGHT][g_GRID_WIDTH];

};

extern Tile g_CTile;


#endif // !__TILE_H__
