#include "Square.h"

// return file or rank number from character
int Square::fromChar(char c)
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
char Square::toChar(int n, bool isRank)
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
int Square::fromString(std::string s)
{
	// convert letters to x and y
	int x = fromChar(s[0]);
	int y = fromChar(s[1]);

	return x + y * 8;
}

// convert x and y values to string, e. g. "a3"
std::string Square::toString(int n)
{
	return std::string(1, toChar(fileOf(n), false)) + toChar(rankOf(n), true);
}

// create square from coordinates
int Square::fromCoords(int file, int rank)
{
	return file + rank * 8;
}

// return file of square
int Square::fileOf(int n)
{
	return n % 8;
}

// return rank of square
int Square::rankOf(int n)
{
	return n / 8;
}

// adjust square if perspective is not white
int Square::perspective(int n, int col)
{
	return (col == WHITE) ? n : (63 - n);
}

// return if square on the board is light
bool Square::isLight(int n)
{
	return (fileOf(n) + rankOf(n)) % 2 == 0;
}