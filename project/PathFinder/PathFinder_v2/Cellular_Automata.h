#ifndef __CELLULAR_AUTOMATA_H__
#define __CELLULAR_AUTOMATA_H__

#include <map>

class Cellular_Automata
{
public:
	Cellular_Automata();
	~Cellular_Automata();

	void Update(void);

	void GenCave(void);
	int Near(int x, int y);
	void SmoothMap(void);

	void searchHole(void);
	int BFS(int y, int x);



	const int alive_Prob = 61;
	const int generations = 9;

	int field_type;

	std::multimap<int,int, std::greater<int>> field_type_tree;

};

extern Cellular_Automata g_Cellular_Automata;

#endif // !__CELLULAR_AUTOMATA_H__

