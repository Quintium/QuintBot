#pragma once
#include "Board.h"
#include "TranspositionTable.h"
#include "Openings.h"
#include "Evaluation.h"

// class for a chess engine
class Engine
{
	// UCI options
	bool useOpeningBook = true;
	double moveOverhead = 0.01;

	Board board;

	std::chrono::time_point<std::chrono::system_clock> searchStart;
	bool searchAborted = false;

	double timeLimit = 0;
	const double defaultTimeLimit = 7;

	Move bestMove = Move::nullmove();
	int bestEval = LOWEST_SCORE;

	long long nodes = 0;

	TranspositionTable tt;
	Openings openings;

	Evaluation evaluation;

	int search(int alpha, int beta, int depth, int plyFromRoot, bool nullMove);
	int quiescenceSearch(int alpha, int beta);
	std::string getPrincipalVariation(int depth);

public:
	
	Engine();

	// actions upon loading board position
	void loadStartPosition();
	void loadFromFen(std::string fen);

	// actions when move is played/unplayed
	void makeMove(Move move);
	void unmakeMove(Move move);

	void newGame();
	Move getBestMove(int timeLeft = -1, int increment = 0, int depth = -1, int exactTime = -1);
	int evaluate();

	// change UCI options
	void setOwnBook(bool useOwnBook);
	void setHash(int sizeMB);
	void setMoveOverhead(int moveOverheadMs);

	Board& getBoard();
};