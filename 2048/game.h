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
const int LINE_DICT_SIZE = 16 * 16 * 16 * 16;

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

	array<char, GRID_NUM> grids;

private:
	static int Coord2Id(int row, int col);
	static void Id2Coord(int id, int &row, int &col);

	static bool isLineDictReady;
	static array<short, LINE_DICT_SIZE> lineDict;
	static void InitLineDict();
	static int Line2Key(const array<char, BOARD_SIZE> &line);
	static void Key2Line(int key, array<char, BOARD_SIZE> &line);

	void PrintHSplitLine();
	void PrintVSplitLine();
};

class GameBase
{
public:
	enum State {
		E_NORMAL,
		E_WIN,
		E_LOSE,
	};

	void Init();
	bool Move(int move = -1);
	void UpdateValidGrids();
	void Generate(int id = -1);

	int validGridCount;
	array<char, GRID_NUM> validGrids;
	Board board;
	int state;
	int turn;
};

class Game : protected GameBase
{
public:
	Game();

	bool Move(int direction);
	void Print();
	bool IsGameFinish();


private:

};

