#include "Zobrist.h"

Zobrist::Zobrist()
{
	std::default_random_engine generator;
	std::uniform_int_distribution<U64> distribution(U64(0), ~U64(0));

	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			pieceRandoms[i][j] = distribution(generator);
		}
	}

	turnRandom = distribution(generator);

	for (int i = 0; i < 4; i++)
	{
		castlingRandoms[i] = distribution(generator);
	}

	for (int i = 0; i < 8; i++)
	{
		enPassantRandoms[i] = distribution(generator);
	}
}

void Zobrist::reset()
{
	hashKey = U64(0);
}

void Zobrist::set(U64 key)
{
	hashKey = key;
}

void Zobrist::changePiece(int piece, int square) 
{
	hashKey ^= pieceRandoms[piece][square];
}

void Zobrist::movePiece(int piece, int from, int to)
{
	changePiece(piece, from);
	changePiece(piece, to);
}

void Zobrist::changeTurn()
{
	hashKey ^= turnRandom;
}

void Zobrist::changeCastling(int index)
{
	hashKey ^= castlingRandoms[index];
}

void Zobrist::changeEnPassant(int file)
{
	hashKey ^= enPassantRandoms[file];
}

U64 Zobrist::getHashKey()
{
	return hashKey;
}