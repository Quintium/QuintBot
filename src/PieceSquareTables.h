#pragma once
#include <vector>
#include "Piece.h"
#include "Board.h"

// class for managing piece square tables
class PieceSquareTables
{
	std::map<int, std::array<int, 64>> tables;

	// piece square table for king in endgame
	std::array<int, 64> kingEnd;

public:
	PieceSquareTables();
	int getScore(int piece, int square, double endgameWeight);
};