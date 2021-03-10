#pragma once
#include "Board.h"
#include "PieceSquareTables.h"

// AI class
class AI
{
	Board* board = nullptr;
	Move bestMove = { -1, -1, EMPTY, EMPTY, false, false, EMPTY, 0 };
	int myColor = 0;
	int nodes = 0;

	// values for squares of pieces
	PieceSquareTables pieceSquareTables;

	int evaluate(int color);

	// function for searching next moves for optimal move
	int search(int color, int alpha, int beta, int depth, int maxDepth);
	int quiescenseSearch(int color, int alpha, int beta, int depth);
	std::vector<Move> orderMoves(std::vector<Move> moves, int color);

public:
	AI(Board* boardVar, int aiColor);
	Move getBestMove();
};