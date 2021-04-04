#pragma once

#include <optional>
#include "Board.h"
#include "TranspositionTable.h"
#include "PieceSquareTables.h"


class Evaluation
{
	Board* board = nullptr;

	// values for squares of pieces
	PieceSquareTables pieceSquareTables;

public:
	// constructor for class
	Evaluation(Board* boardVar);

	// move ordering function
	void orderMoves(std::vector<Move>& moves, TranspositionTable* tt);

	// evaluation function
	int evaluate();
};