#pragma once

#include <optional>
#include "Board.h"
#include "TranspositionTable.h"
#include "PieceSquareTables.h"


class Evaluation
{
	// Command line arguments
	std::vector<int> args;

	// Board variable
	Board& board;

	// transposition table
	TranspositionTable& tt;

	// values of pieces
	std::map<int, int> pieceValues;

	// values for squares of pieces
	PieceSquareTables pieceSquareTables;

	// pawn shield bitboards for each color and wing
	std::array<std::array<U64, 2>, 2> pawnShieldBBs;
	std::array<U64, 64> nearKingSquares;

	// bitboards for files
	std::array<U64, 8> fileBBs;

	// array of all ray directions
	std::array<int, 16> dirs = { EAST,       WEST,       NORTH,      SOUTH,
								 NORTH_EAST, SOUTH_WEST, SOUTH_EAST, NORTH_WEST };

public:
	// constructor for class
	Evaluation(Board& boardPar, TranspositionTable& ttPar, std::vector<int> argsPar);

	// move ordering function
	void orderMoves(std::vector<Move>& moves);

	// evaluation functions
	double getEndgameWeight();
	int evaluate();
};