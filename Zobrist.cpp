#include "Zobrist.h"

// zobrist constructor
Zobrist::Zobrist()
{
	// create random number generator and int distribution
	std::mt19937_64 generator(787659);
	std::uniform_int_distribution<U64> distribution(U64(0), ~U64(0));

	// loop through all pieces and squares
	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			// create random number for that piece and square
			pieceRandoms[i][j] = distribution(generator);
		}
	}

	// create random number for turn
	turnRandom = distribution(generator);

	// create random numbers for castling rights
	for (int i = 0; i < 4; i++)
	{
		castlingRandoms[i] = distribution(generator);
	}

	// create random numbers for en passant files
	for (int i = 0; i < 8; i++)
	{
		enPassantRandoms[i] = distribution(generator);
	}
}

// reset hash key
void Zobrist::reset()
{
	hashKey = U64(0);
}

// set hash key to specific key
void Zobrist::set(U64 key)
{
	hashKey = key;
}

// change a piece on a square
void Zobrist::changePiece(int piece, int square) 
{
	hashKey ^= pieceRandoms[piece][square];
}

// move a piece
void Zobrist::movePiece(int piece, int from, int to)
{
	changePiece(piece, from);
	changePiece(piece, to);
}

// change turn
void Zobrist::changeTurn()
{
	hashKey ^= turnRandom;
}

// change castling right
void Zobrist::changeCastling(int index)
{
	hashKey ^= castlingRandoms[index];
}

// change en passant right
void Zobrist::changeEnPassant(int file)
{
	hashKey ^= enPassantRandoms[file];
}

// return current hash key
U64 Zobrist::getHashKey()
{
	return hashKey;
}