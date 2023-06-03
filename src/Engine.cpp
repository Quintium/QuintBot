#include "Engine.h"

// initialize transposition table, openings and evaluation
Engine::Engine() : openings(Openings::loadOpenings()), tt(TranspositionTable(board)), evaluation(board, tt)
{
	loadStartPosition();
}

// actions upon loading new board position
void Engine::loadStartPosition()
{
	board.loadStartPosition();
	evaluation.reloadEval();
}

void Engine::loadFromFen(std::string fen)
{
	board.loadFromFen(fen);
	evaluation.reloadEval();
}

// actions when new move is played/unplayed
void Engine::makeMove(Move move)
{
	board.makeMove(move);
	evaluation.makeMove(move);
}

void Engine::unmakeMove(Move move)
{
	board.unmakeMove(move);
	evaluation.unmakeMove(move);
}

// actions when a new game starts
void Engine::newGame()
{
	tt.clear();
}

// return principal variation as string
std::string Engine::getPrincipalVariation(int depth)
{
	// iterate through moves in the transposition table while saving moves made
	std::optional<Move> move = tt.getStoredMove(board, true);
	std::stack<Move> moveStack;
	std::string pvString = "";

	for (int i = 0; i < depth && move.has_value() && board.getState() == PLAY; i++)
	{
		pvString += " " + (*move).getNotation();
		moveStack.push(*move);
		makeMove(*move);
		move = tt.getStoredMove(board, true);
	}

	// undo changes
	while (!moveStack.empty())
	{
		unmakeMove(moveStack.top());
		moveStack.pop();
	}
	
	// cut first space if present
	if (pvString != "")
	{
		return pvString.substr(1);
	}
	
	return "";
}

// calculate best move in current position
Move Engine::getBestMove(int timeLeft, int increment, int depthLimit, int exactTime)
{
	if (useOpeningBook && board.getNormalStart())
	{
		// if the current position is in an opening, play a random follow-up move
		std::optional<Node> gameNode = openings.findNode(board.getMoveHistory());

		if (gameNode.has_value())
		{
			bestMove = Move::loadFromNotation(gameNode->randomMove(), board.getPiecesMB());
			return bestMove;
		}
	}

	if (depthLimit != -1)
	{
		timeLimit = 1000000;
	}
	else if (timeLeft != -1)
	{
		// calculate the expected time per move
		timeLimit = (timeLeft / 40.0 + increment) / 1000 - moveOverhead;
	}
	else if (exactTime != -1)
	{
		timeLimit = exactTime / 1000.0;
	}
	else
	{
		// if no time control - use default value
		timeLimit = defaultTimeLimit;
	}

	bestMove = Move::nullmove();
	bestEval = LOWEST_SCORE;
	searchStart = std::chrono::system_clock::now();
	searchAborted = false;

	nodes = 0;
	int depth;

	// go through all depths until time or depth limit is reached
	for (depth = 1; !searchAborted; depth++)
	{
		search(LOWEST_SCORE, HIGHEST_SCORE, depth, 0, false);

		// print out info about current search
		if (!searchAborted)
		{
			std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
			std::cout << std::fixed;
			std::cout << "info score " << Score::toString(bestEval) << " depth " << depth << " nodes " << nodes << " time " << (int)(diff.count() * 1000) << " nps " << (int)(nodes / diff.count()) << " pv " << getPrincipalVariation(depth) << "\n";
		}

		if (depth == depthLimit)
		{
			searchAborted = true;
		}
	}

	// decrease depth counter to cancel out the depth++ at the end of the loop
	depth--;

	// if search hasn't even crossed depth 1 or is illegal (because of bugs or zobrist key collisions), get the best looking move
	board.generateMoves();
	std::vector<Move> moves = board.getMoveList();
	if (Move::isNull(bestMove) || std::find(moves.begin(), moves.end(), bestMove) == moves.end())
	{
		std::cout << "Search error! Move found: " << bestMove.getNotation() << ". Move is chosen by move ordering.\n";
		std::vector<Move> moves = board.getMoveList();
		evaluation.orderMoves(moves);
		bestMove = moves[0];
	}

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - searchStart;

	// print out search stats
	std::cout << std::fixed;
	std::cout << "info score " << Score::toString(bestEval) << " depth " << depth << " nodes " << nodes << " time " << (int)(diff.count() * 1000) << " nps " << (int)(nodes / diff.count()) << " pv " << getPrincipalVariation(depth) << "\n";

	return bestMove;
}

// minimax search of the game tree
int Engine::search(int alpha, int beta, int depth, int plyFromRoot, bool nullMove)
{
	// if the time limit has been reached, abort search and return
	std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
	if (diff.count() >= timeLimit)
	{
		searchAborted = true;
		return alpha;
	}

	nodes++;
	 
	// mark a two-fold repetition as a draw (not completely correct)
	if (board.checkRepetition() && plyFromRoot > 1)
	{
		return DRAW_SCORE;
	}

	// check for trivial draws before generating moves
	if (board.checkDraw())
	{
		return DRAW_SCORE;
	}

	// get the stored eval in the transposition table
	std::optional<int> ttEval = tt.getStoredEval(depth, plyFromRoot, alpha, beta);
	if (ttEval.has_value())
	{
		// replace best move if it's the main search function
		if (plyFromRoot == 0)
		{
			bestMove = *tt.getStoredMove(board, true);
			bestEval = *ttEval;
		}

		return *ttEval;
	}

	// evaluate board with quiescence search if depth limit Sis reached
	if (depth == 0)
	{
		return quiescenceSearch(alpha, beta);
	}

	board.generateMoves();
	std::vector<Move> moves = board.getMoveList();
	evaluation.orderMoves(moves);

	// check if game ended, return scores based on state
	int state = board.getState();

	if ((state == WHITE_WIN) || (state == BLACK_WIN))
	{
		return Score::getNegativeMate(plyFromRoot);
	}
	else if (state == DRAW)
	{
		return DRAW_SCORE;
	}

	// evaluate null move for null move pruning
	if (!board.getCheck() && !nullMove && depth > 3)
	{
		makeMove(Move::nullmove());
		int nullEval = -search(-beta, -alpha, depth - 4, plyFromRoot + 1, true);
		unmakeMove(Move::nullmove());

		if (nullEval >= beta)
		{
			return beta;
		}
	}

	Move bestPositionMove = Move::nullmove();
	int nodeType = UPPER_BOUND_NODE;

	// loop through all legal moves
	for (Move& move : moves)
	{
		// get score of given move
		makeMove(move);
		int eval = -search(-beta, -alpha, depth - 1, plyFromRoot + 1, nullMove);
		unmakeMove(move);

		// beta-cutoff (move is too good to be allowed by the opponent)
		if (eval >= beta)
		{
			tt.storeEntry(beta, depth, move, LOWER_BOUND_NODE, plyFromRoot);
			return beta;
		}

		// new best move for the position is found
		if (eval > alpha)
		{
			alpha = eval;
			bestPositionMove = move;
			nodeType = EXACT_NODE;
			
			// save move as best move if it's the main search function
			if (plyFromRoot == 0)
			{
				bestMove = move;
				bestEval = eval;
			}
		}
	}

	tt.storeEntry(alpha, depth, bestPositionMove, nodeType, plyFromRoot);
	return alpha;
}

// evaluate all non-quiet/messy positions
int Engine::quiescenceSearch(int alpha, int beta)
{
	// if the time limit has been reached, abort search and return
	std::chrono::duration<double> diff = std::chrono::system_clock::now() - searchStart;
	if (diff.count() >= timeLimit)
	{
		searchAborted = true;
		return alpha;
	}

	nodes++;

	// check for trivial draws
	if (board.checkDraw())
	{
		return DRAW_SCORE;
	}

	int eval = evaluation.evaluate();

	// static eval is used as a lower-bound of the score, assuming there is a better move than doing nothing (null move observation)
	if (eval >= beta)
	{
		return beta;
	}
	if (eval > alpha)
	{
		alpha = eval;
	}

	board.generateMoves(true);
	std::vector<Move> moves = board.getMoveList();
	evaluation.orderMoves(moves);

	// loop through all legal captures
	for (const Move& move : moves)
	{
		// get score of given move
		makeMove(move);
		int eval = -quiescenceSearch(-beta, -alpha);
		unmakeMove(move);

		// beta-cutoff (move is too good to be allowed by the opponent)
		if (eval >= beta)
		{
			return beta;
		}

		// new best move for the position is found
		if (eval > alpha)
		{
			alpha = eval;
		}
	}

	return alpha;
}

// evaluate current position
int Engine::evaluate()
{
	return evaluation.evaluate();
}

// option whether to use own opening book
void Engine::setOwnBook(bool useOwnBook)
{
	useOpeningBook = useOwnBook;
}

// option how large the hash table should be in MB
void Engine::setHash(int sizeMB)
{
	tt.setSizeMB(sizeMB);
}

// option how long the move overhead should be in ms
void Engine::setMoveOverhead(int moveOverheadMs)
{
	moveOverhead = (float)moveOverheadMs / 1000;
}

Board& Engine::getBoard()
{
	return board;
}