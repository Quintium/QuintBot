#pragma once
#include <bit>
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
	// bitmasks for excluding certain files when shifting bitboards
	static U64 excludeFiles[8];

public:
	// general operations (dependent on OS)
	static int bitScanForward(U64 x);
	static int popCount(U64 x);

	// shifting bitboards
	static U64 genShift(U64 x, int shift);
	static U64 shiftTwo(U64 x, int shift);

	// attacks for pieces
	static U64 kingAttacks(U64 kingSet);
	static U64 knightAttacks(U64 knightSet);
	static U64 rayAttacks(U64 set, U64 empty, int shift);
	static U64 pawnDirAttacks(U64 pawnSet, int color, int shift);
	static U64 pawnAnyAttacks(U64 pawnSet, int color);

	// other bitboard algorithms
	static U64 dirFill(U64 set, int shift, bool excludeOriginal);
	static U64 fileFill(U64 set);
};