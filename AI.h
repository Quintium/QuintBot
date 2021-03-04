#pragma once
#include "Board.h"

// AI class
class AI
{
	Board* board = nullptr;
	Move bestMove;
	int myColor = 0;
	int nodes = 0;

	// function for searching next moves for optimal move
	int search(int color, int alpha, int beta, int depth, int maxDepth);
	std::vector<Move> orderMoves(std::vector<Move> moves, int color);

public:
	AI(Board* boardVar, int aiColor);
	Move getBestMove();
};