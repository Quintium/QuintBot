#pragma once
#include "Piece.h"

// class for squares in the board
class Square
{
public:
	// return file or rank number from character
	static int fromChar(char c)
	{
		if (std::isdigit(c))
		{
			return 8 - (c - '0');
		}
		else
		{
			return c - 'a';
		}
	}

	// return character from file or rank number
	static char toChar(int n, bool isRank)
	{
		if (isRank)
		{
			return '0' + 8 - n;
		}
		else
		{
			return 'a' + n;
		}
	}

	// create new square from a string, e. g. "a3"
	static int fromString(std::string s)
	{
		// convert letters to x and y
		int x = fromChar(s[0]);
		int y = fromChar(s[1]);

		return x + y * 8;
	}

	// convert x and y values to string, e. g. "a3"
	static std::string toString(int n)
	{
		return std::string(1, toChar(fileOf(n), false)) + toChar(rankOf(n), true);
	}

	// create square from two x and y
	static int fromXY(int x, int y)
	{
		return x + y * 8;
	}

	// return file of square
	static int fileOf(int n)
	{
		return n % 8;
	}

	// return rank of square
	static int rankOf(int n)
	{
		return n / 8;
	}

	// adjust square if perspective is not white
	static int perspective(int n, int col)
	{
		return (col == WHITE) ? n : (63 - n);
	}

	// reverse coordinate if perspective is not white
	static int coordPerspective(int n, int col)
	{
		return (col == WHITE) ? n : (7 - n);
	}

	// return if square on the board should be light
	static bool isLight(int n)
	{
		return (fileOf(n) + rankOf(n)) % 2 == 0;
	}
};

// struct for a move
struct Move
{
	// all move properties
	int from; // square the piece moved from
	int to; // square the piece moved to
	int piece; // piece that moved
	int cPiece; // piece that was captured
	bool enPassant; // if move was en passant
	bool castling; // if move was castling
	int promotion; // piece that was promoted to
	int score; // score of move during move ordering

	// comparisons and notation
	bool operator==(Move move);
	std::string getNotation();

	// loading moves
	static Move loadFromSquares(int from, int to, int* piecesMB);
	static Move loadFromNotation(std::string notation, int* piecesMB);

	// null moves
	static Move nullmove();
	static bool isNull(Move move);
};