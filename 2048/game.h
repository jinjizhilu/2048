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
const int VALID_ACTION_MAX = GRID_NUM * 2;
const int WIN_CONDITION = 11;

class Board
{
public:
	enum Side
	{
		E_PLAYER,
		E_SYSTEM,
	};

	enum Direction
	{
		E_UP,
		E_LEFT,
		E_RIGHT,
		E_DOWN,
		E_DIRECTION_MAX,
	};

	Board();

	void Clear();
	void Print();
	bool Move(Direction d);
	bool Check(Direction d);

	static int Coord2Id(int row, int col);
	static void Id2Coord(int id, int &row, int &col);

	array<char, GRID_NUM> grids;
	int maxValue;

private:
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

	GameBase();
	void Init();
	bool IsGameFinish();
	int GetSide();
	int GetNextMove();
	float CalcScore();
	void GetValidActions(array<uint8_t, VALID_ACTION_MAX> &result, int &count);

	void Move(int action);
	bool PlayerMove(int direction);
	void UpdateValidGrids();
	void RandomGenerate();
	void Generate(int id, int value);
	void CheckLoseCondition();

	static int EncodeAction(int id, int value);
	static void DecodeAction(int action, int &id, int &value);

	int validGridCount;
	array<uint8_t, GRID_NUM> validGrids;
	Board board;
	int state;
	int turn;
	int lastMove;
};

class Game : private GameBase
{
public:
	int GetState() { return state; }
	bool IsGameFinish() { return GameBase::IsGameFinish(); }

	void Print();
	bool Move(int direction);
	static string Move2Str(int direction);
};
