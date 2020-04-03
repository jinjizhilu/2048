#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <list>

#pragma warning (disable:4244)
#pragma warning (disable:4018)

using namespace std;

const int BOARD_SIZE = 4;
const int GRID_NUM = BOARD_SIZE * BOARD_SIZE;

class Board
{
public:
	enum Direction {
		E_UP,
		E_LEFT,
		E_RIGHT,
		E_DOWN,
	};

	Board();

	void Clear();
	void Print();
	bool Move(Direction d);

	static int Coord2Id(int row, int col);
	static void Id2Coord(int id, int &row, int &col);

	array<char, GRID_NUM> grids;

private:
	void PrintHSplitLine();
	void PrintVSplitLine();
};

class Game
{
public:
	enum State {
		E_NORMAL,
		E_WIN,
		E_LOSE,
	};

	Game();
	void Init();
	void Print();
	bool IsGameFinish();
	bool Move(int direction);

private:
	
	void UpdateValidGrids();
	void Generate();

	int validGridCount;
	array<char, GRID_NUM> validGrids;
	Board board;
	int state;
	int turn;
};

