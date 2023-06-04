#pragma once
#include <array>

// class for list of a particular piece type
class PieceList
{
	// list of all squares the pieces are on
	std::array<int, 64> squares;

	// map from square to index in the squares list
	std::array<int, 64> map;

	// count of this piece
	int count = 0;

public:
	PieceList();

	// manage pieces
	void add(int square);
	void remove(int square);
	void move(int from, int to);

	// access methods
	int operator[](int index);
	int getCount();
};