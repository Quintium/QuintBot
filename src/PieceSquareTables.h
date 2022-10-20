#pragma once
#include <vector>
#include "Piece.h"
#include "Board.h"

// class for managing piece square tables
class PieceSquareTables
{
	// normal piece square tables
	std::map<int, std::vector<int>> tables;

	// piece square table for king in endgame
	std::vector<int> kingEnd;

public:
	// constructor
	PieceSquareTables();

	// get the score of a piece on a square
	int getScore(int piece, int square, float endgameWeight);
};