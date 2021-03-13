#pragma once
#include "Board.h"
#include "PieceSquareTables.h"
#include "TranspositionTable.h"

// AI class
class AI
{
	Board* board = nullptr;
	std::chrono::time_point<std::chrono::system_clock> searchStart;
	bool searchAborted = false;
	const double timeLimit = 100;
	const int depthLimit = 5;
	Move bestMove = Move::getInvalidMove();
	int myColor = 0;
	int nodes = 0;

	// values for squares of pieces
	PieceSquareTables pieceSquareTables;
	TranspositionTable* tt = nullptr;

	int evaluate();

	// function for searching next moves for optimal move
	int search( int alpha, int beta, int depth, int maxDepth);
	int quiescenseSearch(int alpha, int beta, int depth);
	std::vector<Move> orderMoves(std::vector<Move> moves, bool useBestMove);

public:
	AI(Board* boardVar, int aiColor);
	Move getBestMove();
};