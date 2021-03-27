#include "AI.h"

AI::AI(Board* boardVar, int aiColor)
{
	board = boardVar;
	myColor = aiColor;
	tt = TranspositionTable(boardVar);
}

// return board evaluation for AI
int AI::evaluate()
{
	int color = board->getTurnColor();

	PieceList* pieceLists = board->getPieceLists();
	int material[2] = { 0, 0 };
	for (int i = 0; i < 12; i++)
	{
		material[Piece::colorOf(i)] += pieceLists[i].getCount() * Piece::valueOf(i);
	}

	int pieceEval = material[color] - material[!color];
	
	float endgameWeight = 1 - std::min(1.0f, (material[color] + material[!color]) / 3200.0f);

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


int AI::search(int alpha, int beta, int depth, int plyFromRoot)
{
	std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
	if (diff.count() >= timeLimit)
	{
		searchAborted = true;
		return alpha;
	}

	nodes++;

	if (plyFromRoot > 0)
	{
		if (board->repeatedPosition())
		{
			return 0;
		}
	}
	 
	std::optional<int> ttEval = tt->getStoredEval(depth, plyFromRoot, alpha, beta);
	if (ttEval.has_value())
	{
		if (plyFromRoot == 0)
		{
			bestMove = tt->getStoredMove();
		}

		return *ttEval;
	}

	// evaluate board if depth limit reached
	if (depth == 0)
	{
		return quiescenceSearch(alpha, beta);
	}

	// generate moves and save them
	board->generateMoves();
	std::vector<Move> moves = board->getMoveList();
	orderMoves(moves, true);

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

	Move bestPositionMove = Move::getInvalidMove();
	NodeType nodeType = NodeType::UPPER_BOUND;

	// loop through moves
	for (Move& move : moves)
	{
		// make the move and calculate the nodes after this position with a lower depth
		board->makeMove(move);

		// get score of that move
		int eval = -search(-beta, -alpha, depth - 1, plyFromRoot + 1);
		
		// unmake move
		board->unmakeMove(move);

		if (searchAborted)
		{
			return alpha;
		}

		if (eval >= beta)
		{
			tt->storeEntry(beta, depth, move, NodeType::LOWER_BOUND, plyFromRoot);
			return beta;
		}

		if (eval > alpha)
		{
			alpha = eval;
			bestPositionMove = move;
			nodeType = NodeType::EXACT;
			
			if (plyFromRoot == 0)
			{
				bestMove = move;
			}
		}
	}

	tt->storeEntry(alpha, depth, bestPositionMove, nodeType, plyFromRoot);

	// return the number of nodes
	return alpha;
}

int AI::quiescenceSearch(int alpha, int beta)
{
	std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
	if (diff.count() >= timeLimit)
	{
		searchAborted = true;
		return alpha;
	}

	nodes++;

	int eval = evaluate();
	if (eval >= beta)
	{
		return beta;
	}
	if (alpha < eval)
	{
		alpha = eval;
	}

	// generate moves and save them
	board->generateMoves(true);
	std::vector<Move> moves = board->getMoveList();
	orderMoves(moves, false);

	// loop through moves
	for (const Move& move : moves)
	{
		// make the move and calculate the nodes after this position with a lower depth
		board->makeMove(move);

		// get score of that move
		int eval = -quiescenceSearch(-beta, -alpha);

		// unmake move
		board->unmakeMove(move);

		if (searchAborted)
		{
			return alpha;
		}

		if (eval >= beta)
		{
			return beta;
		}

		if (eval > alpha)
		{
			alpha = eval;
		}
	}

	return alpha;
}

void AI::orderMoves(std::vector<Move>& moves, bool useTT)
{
	int color = board->getTurnColor();

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

		if ((move == tt->getStoredMove()) && useTT)
		{
			move.score = 10000;
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

		size_t i;
		for (i = 0; (i < newMoves.size()) && (move.score < newMoves[i].score); i++);
		newMoves.insert(newMoves.begin() + i, move);
	}

	moves = newMoves;
}

Move AI::getBestMove()
{
	bestMove = Move::getInvalidMove();
	searchStart = std::chrono::system_clock::now();
	searchAborted = false;

	// clear transposition table so ai doesn't miss mates in fewer moves because of previous analyzing
	tt->clear();

	nodes = 0;
	int i;

	for (i = 1; !searchAborted; i++)
	{
		int eval = search(LOWEST_SCORE, HIGHEST_SCORE, i, 0);

		if (i == depthLimit)
		{
			searchAborted = true;
		}
	}
	i--;
	
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