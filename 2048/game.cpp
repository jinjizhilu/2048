#include "game.h"
#include <cstdlib>

#define max(a, b) ((a > b) ? a : b)

Board::Board()
{
	Clear();
}

void Board::Clear()
{
	grids.fill(0);
}

void Board::PrintHSplitLine()
{
	cout << " ";
	for (int i = 0; i < BOARD_SIZE; ++i)
	{
		printf("------ ");
	}
	cout << endl;
}

void Board::PrintVSplitLine()
{
	cout << "|";
	for (int i = 0; i < BOARD_SIZE; ++i)
	{
		printf("      |");
	}
	cout << endl;
}

void Board::Print()
{
	PrintHSplitLine();

	int id = 0;
	for (int i = 0; i < BOARD_SIZE; ++i)
	{
		PrintVSplitLine();
		cout << "|";

		for (int j = 0; j < BOARD_SIZE; ++j)
		{
			int grid = grids[id++];
			if (grid > 0)
			{
				int num = 1 << (grid);

				if (num >= 100)
					printf(" %4d |", num);
				else
					printf(" %3d  |", num);
			}
			else
				printf("      |");
		}
		cout << endl;
		PrintVSplitLine();
		PrintHSplitLine();
	}
}

bool Board::Move(Direction d)
{
	bool isChange = false;

	for (int i = 0; i < BOARD_SIZE; ++i)
	{
		int validIdCount = 0;
		int validId[BOARD_SIZE] = { 0 };
		int allId[BOARD_SIZE];

		for (int j = 0; j < BOARD_SIZE; ++j)
		{
			int id;
			switch (d)
			{
			case Board::E_UP:
				id = Board::Coord2Id(j, i);
				break;
			case Board::E_LEFT:
				id = Board::Coord2Id(i, j);
				break;
			case Board::E_RIGHT:
				id = Board::Coord2Id(i, BOARD_SIZE - j - 1);
				break;
			case Board::E_DOWN:
				id = Board::Coord2Id(BOARD_SIZE - j - 1, i);
				break;
			}

			if (grids[id] > 0)
			{
				if (j != validIdCount)
					isChange = true;

				validId[validIdCount++] = id;	
			}
			allId[j] = id;
		}
		
		int resultCount = 0;
		int results[BOARD_SIZE] = { 0 };
		
		if (validIdCount > 0)
			results[0] = grids[validId[0]];

		for (int k = 1; k < validIdCount; ++k)
		{
			int v0 = grids[validId[k - 1]];
			int v1 = grids[validId[k]];

			if (v0 == v1)
			{
				results[resultCount++] = v0 + 1;
				isChange = true;
				++k;
			}
			else
			{
				results[resultCount++] = v0;
				results[resultCount] = v1;
			}
		}
		
		for (int j = 0; j < BOARD_SIZE; ++j)
		{
			grids[allId[j]] = results[j];
		}
	}

	return isChange;
}

int Board::Coord2Id(int row, int col)
{
	return row * BOARD_SIZE + col;
}

void Board::Id2Coord(int id, int &row, int &col)
{
	row = id / BOARD_SIZE;
	col = id % BOARD_SIZE;
}

///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////

Game::Game()
{
	Init();
}

void Game::Init()
{
	state = E_NORMAL;
	turn = 1;

	board.Clear();
	UpdateValidGrids();

	Generate();
	Generate();
}

bool Game::IsGameFinish()
{
	return state != E_NORMAL;
}

void Game::Print()
{
	cout << "\n  ===== Current Board =====" << endl;
	board.Print();
}

bool Game::Move(int direction)
{
	if (!board.Move((Board::Direction)direction))
		return false;

	UpdateValidGrids();

	if (validGridCount == 0)
	{
		state = E_LOSE;
		return true;
	}
	
	Generate();
	return true;
}

void Game::UpdateValidGrids()
{
	validGridCount = 0;
	for (int i = 0; i < GRID_NUM; ++i)
	{
		if (board.grids[i] == 0)
			validGrids[validGridCount++] = i;
	}
}

void Game::Generate()
{
	int id = rand() % validGridCount;

	int value = (rand() % 10 == 0) ? 2 : 1;
	board.grids[validGrids[id]] = value;

	swap(validGrids[id], validGrids[--validGridCount]);
}