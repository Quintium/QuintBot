#pragma once
#include "Board.h"
#include "TranspositionTable.h"
#include "Openings.h"
#include "Evaluation.h"

// AI class
class AI
{
	// settings
	bool useOpenings = true;

	// board variable
	Board& board;

	// variables for time control
	std::chrono::time_point<std::chrono::system_clock> searchStart;
	bool searchAborted = false;

	// ai limits
	double timeLimit = 0;
	const double defaultTimeLimit = 7;
	const double moveOverhead = 0.6;

	// saving best move and eval found
	Move bestMove = Move::nullmove();
	int bestEval = LOWEST_SCORE;

	// total nodes searched for debugging
	long long nodes = 0;

	// transposition table and openings variable
	TranspositionTable tt;
	Openings openings;

	// evaluation class
	Evaluation evaluation;

	// functions for negamax algorithm
	int search(int alpha, int beta, int depth, int plyFromRoot, bool nullMove);
	int quiescenceSearch(int alpha, int beta);
	std::string getPrincipalVariation(int depth);

public:
	
	// constructor
	AI(Board& boardPar);

	// new game, best move and evaluation function
	void newGame();
	Move getBestMove(int timeLeft = -1, int increment = 0, int depth = -1, int exactTime = -1);
	int evaluate();
};