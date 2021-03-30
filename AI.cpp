#include "AI.h"

// initialize board, color and transposition table
AI::AI(Board* boardVar)
{
	board = boardVar;
	tt = TranspositionTable(boardVar);
}

// return board evaluation for AI
int AI::evaluate()
{
	// save turn color
	int color = board->getTurnColor();

	// count material of both colors
	PieceList* pieceLists = board->getPieceLists();
	int material[2] = { 0, 0 };
	for (int i = 0; i < 12; i++)
	{
		material[Piece::colorOf(i)] += pieceLists[i].getCount() * Piece::valueOf(i);
	}

	// save piece advantage
	int pieceEval = material[color] - material[!color];
	
	// calculate endgame weight, endgame weight starts rising after both sides have less material than two queens, two rooks and four pawns
	float endgameWeight = 1 - std::min(1.0f, (material[color] + material[!color]) / 3200.0f);

	// add all piece square scores of ally pieces and substract scores of enemy pieces
	int pieceSquareEval = 0;
	for (int i = 0; i < 12; i++)
	{
		PieceList pieceList = pieceLists[i];

		for (int j = 0; j < pieceList.getCount(); j++)
		{
			pieceSquareEval += pieceSquareTables.getScore(i, pieceList[j], endgameWeight) * (Piece::colorOf(i) == color ? 1 : -1);
		}
	}

	// get squares of white and black king, calculate their distance
	int whiteKing = pieceLists[WHITE + KING][0];
	int blackKing = pieceLists[BLACK + KING][0];
	int closeness = 14 - (std::abs(Square::fileOf(whiteKing) - Square::fileOf(blackKing)) + std::abs(Square::rankOf(whiteKing) - Square::rankOf(blackKing)));
	int mopUpEval = 0;

	// if the current color has a big lead, award close kings
	if (pieceEval > 100)
	{
		mopUpEval = (int)(closeness * endgameWeight * 4);
	}
	// if the other color has a big lead, award far kings
	else if (pieceEval < -100)
	{
		mopUpEval = (int)(closeness * endgameWeight * 4);
	}
	
	// return sum of different evals
	return pieceEval + pieceSquareEval + mopUpEval;
}

// order list of moves from best to worst
void AI::orderMoves(std::vector<Move>& moves, bool useTT)
{
	// save turn color
	int color = board->getTurnColor();

	// create map of enemy pawn attacks
	U64 pawnAttacks = BB::pawnAnyAttacks(board->getPiecesBB()[PAWN + !color], !color);

	// loop through all moves
	std::vector<Move> newMoves;
	for (Move& move : moves)
	{
		move.score = 0;

		// if there's a capture award more valuable captured piece and less valuable moved piece
		if (move.cPiece != EMPTY)
		{
			move.score += 10 * Piece::valueOf(move.cPiece) - Piece::valueOf(move.piece);
		}

		// award promotion with value of promotion piece
		if (move.promotion != EMPTY)
		{
			move.score += Piece::valueOf(move.promotion);
		}

		// if enemy pawn could take piece, penalize a more valuable piece
		if ((pawnAttacks & (U64(1) << move.to)) > 0)
		{
			move.score -= Piece::valueOf(move.piece);
		}

		// if this was the best move in the transposition table with a lower depth, examine it first
		if ((move == tt->getStoredMove()) && useTT)
		{
			move.score = 10000;
		}

		// insert it in the right place in the sorted array
		size_t i;
		for (i = 0; (i < newMoves.size()) && (move.score < newMoves[i].score); i++);
		newMoves.insert(newMoves.begin() + i, move);
	}

	// replace the original array with the sorted array
	moves = newMoves;
}

// calculate best move in current position
Move AI::getBestMove()
{
	// set the best move to an invalid one, save search start time and state of search
	bestMove = Move::getInvalidMove();
	bestEval = LOWEST_SCORE;
	searchStart = std::chrono::system_clock::now();
	searchAborted = false;

	// clear transposition table so ai doesn't miss mates in fewer moves because of previous analyzing
	tt->clear();

	// reset node and depth counter
	nodes = 0;
	int i;

	// go through all depths until time or depth limit is reached
	for (i = 1; !searchAborted; i++)
	{
		// get the eval at current depth
		search(LOWEST_SCORE, HIGHEST_SCORE, i, 0);

		// if depth limit is reached, abort search
		if (i == depthLimit)
		{
			searchAborted = true;
		}
	}

	// decrease depth counter to get the accurate depth searched
	i--;

	// save end time and calculate time passed
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - searchStart;
	double timePassed = diff.count();

	// print out search stats
	std::cout << std::fixed;
	std::cout << "info time " << (int)(timePassed * 1000) << "\n";
	std::cout << "info nodes " << nodes << "\n";
	std::cout << "info depth " << i << "\n";
	std::cout << "info nps " << (int)(nodes / timePassed) << "\n";

	// check if it's a mate in x
	if (Score::isMateScore(bestEval))
	{
		// calculate number of moves until mate
		int mateIn = (int)std::ceil(std::abs((std::abs(bestEval) - MATE_SCORE) / 2.0));

		// print out mate information
		if (bestEval > 0)
		{
			std::cout << "info score mate " << mateIn << "\n";
		}
		else
		{
			std::cout << "info score mate -" << -mateIn << "\n";
		}
	}
	else
	{ 
		// print out search score in centipawns
		std::cout << "info score cp " << bestEval << "\n";
	}

	// return best move found
	return bestMove;
}

// search/minimax function
int AI::search(int alpha, int beta, int depth, int plyFromRoot)
{
	// if the time limit has been reached, abort search and return
	std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
	if (diff.count() >= timeLimit)
	{
		searchAborted = true;
		return alpha;
	}

	// increase nodes searched
	nodes++;

	// return draw if there's a repetition
	if (plyFromRoot > 0)
	{
		if (board->repeatedPosition())
		{
			return DRAW_SCORE;
		}
	}
	 
	// get the stored eval in the transposition table
	std::optional<int> ttEval = tt->getStoredEval(depth, plyFromRoot, alpha, beta);
	if (ttEval.has_value())
	{
		// replace best move if it's the main search function
		if (plyFromRoot == 0)
		{
			bestMove = tt->getStoredMove();
			bestEval = *ttEval;
		}

		// return evaluation
		return *ttEval;
	}

	// evaluate board with quiescence search if depth limit reached
	if (depth == 0)
	{
		return quiescenceSearch(alpha, beta);
	}

	// generate moves, save and order them
	board->generateMoves();
	std::vector<Move> moves = board->getMoveList();
	orderMoves(moves, true);

	// check if game ended
	int state = board->getState();

	// return scores based on state
	if ((state == WHITE_WIN) || (state == BLACK_WIN))
	{
		return -MATE_SCORE + plyFromRoot;
	}
	else if (state == DRAW)
	{
		return DRAW_SCORE;
	}

	// save the best move in this position and the node type of this node
	Move bestPositionMove = Move::getInvalidMove();
	NodeType nodeType = NodeType::UPPER_BOUND;

	// loop through moves
	for (Move& move : moves)
	{
		// make the move
		board->makeMove(move);

		// get score of that move
		int eval = -search(-beta, -alpha, depth - 1, plyFromRoot + 1);
		
		// unmake the move
		board->unmakeMove(move);

		// abort search if it's been aborted in the called function
		if (searchAborted)
		{
			return alpha;
		}

		// if eval is greater than beta -> save a lower-bound entry and cause a beta-cutoff
		if (eval >= beta)
		{
			tt->storeEntry(beta, depth, move, NodeType::LOWER_BOUND, plyFromRoot);
			return beta;
		}

		// if eval is greater than alpha -> replace score/alpha with eval, save move and set node type to exact
		if (eval > alpha)
		{
			alpha = eval;
			bestPositionMove = move;
			nodeType = NodeType::EXACT;
			
			// save move as best move if it's the main search function
			if (plyFromRoot == 0)
			{
				bestMove = move;
				bestEval = eval;
			}
		}
	}

	// store the eval of this position
	tt->storeEntry(alpha, depth, bestPositionMove, nodeType, plyFromRoot);

	// return the score
	return alpha;
}

// evaluate all non-quiet/messy positions
int AI::quiescenceSearch(int alpha, int beta)
{
	// if the time limit has been reached, abort search and return
	std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
	if (diff.count() >= timeLimit)
	{
		searchAborted = true;
		return alpha;
	}

	// increase nodes searched
	nodes++;

	// calculate eval of current board position
	int eval = evaluate();

	// if eval is greater than beta, cause beta-cutoff
	if (eval >= beta)
	{
		return beta;
	}

	// if eval is greater than alpha, set alpha to eval
	if (eval > alpha)
	{
		alpha = eval;
	}

	// generate moves, save and order them
	board->generateMoves(true);
	std::vector<Move> moves = board->getMoveList();
	orderMoves(moves, false);

	// loop through moves
	for (const Move& move : moves)
	{
		// make the move
		board->makeMove(move);

		// get score of that move
		int eval = -quiescenceSearch(-beta, -alpha);

		// unmake the move
		board->unmakeMove(move);

		// abort search if it has been aborted in previous function
		if (searchAborted)
		{
			return alpha;
		}

		// if eval is greater than beta -> cause beta cutoff
		if (eval >= beta)
		{
			return beta;
		}

		// if eval is greater than current maximum eval -> increase the score
		if (eval > alpha)
		{
			alpha = eval;
		}
	}

	// return score
	return alpha;
}