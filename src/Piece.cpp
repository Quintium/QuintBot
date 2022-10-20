#include "Piece.h"

// map for converting chars to ids
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

// map for converting piece ids to value
std::map<int, int> Piece::pieceToValue = {
	{QUEEN, 900},
	{BISHOP, 330},
	{KNIGHT, 320},
	{ROOK, 500},
	{PAWN, 100},
	{KING, 0},
	{EMPTY, 0}
};

// convert a piece char to int
int Piece::charToInt(char c)
{
	return charIDs.at(c);
}

// convert an int to piece char
char Piece::intToChar(int n)
{
	return IDchars.at(n);
}

// return the color of piece
int Piece::colorOf(int piece)
{
	return piece % 2;
}

// return the type (knight, pawn...) of piece
int Piece::typeOf(int piece)
{
	return piece / 2 * 2;
}

// convert piece to value
int Piece::valueOf(int piece)
{
	return pieceToValue.at(typeOf(piece));
}