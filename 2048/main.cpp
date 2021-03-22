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

		if (!g.Move(move))
		{
			cout << "Invalid move!\n";
		}
		g.Print();
	}
	cin >> input;

	return 0;
}