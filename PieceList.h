#pragma once
#include <vector>

// class for list of a particular piece
class PieceList
{
	// list of all squares the pieces are on
	int squares[64] = {};

	// map from square to index in the squares list
	int map[64] = {};

	// count of this piece
	int count = 0;

public:
	// add a piece at a square
	void add(int square)
	{
		squares[count] = square;;
		map[square] = count;
		count++;
	}

	// remove piece by replacing that piece by the last piece
	void remove(int square)
	{
		int pieceIndex = map[square];
		squares[pieceIndex] = squares[count - 1];
		map[squares[pieceIndex]] = pieceIndex;
		count--;
	}

	// move a piece
	void move(int from, int to)
	{
		squares[map[from]] = to;
		map[to] = map[from];
	}

	// get the square at an index
	int operator[](int index)
	{
		return squares[index];
	}

	// return the count of this piece
	int getCount()
	{
		return count;
	}
};