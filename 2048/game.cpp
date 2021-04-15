#include "game.h"
#include <cmath>
#include <cstdlib>

#define USE_LINE_DICT 1
#define OUTPUT_LINE_DICT 0

bool Board::isLineDictReady = false;
array<short, LINE_DICT_SIZE> Board::lineDict;

Board::Board()
{
	if (USE_LINE_DICT && !Board::isLineDictReady)
		Board::InitLineDict();

	Clear();
}

void Board::Clear()
{
	maxValue = 0;
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

#if USE_LINE_DICT

__declspec(noinline)
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
				maxValue = max(maxValue, (int)line[j]);
			}
		}

	}

	return isChange;
}

#else

__declspec(noinline)
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
				maxValue = max(maxValue, v0 + 1);
				isChange = true;
				++k;

				if (k < validIdCount)
					results[resultCount] = grids[validId[k]];
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
#endif

bool Board::Check(Direction d)
{
	for (int i = 0; i < BOARD_SIZE; ++i)
	{
		int lastValue = -1;
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

			int value = grids[id];
			if (value > 0)
			{
				if (lastValue == 0 || value == lastValue)
					return true;

			}
			lastValue = value;
		}
	}
	return false;
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


///////////////////////////////////////////////////////////////////

GameBase::GameBase()
{
	Init();
}

void GameBase::Init()
{
	state = E_NORMAL;
	turn = 1;

	board.Clear();
	UpdateValidGrids();

	RandomGenerate();
	RandomGenerate();

	turn = 1; // reset turn to 1
}

bool GameBase::IsGameFinish()
{
	return state != E_NORMAL;
}

int GameBase::GetSide()
{
	return (turn % 2 == 1) ? Board::E_PLAYER : Board::E_SYSTEM;
}

int GameBase::GetNextMove()
{
	if (GetSide() == Board::E_PLAYER)
	{
		// naive stategy
		static int direction[][4] =
		{
			{ Board::E_LEFT, Board::E_UP, Board::E_RIGHT, Board::E_DOWN },
			{ Board::E_UP, Board::E_LEFT, Board::E_RIGHT, Board::E_DOWN },
			{ Board::E_RIGHT, Board::E_UP, Board::E_LEFT, Board::E_DOWN },
			{ Board::E_UP, Board::E_RIGHT, Board::E_LEFT, Board::E_DOWN },
			{ Board::E_RIGHT, Board::E_LEFT, Board::E_UP, Board::E_DOWN },
			{ Board::E_LEFT, Board::E_RIGHT, Board::E_UP, Board::E_DOWN }
		};

		int i = rand() % 6;
		int j = 0;
		while (!board.Check((Board::Direction)direction[i][j]))
		{
			++j;
		}
		return direction[i][j];
	}
	else // E_SYSTEM
	{
		int rnd = rand();
		int id = (rnd >> 4) % validGridCount;

		int ratio = min(validGridCount + 3, 10);
		int value = (rnd % ratio == 0) ? 2 : 1;
		int action = GameBase::EncodeAction(id, value);
		return action;
	}
}

float GameBase::CalcFastStopScore()
{
	float score1 = CalcFinishScore(1.f);
	float score2 = clamp(validGridCount, 0, 8) / 8.f;
	float score = score1 + score2 * 0.2f;
	return score;
}

float GameBase::CalcFinishScore(float ratio)
{
	float score = ratio * 0.8f;
	return score;
}

void GameBase::GetValidActions(array<uint8_t, VALID_ACTION_MAX> &result, int &count)
{
	count = 0;
	if (GetSide() == Board::E_PLAYER)
	{
		for (int d = 0; d < Board::E_DIRECTION_MAX; ++d)
		{
			if (board.Check((Board::Direction)d))
				result[count++] = d;
		}
	}
	else // E_SYSTEM
	{
		for (int i = 0; i < validGridCount; ++i)
		{
			result[count++] = GameBase::EncodeAction(validGrids[i], 1);
			result[count++] = GameBase::EncodeAction(validGrids[i], 2);
		}
	}
}

string GameBase::LastAction2Str()
{
	if (GetSide() == Board::E_PLAYER) // last action is system move
	{
		int id, value, row, col;
		GameBase::DecodeAction(lastMove, id, value);
		Board::Id2Coord(id, row, col);
		string result(1, col + 'A');
		result += (row + '1');
		result += (value == 1) ? "|2" : "|4";
		return result;
	}
	else // last action is player move
	{
		return Game::Move2Str(lastMove);
	}
}

void GameBase::SetDebugBoard(const array<char, GRID_NUM> &grids)
{
	int total = 0;
	for (int i = 0; i < GRID_NUM; ++i)
	{
		board.grids[i] = grids[i];
		total += pow(2, grids[i]);
	}
	UpdateValidGrids();
	total = int((float)total / 2.2);
	turn = (total % 2 == 1) ? total : total + 1;
}

void GameBase::Move(int action)
{
	if (GetSide() == Board::E_PLAYER)
	{
		PlayerMove(action);
	}
	else // E_SYSTEM
	{
		int id, value;
		GameBase::DecodeAction(action, id, value);
		Generate(id, value);
	}
	lastMove = action;
}

bool GameBase::PlayerMove(int direction)
{
	if (!board.Move((Board::Direction)direction))
		return false;

	lastMove = direction;
	UpdateValidGrids();

	if (board.maxValue >= WIN_CONDITION)
	{
		state = E_WIN;
		return true;
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

void GameBase::RandomGenerate()
{
	int rnd = rand();
	int id = (rnd >> 4) % validGridCount;
	int value = (rnd % 10 == 0) ? 2 : 1;
	Generate(id, value);
}

void GameBase::Generate(int id, int value)
{
	board.grids[validGrids[id]] = value;

	swap(validGrids[id], validGrids[--validGridCount]);

	if (validGridCount == 0)
		CheckLoseCondition();

	++turn;
}

void GameBase::CheckLoseCondition()
{
	bool hasMove = false;
	for (int d = 0; d < Board::E_DIRECTION_MAX; ++d)
	{
		if (board.Check((Board::Direction)d))
		{
			hasMove = true;
			break;
		}
	}

	if (!hasMove)
		state = GameBase::E_LOSE;
}

int GameBase::EncodeAction(int id, int value)
{
	return (id << 4) | value;
}

void GameBase::DecodeAction(int action, int &id, int &value)
{
	id = action >> 4;
	value = action & 0xf;
}

void Game::Print()
{
	cout << "\n  ===== Current Board =====" << endl;
	board.Print();

	if (state == E_WIN)
		cout << "Congratulations! You Win!" << endl;

	if (state == E_LOSE)
		cout << "Sorry! You Lose!" << endl;
}

bool Game::Move(int direction)
{
	if (direction < 0 || direction >= Board::E_DIRECTION_MAX)
		return false;

	if (!GameBase::PlayerMove(direction))
		return false;

	if (!IsGameFinish())
		GameBase::RandomGenerate();

	return true;
}

string Game::Move2Str(int direction)
{
	switch (direction)
	{
	case Board::E_UP:
		return "up";
	case Board::E_LEFT:
		return "left";
	case Board::E_RIGHT:
		return "right";
	case Board::E_DOWN:
		return "down";
	default:
		return "";
	}
}