#include "game.h"
#include <ctime>

int main()
{
	srand((unsigned)time(NULL));

	Game g;
	string input;
	int move;

	g.Print();
	while (!g.IsGameFinish())
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

		if (g.Move(move))
		{
			g.Print();
		}
		else
		{
			cout << "Invalid move!" << endl;
		}
	}

	return 0;
}