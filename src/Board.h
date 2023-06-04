#pragma once

#include <cctype>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <chrono>
#include <algorithm>

#include "Bitboard.h"
#include "PieceList.h"
#include "Zobrist.h"
#include "Move.h"

// enum for different game states
enum State
{
	PLAY,
	WHITE_WIN,
	BLACK_WIN,
	DRAW
};

// structure to store positional info which isn't stored in moves (to avoid added complexity in move creation)
struct PositionalInfo
{
	std::array<bool, 4> castlingRights;
	int enPassantSquare;
	int halfMoveClock;
};

// class for the chess board
class Board
{
	const std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	// board information (bitboards, 8x8 mailbox, piece lists)
	std::array<U64, 12> piecesBB = {};
	U64 takenBB = U64(0);
	std::array<U64, 2> colorBB = {};
	std::array<int, 64> piecesMB = {};
	std::array<PieceList, 12> pieceLists = {};

	// chess board properties
	bool turnColor = WHITE;
	std::array<bool, 4> castlingRights = { false, false, false, false };
	int enPassantSquare = -1;
	int halfMoveClock = 0;
	int moveCount = 1;
	Zobrist zobrist;
	bool isCheck = false;
	bool normalStart = true;

	// stack for reversing previous moves, list of all zobrist keys of previous positions, move history
	std::stack<PositionalInfo> previousInfo;
	std::vector<U64> previousPositions;
	std::vector<Move> moveHistory;

	// list of all possible moves in current position
	std::vector<Move> moveList;

	// direction array and direction to index map
	std::array<int, 16> dirs = { EAST,             WEST,             NORTH,            SOUTH,
			    				 NORTH_EAST,       SOUTH_WEST,       SOUTH_EAST,       NORTH_WEST,
			    				 NORTH_NORTH_EAST, SOUTH_SOUTH_EAST, NORTH_NORTH_WEST, SOUTH_SOUTH_WEST,
								 NORTH_EAST_EAST,  NORTH_WEST_WEST,  SOUTH_EAST_EAST,  SOUTH_WEST_WEST };
	 
	std::map<int, int> dirToIndex = { {EAST, 0},             {WEST, 1},             {NORTH, 2},             {SOUTH, 3},
									  {NORTH_EAST, 4},       {SOUTH_WEST, 5},       {SOUTH_EAST, 6},        {NORTH_WEST, 7},
									  {NORTH_NORTH_EAST, 8}, {SOUTH_SOUTH_EAST, 9}, {NORTH_NORTH_WEST, 10}, {SOUTH_SOUTH_WEST, 11},
									  {NORTH_EAST_EAST, 12}, {NORTH_WEST_WEST, 13}, {SOUTH_EAST_EAST, 14},  {SOUTH_WEST_WEST, 15} };

	// helper functions for makeMove
	void movePiece(int piece, int from, int to);
	void addPiece(int piece, int square);
	void removePiece(int piece, int square);
	void rookChanged(int square);

public:
	// load and get board position from FEN
	void loadStartPosition();
	void loadFromFen(std::string fen);
	std::string getFen();

	// make and unmake a given move
	void makeMove(Move move);
	void unmakeMove(Move move);

	// generate moves based on position
	void generateMoves(bool onlyCaputures = false);

	// get the state of the game
	bool checkDraw();
	int getState();
	bool checkRepetition();

	// return board properties
	int getTurnColor();
	bool getCheck();
	int getHalfMoveClock();
	int getMoveCount();
	bool getNormalStart();

	// return piece information
	std::array<U64, 12> getPiecesBB();
	std::array<int, 64> getPiecesMB();
	std::array<PieceList, 12> getPieceLists();

	// return zobrist key
	U64 getZobristKey();

	// return legal moves and move history
	std::vector<Move> getMoveList();
	std::vector<Move> getMoveHistory();
};