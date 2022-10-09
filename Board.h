#pragma once

// include libraries and files
#include <cstdint>
#include <cctype>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <chrono>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "Texture.h"
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
	// properties
	bool castlingRights[4];
	int enPassant;
	int halfMoveClock;

	// constructor to copy castling right array
	PositionalInfo(bool* rights, int passant, int clock)
	{
		for (int i = 0; i < 4; i++)
		{
			castlingRights[i] = rights[i];
		}

		enPassant = passant;
		halfMoveClock = clock;
	}
};

// class for the board itself
class Board 
{
	// starting board position
	const std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	// board information (bitboards for all pieces, bitboards for colors, bitboard for all pieces, 8x8 piece array)
	U64 piecesBB[12] = {};
	U64 takenBB = U64(0);
	U64 colorBB[2] = {};
	int piecesMB[64] = {};
	PieceList pieceLists[12] = {};

	// chess board properties
	bool turnColor = WHITE;
	bool castlingRights[4] = { false, false, false, false };
	int enPassant = -1;
	int halfMoveClock = 0;
	int moveCount = 1;
	Zobrist zobrist;

	// variable to check for checks
	bool isCheck = false;

	// stack for reversing previous moves, list of all zobrist keys of previous positions, move history
	std::stack<PositionalInfo> previousInfo;
	std::vector<U64> previousPositions;
	std::vector<Move> moveHistory;
	bool normalStart = true;

	// list of all possible moves in current position
	std::vector<Move> moveList;

	// direction array and direction to index map
	int dirs[16] = { EAST,             WEST,             NORTH,            SOUTH,
			    	 NORTH_EAST,       SOUTH_WEST,       SOUTH_EAST,       NORTH_WEST,
			    	 NORTH_NORTH_EAST, SOUTH_SOUTH_EAST, NORTH_NORTH_WEST, SOUTH_SOUTH_WEST,
					 NORTH_EAST_EAST,  NORTH_WEST_WEST,  SOUTH_EAST_EAST,  SOUTH_WEST_WEST };
	 
	std::map<int, int> dirToIndex = { {EAST, 0},             {WEST, 1},             {NORTH, 2},             {SOUTH, 3},
									  {NORTH_EAST, 4},       {SOUTH_WEST, 5},       {SOUTH_EAST, 6},        {NORTH_WEST, 7},
									  {NORTH_NORTH_EAST, 8}, {SOUTH_SOUTH_EAST, 9}, {NORTH_NORTH_WEST, 10}, {SOUTH_SOUTH_WEST, 11},
									  {NORTH_EAST_EAST, 12}, {NORTH_WEST_WEST, 13}, {SOUTH_EAST_EAST, 14},  {SOUTH_WEST_WEST, 15} };


public:
	// load and get board position from Forsyth-Edwards-Notation
	void loadStartPosition();
	void loadFromFen(std::string fen);
	std::string getFen();

	// change all piece information on board with a single function
	void movePiece(int piece, int from, int to);
	void addPiece(int piece, int square);
	void removePiece(int piece, int square);

	// make and unmake a given move
	void makeMove(Move move);
	void unmakeMove(Move move);

	// generate moves based on position
	void generateMoves(bool onlyCaputures = false);

	// get the state of the game
	int getState();

	// return current move color, checks and half move clock
	int getTurnColor();
	bool getCheck();
	int getHalfMoveClock();
	int getMoveCount();

	// return board information
	U64* getPiecesBB();
	int* getPiecesMB();
	PieceList* getPieceLists();

	// return zobrist key
	U64 getZobristKey();

	// return possible moves and move history
	std::vector<Move> getMoveList();
	std::vector<Move> getMoveHistory();
	bool getNormalStart();
};