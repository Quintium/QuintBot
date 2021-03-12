#pragma once
#include "Board.h"
#include "PieceSquareTables.h"

enum Scores
{
	DRAW_SCORE = 0,
	MATE_SCORE = 100000,
	LOWEST_SCORE = -1000000,
	HIGHEST_SCORE = 1000000
};

// AI class
class AI
{
	Board* board = nullptr;
	std::chrono::time_point<std::chrono::system_clock> searchStart;
	bool searchAborted = false;
	double timeLimit = 100;
	int nodeLimit = 69092;
	Move bestMove = { -1, -1, EMPTY, EMPTY, false, false, EMPTY, 0 };
	const Move invalidMove = { -1, -1, EMPTY, EMPTY, false, false, EMPTY, 0 };
	int myColor = 0;
	int nodes = 0;

	// values for squares of pieces
	PieceSquareTables pieceSquareTables;

	int evaluate(int color);

	// function for searching next moves for optimal move
	int search(int color, int alpha, int beta, int depth, int maxDepth);
	int quiescenseSearch(int color, int alpha, int beta, int depth);
	std::vector<Move> orderMoves(std::vector<Move> moves, int color, bool useBestMove);

public:
	AI(Board* boardVar, int aiColor);
	Move getBestMove();
};