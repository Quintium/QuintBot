#include "AI.h"

AI::AI(Board* boardVar, int aiColor)
{
	board = boardVar;
	myColor = aiColor;
}

int AI::search(int color, int depth, int maxDepth)
{
	// evaluate board if depth limit reached
	if (depth == 0)
	{
		return board->evaluate(color);
	}

	// generate moves and save them
	board->generateMoves();
	std::vector<Move> currentMoveList = *board->getMoveList();

	// check if there are no moves left
	if (currentMoveList.size() == 0)
	{
		// return scores for checkmate or stalemate
		if (board->getCheck())
		{
			return -100000 / (maxDepth - depth);
		}
		else
		{
			return 0;
		}
	}

	// keep track of best score yet
	int bestScore = -1000000;

	// loop through moves
	for (Move move : currentMoveList)
	{
		// make the move and calculate the nodes after this position with a lower depth
		board->makeMove(&move);

		// get score of that move
		int score = -search(!color, depth - 1, maxDepth);

		if (score > bestScore)
		{
			bestScore = score;
			
			if (depth == maxDepth)
			{
				bestMove.load(move);
			}
		}
		
		// unmake move
		board->unmakeMove(&move);
	}

	// return the number of nodes
	return bestScore;
}

Move AI::getBestMove(int depth)
{
	int score = search(myColor, depth, depth);
	return bestMove;
}