#pragma once
#include <map>

// IDs for all pieces
enum Piece_IDs
{
	EMPTY = -1,
	WHITE = 0,
	BLACK = 1,
	KING = 0,
	QUEEN = 2,
	BISHOP = 4,
	KNIGHT = 6,
	ROOK = 8,
	PAWN = 10
};

// map for converting chars to ids
const std::map<char, int> charIDs = {
	{ 'k', KING + BLACK },
	{ 'q', QUEEN + BLACK },
	{ 'b', BISHOP + BLACK },
	{ 'n', KNIGHT + BLACK },
	{ 'r', ROOK + BLACK },
	{ 'p', PAWN + BLACK },
	{ 'K', KING + WHITE },
	{ 'Q', QUEEN + WHITE },
	{ 'B', BISHOP + WHITE },
	{ 'N', KNIGHT + WHITE },
	{ 'R', ROOK + WHITE },
	{ 'P', PAWN + WHITE }
};

// map for converting piece ids to value
const std::map<int, int> pieceToValue = {
	{QUEEN, 900},
	{BISHOP, 300},
	{KNIGHT, 300},
	{ROOK, 800},
	{PAWN, 100},
	{KING, 0}
};

// class for functions regarding pieces
class Piece {
public:
	// convert a piece char to int
	static int charToInt(char c)
	{
		return charIDs.at(c);
	}

	// return the color of piece
	static int colorOf(int piece)
	{
		return piece % 2;
	}

	// return the type (knight, pawn...) of piece
	static int typeOf(int piece)
	{
		return piece / 2 * 2;
	}

	// convert piece to value
	static int valueOf(int piece)
	{
		return pieceToValue.at(typeOf(piece));
	}
};