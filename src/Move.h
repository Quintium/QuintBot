#pragma once
#include "Square.h"
#include "Piece.h"
#include <array>

// struct for a move
struct Move
{
	// all move properties
	char from; // square the piece moved from
	char to; // square the piece moved to
	char piece; // piece that moved
	char cPiece; // piece that was captured
	bool enPassant; // if move was en passant
	bool castling; // if move was castling
	char promotion; // piece that was promoted to
	int score; // score of move during move ordering

	// comparisons and notation
	bool operator==(Move move);
	std::string getNotation();

	// loading moves
	static Move loadFromSquares(int from, int to, const std::array<int, 64>& piecesMB);
	static Move loadFromNotation(std::string notation, const std::array<int, 64>& piecesMB);

	// null moves
	static Move nullmove();
	static bool isNull(Move move);
};