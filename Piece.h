#pragma once
#include <map>
#include <iostream>

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

// class for functions regarding pieces
class Piece {
	// maps for pieces
	static std::map<char, int> charIDs;
	static std::map<int, char> IDchars;
	static std::map<int, int> pieceToValue;

public:
	// functions for handling pieces
	static int charToInt(char c);
	static char intToChar(int n);
	static int colorOf(int piece);
	static int typeOf(int piece);
	static int valueOf(int piece);
};