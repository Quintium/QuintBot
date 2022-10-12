#include "Bitboard.h"

U64 BB::excludeFiles[8] = { 0xFFFFFFFFFFFFFFFF, 0x7f7f7f7f7f7f7f7f, 0x3F3F3F3F3F3F3F3F, 0, 0, 0, 0xFCFCFCFCFCFCFCFC, 0xFEFEFEFEFEFEFEFE };

// shift bitboard by signed int
U64 BB::genShift(U64 x, int shift)
{
	return (shift > 0) ? (x << shift) : (x >> -shift);
}

// shift bitboard by max two tiles (knight moves) in any direction
U64 BB::shiftTwo(U64 x, int shift)
{
	// get horizontal offset from shift
	int horizontal = (shift % 8 + 8) % 8;

	// erase file according to offset
	x &= excludeFiles[horizontal];

	return genShift(x, shift);
}

// calculate king attacks by parallel prefix
U64 BB::kingAttacks(U64 kingSet)
{
	kingSet |= shiftTwo(kingSet, EAST) | shiftTwo(kingSet, WEST);
	kingSet |= shiftTwo(kingSet, SOUTH) | shiftTwo(kingSet, NORTH);
	return kingSet;
}

// calculate knight attacks by parallel prefix
U64 BB::knightAttacks(U64 knightSet)
{
	U64 west, east, attacks;
	east = shiftTwo(knightSet, EAST);
	west = shiftTwo(knightSet, WEST);
	attacks = genShift(east | west, NORTH + NORTH);
	attacks |= genShift(east | west, SOUTH + SOUTH);
	east = shiftTwo(east, EAST);
	west = shiftTwo(west, WEST);
	attacks |= genShift(east | west, NORTH);
	attacks |= genShift(east | west, SOUTH);
	return attacks;
}

// cast a ray in given direction
U64 BB::rayAttacks(U64 set, U64 empty, int shift)
{
	for (int cycle = 0; cycle < 7; cycle++)
	{
		set |= empty & shiftTwo(set, shift);
	}

	return shiftTwo(set, shift);
}

// return pawn attacks in given direction
U64 BB::pawnDirAttacks(U64 pawnSet, int color, int shift)
{
	return shiftTwo(pawnSet, shift + (color == WHITE ? NORTH : SOUTH));
}

// return pawn attacks in any direction
U64 BB::pawnAnyAttacks(U64 pawnSet, int color)
{
	return pawnDirAttacks(pawnSet, color, EAST) | pawnDirAttacks(pawnSet, color, WEST);
}