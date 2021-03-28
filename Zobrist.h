#pragma once
#include <random>
#include "Bitboard.h"

// class for zobrist hashing
class Zobrist
{
	// random numbers for all board information
	U64 pieceRandoms[12][64];
	U64 turnRandom;
	U64 castlingRandoms[4];
	U64 enPassantRandoms[8];

	// initialize hash key
	U64 hashKey = U64(0);

public:
	// constructor
	Zobrist();

	// reset and set zobrist key
	void reset();
	void set(U64 key);

	// functions for changing zobrist key
	void changePiece(int piece, int square);
	void movePiece(int piece, int from, int to);
	void changeTurn();
	void changeCastling(int index);
	void changeEnPassant(int file);

	// return hash key
	U64 getHashKey();
};