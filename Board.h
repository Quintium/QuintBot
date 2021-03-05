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

// structure to store additional position info which isn't stored in moves
struct AdditionalInfo
{
	// properties
	bool castlingRights[4];
	int enPassant;
	int halfMoveClock;

	// constructor to copy castling right array
	AdditionalInfo(bool* rights, int passant, int clock)
	{
		for (int i = 0; i < 4; i++)
		{
			castlingRights[i] = rights[i];
		}

		enPassant = passant;
		halfMoveClock = clock;
	}
};

// class for squares in the board
class Square
{
	public:
	// create new square from a string, e. g. "a3"
	static int fromString(std::string s) 
	{
		// convert letters to x and y
		int x = s[0] - 'a';
		int y = 8 - (s[1] - '0');

		return x + y * 8;
	}

	// create square from two x and y
	static int fromXY(int x, int y)
	{
		return x + y * 8;
	}

	// convert x and y values to string, e. g. "a3"
	static std::string toString(int n)
	{
		return std::string(1, n % 8 + 'a') + (char)('0' + 8 - n / 8);
	}

	// return file of square
	static int file(int n)
	{
		return n % 8;
	}

	// return rank of square
	static int rank(int n)
	{
		return n / 8;
	}
};

// struct for a move
struct Move
{
	int from;
	int to;
	int piece;
	int cPiece;
	bool enPassant;
	bool castling;
	int promotion;
	int score;

	void load(Move move)
	{
		from = move.from;
		to = move.to;
		piece = move.piece;
		cPiece = move.cPiece;
		enPassant = move.enPassant;
		castling = move.castling;
		promotion = move.promotion;
		score = move.score;
	}
};

// enum for different game states
enum State
{
	PLAY,
	WHITE_WIN,
	BLACK_WIN,
	DRAW
};

// class for the board itself
class Board 
{
	// board information (bitboards for all pieces, bitboards for colors, bitboard for all pieces, 8x8 piece array)
	U64 piecesBB[12] = { U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0) };
	U64 takenBB = U64(0);
	U64 colorBB[2] = { U64(0), U64(0) };
	int piecesMB[64] = { EMPTY };
	PieceList pieceLists[12];

	// chess board properties
	bool turnColor = WHITE;
	bool castlingRights[4] = { false, false, false, false };
	int enPassant = -1;
	int halfMoveClock = 0;
	int moveCount = 1;

	// variable to check for checks
	bool isCheck = false;

	// stack for reversing previous moves, list for all possible moves
	std::stack<AdditionalInfo> previousInfo;
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
	// constructor
	Board();

	// load board position from Forsyth-Edwards-Notation
	void loadFromFen(std::string fen);

	// make and unmake a given move
	void makeMove(Move move);
	void unmakeMove(Move move);

	// generate moves based on position, evaluate position
	void generateMoves(bool onlyCaputures = false);

	int getState();

	// return current move color, checks, half move clock, board information and move list
	bool getTurnColor();
	bool getCheck();
	int getHalfMoveClock();
	U64* getPiecesBB();
	int* getPiecesMB();
	std::vector<Move> getMoveList();
	PieceList* getPieceLists();
};