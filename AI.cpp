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
	
	float endgameWeight = 1 - std::min(1.0f, (material[color] + material[!color]) / 1600.0f);

	int pieceSquareEval = 0;
	for (int i = 0; i < 12; i++)
	{
		PieceList pieceList = pieceLists[i];

		for (int j = 0; j < pieceList.getCount(); j++)
		{
			pieceSquareEval += pieceSquareTables.getScore(i, pieceList[j], endgameWeight) * (Piece::colorOf(i) == color ? 1 : -1);
		}
	}

	int whiteKing = pieceLists[WHITE + KING][0];
	int blackKing = pieceLists[BLACK + KING][0];
	int distance = 14 - (std::abs(Square::fileOf(whiteKing) - Square::fileOf(blackKing)) + std::abs(Square::rankOf(whiteKing) - Square::rankOf(blackKing)));
	int mopUpEval = 0;

	if (pieceEval > 0)
	{
		if (material[color] > (material[!color] + 200))
		{
			mopUpEval = (int)(distance * endgameWeight * 4);
		}
	}
	else
	{
		if (material[!color] > (material[color] + 200))
		{
			mopUpEval = (int)(distance * endgameWeight * -4);
		}
	}
	
	return pieceEval + pieceSquareEval + mopUpEval;
}


int AI::search(int color, int alpha, int beta, int depth, int plyFromRoot)
{
	std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
	if (diff.count() >= timeLimit)
	{
		searchAborted = true;
		return alpha;
	}

	nodes++;

	// evaluate board if depth limit reached
	if (depth == 0)
	{
		return quiescenseSearch(color, alpha, beta, 3);
	}

	// generate moves and save them
	board->generateMoves();
	std::vector<Move> currentMoveList = orderMoves(board->getMoveList(), color, plyFromRoot == 0);

	// check if game ended
	int state = board->getState();

	if ((state == WHITE_WIN) || (state == BLACK_WIN))
	{
		return -MATE_SCORE + plyFromRoot;
	}
	else if (state == DRAW)
	{
		return DRAW_SCORE;
	}

	// loop through moves
	for (const Move& move : currentMoveList)
	{
		// make the move and calculate the nodes after this position with a lower depth
		board->makeMove(move);

		// get score of that move
		int score = -search(!color, -beta, -alpha, depth - 1, plyFromRoot + 1);
		
		// unmake move
		board->unmakeMove(move);

		if (score > alpha)
		{
			alpha = score;

			if (score >= beta)
			{
				return beta;
			}

			if (plyFromRoot == 0)
			{
				bestMove.load(move);
			}
		}

		if (searchAborted)
		{
			break;
		}
	}

	// return the number of nodes
	return alpha;
}

int AI::quiescenseSearch(int color, int alpha, int beta, int depth)
{
	std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
	if (diff.count() >= timeLimit)
	{
		searchAborted = true;
		return alpha;
	}

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
	std::vector<Move> currentMoveList = orderMoves(board->getMoveList(), color, false);

	// loop through moves
	for (const Move& move : currentMoveList)
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

		if (searchAborted)
		{
			break;
		}
	}

	return alpha;
}

std::vector<Move> AI::orderMoves(std::vector<Move> moves, int color, bool useBestMove)
{
	U64 pawnAttacks = BB::pawnAnyAttacks(board->getPiecesBB()[PAWN + !color], !color);

	std::vector<Move> newMoves;
	for (Move& move : moves)
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

		if ((move == bestMove) && useBestMove)
		{
			move.score = 10000;
		}
		
		size_t i = 0;
		for (; (i < newMoves.size()) && (move.score < newMoves[i].score); i++);
		newMoves.insert(newMoves.begin() + i, move);
	}

	return newMoves;
}

Move AI::getBestMove()
{
	bestMove = { -1, -1, EMPTY, EMPTY, false, false, EMPTY, 0 };
	searchStart = std::chrono::system_clock::now();
	searchAborted = false;

	nodes = 0;
	int i;

	for (i = 0; !searchAborted; i++)
	{
		int score = search(myColor, LOWEST_SCORE, HIGHEST_SCORE, i, 0);
		if (score > MATE_SCORE - 1000)
		{
			searchAborted = true;
		}
	}
	
	// save end time and calculate time Passed
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - searchStart;
	double timePassed = diff.count();
	std::cout << "AI needed time: " << timePassed << "\n";
	std::cout << "Evaluated nodes: " << nodes << "\n";
	std::cout << "Depth searched: " << i << "\n";
	std::cout << "\n";

	return bestMove;
}