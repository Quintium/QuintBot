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
	// return file or rank number from character
	static int fromChar(char c)
	{
		if (std::isdigit(c))
		{
			return 8 - (c - '0');
		}
		else
		{
			return c - 'a';
		}
	}

	// return character from file or rank number
	static char toChar(int n, bool isRank)
	{
		if (isRank)
		{
			return '0' + 8 - n;
		}
		else
		{
			return 'a' + n;
		}
	}

	// create new square from a string, e. g. "a3"
	static int fromString(std::string s) 
	{
		// convert letters to x and y
		int x = fromChar(s[0]);
		int y = fromChar(s[1]);

		return x + y * 8;
	}

	// convert x and y values to string, e. g. "a3"
	static std::string toString(int n)
	{
		return std::string(1, toChar(fileOf(n), false)) + toChar(rankOf(n), true);
	}

	// create square from two x and y
	static int fromXY(int x, int y)
	{
		return x + y * 8;
	}

	// return file of square
	static int fileOf(int n)
	{
		return n % 8;
	}

	// return rank of square
	static int rankOf(int n)
	{
		return n / 8;
	}

	// adjust square if perspective is not white
	static int perspective(int n, int col)
	{
		return (col == WHITE) ? n : (63 - n);
	}

	// reverse coordinate if perspective is not white
	static int coordPerspective(int n, int col)
	{
		return (col == WHITE) ? n : (7 - n);
	}

	// return if square on the board should be light
	static bool isLight(int n)
	{
		return (fileOf(n) + rankOf(n)) % 2 == 0;
	}
};

// struct for a move
struct Move
{
	// all move properties
	int from;
	int to;
	int piece;
	int cPiece;
	bool enPassant;
	bool castling;
	int promotion;
	int score;

	// checking if two moves are the same by checking from and to squares and promotion piece
	bool operator==(Move move)
	{
		return (move.from == from) && (move.to == to) && (move.promotion == promotion);
	}

	// return notation of move e.g. "a1f3" or "c2c1q" 
	std::string getNotation()
	{
		// handle nullmoves
		if (*this == Move::getInvalidMove())
		{
			return "0000";
		}

		// concatenate from and to squares as base of move
		std::string base = Square::toString(from) + Square::toString(to);

		// if there's a promotion, specify the promotion piece
		if (promotion != -1)
		{
			switch (Piece::typeOf(promotion))
			{
			case QUEEN:
				base += "q";
				break;
			case ROOK:
				base += "r";
				break;
			case BISHOP:
				base += "b";
				break;
			case KNIGHT:
				base += "n";
				break;
			}
		}

		return base;
	}

	// load move from source and target squares
	static Move loadFromSquares(int from, int to, int* piecesMB)
	{
		// move properties
		int movedPiece = piecesMB[from];
		int takenPiece = piecesMB[to];
		int promotionPiece = EMPTY;
		bool isEnPassant = false;
		bool isCastling = false;

		// get piece color and type
		int pieceColor = Piece::colorOf(movedPiece);
		int pieceType = Piece::typeOf(movedPiece);

		// en passant move properties
		if ((from - to) % 8 != 0 && piecesMB[to] == EMPTY && pieceType == PAWN)
		{
			takenPiece = !pieceColor + PAWN;
			isEnPassant = true;
		}

		// castling move properties
		if (std::abs(from - to) == 2 && pieceType == KING)
		{
			isCastling = true;
		}

		// promotion move properties
		if ((Square::rankOf(to) == pieceColor * 7) && (pieceType == PAWN))
		{
			promotionPiece = QUEEN + pieceColor;
		}

		// return move based on properties
		Move move = { from, to, movedPiece, takenPiece, isEnPassant, isCastling, promotionPiece, 0 };
		return move;
	}

	// load move from long algebraic notation
	static Move loadFromNotation(std::string notation, int* piecesMB)
	{
		// handle nullmoves
		if (notation == "0000")
		{
			return Move::getInvalidMove();
		}

		int from = Square::fromString(notation.substr(0, 2));
		int to = Square::fromString(notation.substr(2, 4));
		Move move = loadFromSquares(from, to, piecesMB);

		int pieceColor = Piece::colorOf(piecesMB[from]);
		if (notation.size() == 5)
		{
			switch (notation[4])
			{
			case 'q':
				move.promotion = QUEEN + pieceColor;
				break;
			case 'r':
				move.promotion = ROOK + pieceColor;
				break;
			case 'b':
				move.promotion = BISHOP + pieceColor;
				break;
			case 'n':
				move.promotion = KNIGHT + pieceColor;
				break;
			}
		}

		return move;
	}

	// return an invalid move
	static Move getInvalidMove()
	{
		Move newMove = { EMPTY, EMPTY, EMPTY, EMPTY, false, false, EMPTY, 0 };
		return newMove;
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

	// stack for reversing previous moves, list of all previous positions, move history
	std::stack<AdditionalInfo> previousInfo;
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

	// get the state of the game and check for repetition
	int getState();
	bool repeatedPosition();

	// return current move color, checks and half move clock
	int getTurnColor();
	bool getCheck();
	int getHalfMoveClock();

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