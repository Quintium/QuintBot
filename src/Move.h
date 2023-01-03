#pragma once
#include "Square.h"
#include "Piece.h"

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