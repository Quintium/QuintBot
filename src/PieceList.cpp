#include "PieceList.h"

PieceList::PieceList()
{
	squares.fill(-1);
	map.fill(-1);
}

// add a piece at a square
void PieceList::add(int square)
{
	squares[count] = square;
	map[square] = count;
	count++;
}

// remove piece at a square
void PieceList::remove(int square)
{
	int pieceIndex = map[square];
	if (pieceIndex == count - 1)
	{
		// remove last piece
		squares[pieceIndex] = -1;
		map[square] = -1;
	}
	else
	{
		// insert last piece into slot of removed piece
		map[square] = -1;
		map[squares[count - 1]] = pieceIndex;
		squares[pieceIndex] = squares[count - 1];
		squares[count - 1] = -1;
	}
	count--;
}

// move a piece
void PieceList::move(int from, int to)
{
	squares[map[from]] = to;
	map[to] = map[from];
	map[from] = -1;
}

// access methods
int PieceList::operator[](int index)
{
	return squares[index];
}

int PieceList::getCount()
{
	return count;
}