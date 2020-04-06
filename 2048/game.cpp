#include "game.h"
#include <cstdlib>

#define max(a, b) ((a > b) ? a : b)

#define OUTPUT_LINE_DICT 1

bool Board::isLineDictReady = false;
array<short, LINE_DICT_SIZE> Board::lineDict;

Board::Board()
{
	if (!Board::isLineDictReady)
		Board::InitLineDict();

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

// line grid order: 3, 2, 1, 0
void Board::InitLineDict()
{
	FILE *fp;
	if (OUTPUT_LINE_DICT)
		fopen_s(&fp, "line_dict.txt", "w");

	int validIdCount, resultCount;
	array<char, BOARD_SIZE> line, result, validId, allId;

	for (int i = 0; i < LINE_DICT_SIZE; ++i)
	{
		// init line
		Board::Key2Line(i, line);

		// collect valid grids
		validIdCount = 0;
		validId.fill(0);

		for (int j = 0; j < BOARD_SIZE; ++j)
		{
			if (line[j] > 0)
			{
				validId[validIdCount++] = j;
			}
			allId[j] = j;
		}

		// move & combine
		resultCount = 0;
		result.fill(0);

		if (validIdCount > 0)
			result[0] = line[validId[0]];

		int v0 = -1, v1;

		for (int j = 0; j < validIdCount; ++j)
		{
			v1 = line[validId[j]];

			if (v0 == -1)
			{
				result[resultCount] = v1;
				v0 = v1;
			}
			else if (v0 == v1)
			{
				result[resultCount++] = v0 + 1;
				v0 = -1;
			}
			else
			{
				result[resultCount++] = v0;
				result[resultCount] = v1;
				v0 = v1;
			}
		}

		// calc line result
		int value = Board::Line2Key(result);
		lineDict[i] = value;

		if (OUTPUT_LINE_DICT && value != i)
		{
			fprintf(fp, "%6d: ", i);
			for (int j = 0; j < BOARD_SIZE; ++j)
			{
				int num = line[j] > 0 ? (1 << line[j]) : 0;
				fprintf(fp, "%6d", num);
			}
			fprintf(fp, " | ");

			for (int j = 0; j < BOARD_SIZE; ++j)
			{
				int num = result[j] > 0 ? (1 << result[j]) : 0;
				fprintf(fp, "%6d", num);
			}
			fprintf(fp, "\n");
		}
	}

	if (OUTPUT_LINE_DICT)
		fclose(fp);

	Board::isLineDictReady = true;
}

int Board::Line2Key(const array<char, BOARD_SIZE> &line)
{
	int key = 0;
	for (int i = BOARD_SIZE - 1; i >= 0; --i)
	{
		key = (key << 4) + line[i];
	}
	return key;
}

void Board::Key2Line(int key, array<char, BOARD_SIZE> &line)
{
	for (int i = 0; i < BOARD_SIZE; ++i)
	{
		line[i] = key & 0xf;
		key = key >> 4;
	}
}

bool Board::Move(Direction d)
{
	bool isChange = false;
	array<char, BOARD_SIZE> line, allId;

	for (int i = 0; i < BOARD_SIZE; ++i)
	{
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
			allId[j] = id;
			line[j] = grids[id];
		}
		int key = Board::Line2Key(line);
		int value = Board::lineDict[key];

		if (value != key)
		{
			isChange = true;
			Board::Key2Line(value, line);

			for (int j = 0; j < BOARD_SIZE; ++j)
			{
				grids[allId[j]] = line[j];
			}
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

void GameBase::Init()
{
	state = E_NORMAL;
	turn = 1;

	board.Clear();
	UpdateValidGrids();

	Generate();
	Generate();
}

bool GameBase::Move(int move)
{
	if (turn % 2 == 1)
	{
		if (!board.Move((Board::Direction)move))
			return false;

		UpdateValidGrids();

		if (validGridCount == 0)
		{
			state = E_LOSE;
		}
	}
	else
	{
		Generate(move);
	}

	++turn;
	return true;
}

void GameBase::UpdateValidGrids()
{
	validGridCount = 0;
	for (int i = 0; i < GRID_NUM; ++i)
	{
		if (board.grids[i] == 0)
			validGrids[validGridCount++] = i;
	}
}

void GameBase::Generate(int id)
{
	if (id == -1)
		id = rand() % validGridCount;

	int value = (rand() % 10 == 0) ? 2 : 1;
	board.grids[validGrids[id]] = value;

	swap(validGrids[id], validGrids[--validGridCount]);
}

///////////////////////////////////////////////////////////////////

Game::Game()
{
	Init();
}

bool Game::Move(int direction)
{
	if (!GameBase::Move(direction))
		return false;

	GameBase::Move();
	return true;
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