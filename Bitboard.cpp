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

// bitscan forward algorithm from chess programming wiki
int BB::bitScanForward(U64 bb) {
	const int index64[64] = {
		0, 47,  1, 56, 48, 27,  2, 60,
	   57, 49, 41, 37, 28, 16,  3, 61,
	   54, 58, 35, 52, 50, 42, 21, 44,
	   38, 32, 29, 23, 17, 11,  4, 62,
	   46, 55, 26, 59, 40, 36, 15, 53,
	   34, 51, 20, 43, 31, 22, 10, 45,
	   25, 39, 14, 33, 19, 30,  9, 24,
	   13, 18,  8, 12,  7,  6,  5, 63
	};
	const U64 debruijn64 = U64(0x03f79d71b4cb0a89);
	return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
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