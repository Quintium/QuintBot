#pragma once
#include <vector>

class PieceList
{
	int squares[64] = {};
	int map[64] = {};
	int count = 0;

public:
	void add(int square)
	{
		squares[count] = square;;
		map[square] = count;
		count++;
	}

	void remove(int square)
	{
		int pieceIndex = map[square];
		squares[pieceIndex] = squares[count - 1];
		map[squares[pieceIndex]] = pieceIndex;
		count--;
	}

	void move(int from, int to)
	{
		squares[map[from]] = to;
		map[to] = map[from];
	}

	int operator[](int index)
	{
		return squares[index];
	}

	int getCount()
	{
		return count;
	}
};