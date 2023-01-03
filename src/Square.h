#pragma once
#include <string>
#include "Piece.h"

// class for squares in the board
class Square
{
public:
	// file/rank - character conversion
	static int fromChar(char c);
	static char toChar(int n, bool isRank);

	// square - string conversion
	static int fromString(std::string s);
	static std::string toString(int n);

	// create square from coordinates
	static int fromCoords(int file, int rank);

	// file/rank of square
	static int fileOf(int n);
	static int rankOf(int n);

	// adjust square for perspective
	static int perspective(int n, int color);

	// return if square on the board is light
	static bool isLight(int n);
};