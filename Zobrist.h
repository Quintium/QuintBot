#pragma once
#include <random>
#include "Bitboard.h"

class Zobrist
{
	U64 pieceRandoms[12][64];
	U64 turnRandom;
	U64 castlingRandoms[4];
	U64 enPassantRandoms[8];

	U64 hashKey = U64(0);

public:
	Zobrist();
	void reset();
	void set(U64 key);
	void changePiece(int piece, int square);
	void movePiece(int piece, int from, int to);
	void changeTurn();
	void changeCastling(int index);
	void changeEnPassant(int file);
	U64 getHashKey();
};