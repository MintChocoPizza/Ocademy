
#include <random>
#include <time.h>
#include <queue>

#include "Tile.h"

#include "Cellular_Automata.h"

Cellular_Automata g_Cellular_Automata;

Cellular_Automata::Cellular_Automata() : field_type(4)
{
    
}

Cellular_Automata::~Cellular_Automata()
{
}

void Cellular_Automata::Update(void)
{
    // 맵을 새로 만든다. 
    field_type_tree.clear();
    field_type = 4;

    GenCave();
    SmoothMap();

    searchHole();
}

void Cellular_Automata::GenCave(void)
{
    srand(time(NULL));
    for (int h = 0; h < g_GRID_HEIGHT; h++) {
        for (int w = 0; w < g_GRID_WIDTH; w++) {
            if (w == 0 || w == g_GRID_WIDTH - 1 || h == 0 || h == g_GRID_HEIGHT - 1) g_CTile.st_GRID[h][w] = WALL;
            else if (rand() % 100 > alive_Prob) g_CTile.st_GRID[h][w] = WALL;
            else g_CTile.st_GRID[h][w] = ROAD;
        }
    }
}

int Cellular_Automata::Near(int x, int y)
{
    int count = 0;
    for (int h = y - 1; h < 2 + y; h++)
    {
        for (int w = x - 1; w < 2 + x; w++)
        {
            if (w >= 0 && w < g_GRID_WIDTH && h >= 0 && h < g_GRID_HEIGHT)
            {
                if (!(w == x && h == y)) {
                    if (g_CTile.st_GRID[h][w] == ROAD)
                        ++count;
                }
            }
            else
                ++count;
        }
    }
    return count;
}

void Cellular_Automata::SmoothMap(void)
{
    for (int gen = 0; gen < generations; gen++) {
        for (int h = 1; h < g_GRID_HEIGHT - 1; h++) {
            for (int w = 1; w < g_GRID_WIDTH - 1; w++) {
                int count = Near(w, h);
                if (g_CTile.st_GRID[h][w] == WALL && count >= 6) g_CTile.st_GRID[h][w] = ROAD;
                if (g_CTile.st_GRID[h][w] == ROAD && count <= 3) g_CTile.st_GRID[h][w] = WALL;
            }
        }
    }
}

void Cellular_Automata::searchHole(void)
{
    int iCnt;
    int Max_Hole_Num;
    

    for (int y = 0; y < g_GRID_HEIGHT; ++y)
    {
        for (int x = 0; x < g_GRID_WIDTH; ++x)
        {
            if (g_CTile.st_GRID[y][x] == 0)
            {
                iCnt = BFS(y, x);
                field_type_tree.insert(std::make_pair(iCnt, field_type));
                ++field_type;
            }
        }
    }

    Max_Hole_Num = (field_type_tree.begin())->second;
    
    for (int y = 0; y < g_GRID_HEIGHT; ++y)
    {
        for (int x = 0; x < g_GRID_WIDTH; ++x)
        {
            // 1~4는 필드와 관계 없다.
            if (g_CTile.st_GRID[y][x] < 4)
                continue;
            
            if (g_CTile.st_GRID[y][x] == Max_Hole_Num)
                g_CTile.st_GRID[y][x] = 0;
            else
            {
                g_CTile.st_GRID[y][x] = 1;
            }
        }
    }
}

int Cellular_Automata::BFS(int y, int x)
{
    int Breadth_Count = 0;

    const int dx[] = { -1, 1, 0, 0 };
    const int dy[] = { 0, 0, -1, 1 };

    std::queue<std::pair<int, int>> q;
    q.push(std::make_pair(y, x));

    g_CTile.st_GRID[y][x] = field_type;

    while (!q.empty())
    {
        int cur_Y = q.front().first;
        int cur_X = q.front().second;
        q.pop();
        ++Breadth_Count;

        
        for (int i = 0; i < 4; ++i)
        {
            int nX = cur_X + dx[i];
            int nY = cur_Y + dy[i];

            if (nX >= 0 && nX < g_GRID_WIDTH && nY >= 0 && nY < g_GRID_HEIGHT)
            {
                if (g_CTile.st_GRID[nY][nX] != 0)
                    continue;
                q.push(std::make_pair(nY, nX));
                g_CTile.st_GRID[nY][nX] = field_type;
            }
        }
    }
    
    // 방문한 노드의 갯수
    return Breadth_Count;
}

