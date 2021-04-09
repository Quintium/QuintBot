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

	// pawn shield bitboards for each color and wing
	U64 pawnShieldBBs[2][2];
	U64 nearKingSquares[64];

	// bitboard of middle files
	U64 middleFiles = 0x1818181818181818;
	
	// array of all ray directions
	int dirs[16] = { EAST,       WEST,       NORTH,      SOUTH,
					 NORTH_EAST, SOUTH_WEST, SOUTH_EAST, NORTH_WEST};

public:
	// constructor for class
	Evaluation(Board* boardVar);

	// move ordering function
	void orderMoves(std::vector<Move>& moves, TranspositionTable* tt);

	// evaluation function
	int evaluate();
};