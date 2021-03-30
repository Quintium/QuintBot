#pragma once
#include "Board.h"
#include "PieceSquareTables.h"
#include "TranspositionTable.h"

// AI class
class AI
{
	// board variable
	Board* board = nullptr;

	// variables for time control
	std::chrono::time_point<std::chrono::system_clock> searchStart;
	bool searchAborted = false;

	// ai limits
	const double timeLimit = 5;
	const int depthLimit = 100;

	// saving best move and eval found
	Move bestMove = Move::getInvalidMove();
	int bestEval = LOWEST_SCORE;

	// total nodes searched for debugging
	int nodes = 0;

	// values for squares of pieces
	PieceSquareTables pieceSquareTables;

	// transposition table variable
	std::optional<TranspositionTable> tt;
	
	// evaluation, move ordering function
	int evaluate();
	void orderMoves(std::vector<Move>& moves, bool useTT);

	// functions for negamax algorithm
	int search(int alpha, int beta, int depth, int maxDepth);
	int quiescenceSearch(int alpha, int beta);

public:
	// constructor and best move function
	AI(Board* boardVar);
	Move getBestMove();
};