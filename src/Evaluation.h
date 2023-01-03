#pragma once

#include <optional>
#include "Board.h"
#include "TranspositionTable.h"
#include "PieceSquareTables.h"


class Evaluation
{
	Board& board;

	// transposition table
	TranspositionTable& tt;

	// values of pieces
	std::map<int, int> pieceValues;

	// values for squares of pieces
	PieceSquareTables pieceSquareTables;

	// pawn shield bitboards for each color and wing
	U64 pawnShieldBBs[2][2];
	U64 nearKingSquares[64];

	// bitboards for files
	U64 fileBBs[8];
	
	// array of all ray directions
	int dirs[16] = { EAST,       WEST,       NORTH,      SOUTH,
					 NORTH_EAST, SOUTH_WEST, SOUTH_EAST, NORTH_WEST};

public:
	// constructor for class
	Evaluation(Board& boardPar, TranspositionTable& ttPar);

	// move ordering function
	void orderMoves(std::vector<Move>& moves);

	// evaluation functions
	double getEndgameWeight();
	int evaluate();
};