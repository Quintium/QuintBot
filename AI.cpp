#include "AI.h"

AI::AI(Board* boardVar, int aiColor)
{
	board = boardVar;
	myColor = aiColor;
}

// return board evaluation for AI
int AI::evaluate(int color)
{
	PieceList* pieceLists = board->getPieceLists();
	int material[2] = { 0, 0 };
	for (int i = 0; i < 12; i++)
	{
		material[Piece::colorOf(i)] += pieceLists[i].getCount() * Piece::valueOf(i);
	}

	int pieceEval = material[color] - material[!color];
	
	bool endgame = material[color] + material[!color] < 2500;

	int pieceSquareEval = 0;
	for (int i = 0; i < 12; i++)
	{
		PieceList pieceList = pieceLists[i];

		for (int j = 0; j < pieceList.getCount(); j++)
		{
			pieceSquareEval += pieceSquareTables.getScore(i, pieceList[j], endgame) * (Piece::colorOf(i) == color ? 1 : -1);
		}
	}

	//std::cout << "Piece square eval: " << pieceSquareEval << "\n";
	return pieceEval + pieceSquareEval;
}


int AI::search(int color, int alpha, int beta, int depth, int maxDepth)
{
	nodes++;

	// evaluate board if depth limit reached
	if (depth == 0)
	{
		return quiescenseSearch(color, alpha, beta, 3);
	}

	// generate moves and save them
	board->generateMoves();
	std::vector<Move> currentMoveList = orderMoves(board->getMoveList(), color);

	// check if there are no moves left
	if (currentMoveList.size() == 0)
	{
		// return scores for checkmate or stalemate
		if (board->getCheck())
		{
			return -100000 + (maxDepth - depth);
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

int AI::quiescenseSearch(int color, int alpha, int beta, int depth)
{
	nodes++;

	if (depth == 0)
	{
		return evaluate(color);
	}

	int standPat = evaluate(color);
	if (standPat >= beta)
	{
		return beta;
	}
	if (alpha < standPat)
	{
		alpha = standPat;
	}

	// generate moves and save them
	board->generateMoves(true);
	std::vector<Move> currentMoveList = orderMoves(board->getMoveList(), color);

	// loop through moves
	for (Move move : currentMoveList)
	{
		// make the move and calculate the nodes after this position with a lower depth
		board->makeMove(move);

		// get score of that move
		int score = -quiescenseSearch(!color, -beta, -alpha, depth - 1);

		// unmake move
		board->unmakeMove(move);

		if (score > alpha)
		{
			alpha = score;

			if (score >= beta)
			{
				return beta;
			}
		}
	}

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
			move.score += 10 * Piece::valueOf(move.cPiece) - Piece::valueOf(move.piece);
		}

		if (move.promotion != EMPTY)
		{
			move.score += Piece::valueOf(move.promotion);
		}

		if ((pawnAttacks & (U64(1) << move.to)) > 0)
		{
			move.score -= Piece::valueOf(move.piece);
		}

		/*// best first instead of full sort
		if ((newMoves.size() > 0) && (move.score > newMoves[0].score))
		{
			newMoves.insert(newMoves.begin(), move);
		}
		else
		{
			newMoves.push_back(move);
		}*/

		
		int i = 0;
		for (; (i < newMoves.size()) && (move.score < newMoves[i].score); i++);
		newMoves.insert(newMoves.begin() + i, move);
	}

	return newMoves;
}

Move AI::getBestMove()
{
	auto start = std::chrono::system_clock::now();

	nodes = 0;
	int score = search(myColor, -1000000, 1000000, 1, 1);

	// save end time and calculate time Passed
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	double timePassed = diff.count();
	std::cout << "AI needed time: " << timePassed << "\n";
	std::cout << "Evaluated nodes: " << nodes << "\n";
	std::cout << "\n";

	return bestMove;
}