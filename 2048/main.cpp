#include "game.h"
#include "mcts.h"
#include <ctime>

int main()
{
	srand((unsigned)time(NULL));

	MCTS ai;
	bool useAI = true;

	Game g;
	string input;
	int move;

	array<char, GRID_NUM> grids = { 1, 7, 10, 9,
								    4,  5, 6, 1,
								    1,  3, 2, 5,
								    0,  3, 1, 3 };
	//((GameBase*)&g)->SetDebugBoard(grids);

	g.Print();
	while (!g.IsGameFinish())
	{
		if (useAI)
		{
			move = ai.Search(&g);
		}
		else
		{
			cout << "Enter your move: ";
			cin >> input;

			if (input == "w")
				move = Board::E_UP;
			else if (input == "s")
				move = Board::E_DOWN;
			else if (input == "a")
				move = Board::E_LEFT;
			else if (input == "d")
				move = Board::E_RIGHT;
			else
				move = -1;
		}

		if (g.Move(move))
		{
			g.Print();
		}
		else
		{
			cout << "Invalid move!" << endl;
		}
	}
	cin >> input;

	return 0;
}