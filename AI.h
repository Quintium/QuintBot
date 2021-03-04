#pragma once
#include "Board.h"

// AI class
class AI
{
	Board* board = nullptr;
	Move bestMove;
	int myColor = 0;

	// function for searching next moves for optimal move
	int search(int color, int depth, int maxDepth);

public:
	AI(Board* boardVar, int aiColor);
	Move getBestMove(int depth);
};