#include "AI.h"

// initialize board, color and transposition table
AI::AI(Board* boardVar, std::string assetsPath)
{
	board = boardVar;
	tt = new TranspositionTable(boardVar);
	openings = Openings::loadOpenings(assetsPath);
	evaluation = Evaluation(boardVar);
}

// calculate best move in current position
Move AI::getBestMove(int timeLeft, int increment, int depthLimit, int exactTime)
{
	// only check openings if game started normally
	if (useOpenings && board->getNormalStart())
	{
		// get the current node of the opening
		std::optional<Node> gameNode = openings->getNode(board->getMoveHistory());

		if (gameNode.has_value())
		{
			// if the current position is in an opening, load a random follow-up move
			bestMove = Move::loadFromNotation(gameNode->randomMove(), board->getPiecesMB());
			return bestMove;
		}
	}

	if (depthLimit != -1)
	{
		timeLimit = 1000000;
	}
	// check if the game has time control
	else if (timeLeft != -1)
	{
		// get the expected time per move, adjust it according to minimum time limit
		timeLimit = std::max((timeLeft / 40.0 + increment) / 1000 - 0.6, 0.2);
	}
	else if (exactTime != -1)
	{
		// if exact time is given, set time limit to that
		timeLimit = exactTime / 1000.0;
	}
	else
	{
		// if no time control - use default value
		timeLimit = defaultTimeLimit;
	}

	// set the best move to an invalid one, save search start time and state of search
	bestMove = Move::getInvalidMove();
	bestEval = LOWEST_SCORE;
	searchStart = std::chrono::system_clock::now();
	searchAborted = false;

	// clear transposition table so ai doesn't miss mates in fewer moves because of previous analyzing
	tt->clear();

	// reset node and depth counter
	nodes = 0;
	int depth;

	// go through all depths until time or depth limit is reached
	for (depth = 1; !searchAborted; depth++)
	{
		std::cout << "info depth " << depth << "\n";

		// get the eval at current depth
		search(LOWEST_SCORE, HIGHEST_SCORE, depth, 0);

		// print out info about current search
		if (!searchAborted)
		{
			std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
			std::cout << std::fixed;
			std::cout << "info score " << Score::toString(bestEval) << " depth " << depth << " nodes " << nodes << " time " << (int)(diff.count() * 1000) << " nps " << (int)(nodes / diff.count()) << " pv " << bestMove.getNotation() << "\n";
		}

		// if mate was found, abort search
		if (Score::isMateScore(bestEval))
		{
			searchAborted = true;
		}

		// if depth limit is reached, abort search
		if (depth == depthLimit)
		{
			searchAborted = true;
		}
	}

	// decrease depth counter to get the accurate depth searched
	depth--;

	// if search hasn't even crossed depth 1 (because of too deep quiescence search) or is illegal because of zobrist key collisions, get the best looking move
	board->generateMoves();
	std::vector<Move> moves = board->getMoveList();
	if (bestMove == Move::getInvalidMove() || std::find(moves.begin(), moves.end(), bestMove) == moves.end())
	{
		std::vector<Move> moves = board->getMoveList();
		evaluation->orderMoves(moves, tt);
		bestMove = moves[0];
	}

	// save end time and calculate time passed
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - searchStart;

	// print out search stats
	std::cout << std::fixed;
	std::cout << "info score " << Score::toString(bestEval) << " depth " << depth << " nodes " << nodes << " time " << (int)(diff.count() * 1000) << " nps " << (int)(nodes / diff.count()) << " pv " << bestMove.getNotation() << "\n";
	
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
	evaluation->orderMoves(moves, tt);

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
	int eval = evaluation->evaluate();

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
	evaluation->orderMoves(moves, tt);

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