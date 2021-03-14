#pragma once
#include "Piece.h"

// rename uint64_t to U64 for easier access to bitboards
typedef uint64_t U64;
typedef int64_t I64;

// shift numbers for each direction
enum Directions
{
	NORTH = -8,
	EAST = 1,
	SOUTH = 8,
	WEST = -1,

	NORTH_EAST = -7,
	SOUTH_EAST = 9,
	NORTH_WEST = -9,
	SOUTH_WEST = 7,

	NORTH_NORTH_EAST = -15,
	SOUTH_SOUTH_EAST = 17,
	NORTH_NORTH_WEST = -17,
	SOUTH_SOUTH_WEST = 15,
	NORTH_EAST_EAST = -6,
	NORTH_WEST_WEST = -10,
	SOUTH_EAST_EAST = 10,
	SOUTH_WEST_WEST = 6
};

// class for altering bitboards
class BB
{
	// bitboards for excluding certain files
	static const U64 notAFile = 0xfefefefefefefefe;
	static const U64 notBFile = 0xFDFDFDFDFDFDFDFD;
	static const U64 notGFile = 0xBFBFBFBFBFBFBFBF;
	static const U64 notHFile = 0x7f7f7f7f7f7f7f7f;

public:
	// shifting bitboards
	static U64 genShift(U64 x, int shift);
	static U64 shiftOne(U64 x, int shift);
	static U64 shiftTwo(U64 x, int shift);

	// attacks for pieces
	static U64 kingAttacks(U64 kingSet);
	static U64 knightAttacks(U64 knightSet);
	static U64 rayAttacks(U64 set, U64 empty, int shift);
	static U64 pawnDirAttacks(U64 pawnSet, int color, int shift);
	static U64 pawnAnyAttacks(U64 pawnSet, int color);
};