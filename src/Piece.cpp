#include "Piece.h"

// map for converting ids to chars
std::map<int, char> Piece::IDchars = {
	{ KING + BLACK, 'k' },
	{ QUEEN + BLACK, 'q' },
	{ BISHOP + BLACK, 'b' },
	{ KNIGHT + BLACK, 'n' },
	{ ROOK + BLACK, 'r' },
	{ PAWN + BLACK, 'p' },
	{ KING + WHITE, 'K' },
	{ QUEEN + WHITE, 'Q' },
	{ BISHOP + WHITE, 'B' },
	{ KNIGHT + WHITE, 'N' },
	{ ROOK + WHITE, 'R' },
	{ PAWN + WHITE, 'P' }
};

// map for converting chars to ids
std::map<char, int> Piece::charIDs = {
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

// conversion between piece chars and ids
int Piece::charToInt(char c)
{
	return charIDs.at(c);
}

char Piece::intToChar(int n)
{
	return IDchars.at(n);
}

// return the color and type of piece
int Piece::colorOf(int piece)
{
	return piece % 2;
}

int Piece::typeOf(int piece)
{
	return piece / 2 * 2;
}