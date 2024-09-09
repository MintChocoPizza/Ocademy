
#include <string.h>
#include <new>
#include "Tile.h"

//const int g_GRID_WIDTH = 100;
// const int g_GRID_HEIGHT = 100;

Tile g_CTile;

Tile::Tile() : _GRID_SIZE(16), _Render_StartY(0), _Render_StartX(0), 
	_bErase(false),
	_Starting_Y(0),	_Starting_X(0), _b_Starting_Erase(0),
	_Ending_Y(0), _Ending_X(0), _b_Ending_Erase(0)
{
	memset(st_GRID, 0, sizeof(st_GRID));
}

Tile::~Tile()
{
}

void Tile::Reset()
{
	new(&g_CTile) Tile();
}
