#include "AI.h"

AI::AI(Board* boardVar, int aiColor)
{
	board = boardVar;
	myColor = aiColor;
}

int AI::search(int color, int alpha, int beta, int depth, int maxDepth)
{
	nodes++;

	// evaluate board if depth limit reached
	if (depth == 0)
	{
		return board->evaluate(color);
	}

	// generate moves and save them
	board->generateMoves();
	std::vector<Move> currentMoveList = orderMoves(*board->getMoveList(), color);

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

	// loop through moves
	for (Move move : currentMoveList)
	{
		// make the move and calculate the nodes after this position with a lower depth
		board->makeMove(move);

		// get score of that move
		int score = -search(!color, -beta, -alpha, depth - 1, maxDepth);
		
		// unmake move
		board->unmakeMove(move);

		if (score > alpha)
		{
			alpha = score;

			if (score >= beta)
			{
				return beta;
			}

			if (depth == maxDepth)
			{
				bestMove.load(move);
			}
		}
	}

	// return the number of nodes
	return alpha;
}

std::vector<Move> AI::orderMoves(std::vector<Move> moves, int color)
{
	U64 pawnAttacks = BB::pawnAnyAttacks(board->getPiecesBB()[PAWN + !color], !color);

	std::vector<Move> newMoves;
	for (Move move : moves)
	{
		move.score = 0;

		if (move.cPiece != EMPTY)
		{
			move.score = 10 * Piece::valueOf(move.cPiece) - Piece::valueOf(move.piece);
		}

		if (move.promotion != EMPTY)
		{
			move.score += Piece::valueOf(move.promotion);
		}

		if ((pawnAttacks & (U64(1) << move.to)) > 0)
		{
			move.score -= Piece::valueOf(move.piece);
		}

		int i = 0;
		for (; (i < newMoves.size()) && (move.score < newMoves[i].score); i++);
		newMoves.insert(newMoves.begin() + i, move);
	}

	return newMoves;
}

Move AI::getBestMove(int depth)
{
	auto start = std::chrono::system_clock::now();

	nodes = 0;
	int score = search(myColor, -1000000, 1000000, depth, depth);

	// save end time and calculate time Passed
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	double timePassed = diff.count();
	std::cout << "AI needed time: " << timePassed << "\n";
	std::cout << "Evaluated nodes: " << nodes << "\n";

	return bestMove;
}