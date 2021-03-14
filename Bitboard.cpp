#include "Bitboard.h"

// shift bitboard by signed int
U64 BB::genShift(U64 x, int shift)
{
	return (shift > 0) ? (x << shift) : (x >> -shift);
}

// shift bitboard by one in any direction
U64 BB::shiftOne(U64 x, int shift)
{
	// get horizontal offset from shift
	int horizontal = (shift % 8 + 8) % 8;

	// erase file according to offset
	if (horizontal == 1)
	{
		x &= notHFile;
	}
	else if (horizontal == 7)
	{
		x &= notAFile;
	}

	return genShift(x, shift);
}

// shift bitboard by two (knight moves) in any direction
U64 BB::shiftTwo(U64 x, int shift)
{
	// get horizontal offset from shift
	int horizontal = (shift % 8 + 8) % 8;

	// erase file according to offset
	if (horizontal == 1)
	{
		x &= notHFile;
	}
	else if (horizontal == 2)
	{
		x &= (notGFile & notHFile);
	}
	else if (horizontal == 6)
	{
		x &= (notAFile & notBFile);
	}
	else if (horizontal == 7)
	{
		x &= notAFile;
	}

	return genShift(x, shift);
}

// calculate king attacks by parallel prefix
U64 BB::kingAttacks(U64 kingSet)
{
	kingSet |= shiftOne(kingSet, EAST) | shiftOne(kingSet, WEST);
	kingSet |= shiftOne(kingSet, SOUTH) | shiftOne(kingSet, NORTH);
	return kingSet;
}

// calculate knight attacks by parallel prefix
U64 BB::knightAttacks(U64 knightSet)
{
	U64 west, east, attacks;
	east = shiftOne(knightSet, EAST);
	west = shiftOne(knightSet, WEST);
	attacks = genShift(east | west, NORTH + NORTH);
	attacks |= genShift(east | west, SOUTH + SOUTH);
	east = shiftOne(east, EAST);
	west = shiftOne(west, WEST);
	attacks |= genShift(east | west, NORTH);
	attacks |= genShift(east | west, SOUTH);
	return attacks;
}

// cast a ray in given direction
U64 BB::rayAttacks(U64 set, U64 empty, int shift)
{
	for (int cycle = 0; cycle < 7; cycle++)
	{
		set |= empty & shiftOne(set, shift);
	}

	return shiftOne(set, shift);
}

// return pawn attacks in given direction
U64 BB::pawnDirAttacks(U64 pawnSet, int color, int shift)
{
	return shiftOne(pawnSet, shift + (color == WHITE ? NORTH : SOUTH));
}

// return pawn attacks in any direction
U64 BB::pawnAnyAttacks(U64 pawnSet, int color)
{
	return pawnDirAttacks(pawnSet, color, EAST) | pawnDirAttacks(pawnSet, color, WEST);
}