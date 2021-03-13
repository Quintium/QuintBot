#pragma once

#include "Board.h"
#include "Score.h"

enum class NodeType
{
	EXACT,
	UPPER_BOUND,
	LOWER_BOUND
};

struct Entry
{
	int eval;
	int depth;
	Move move;
	NodeType nodeType;
	U64 key;
};

class TranspositionTable
{
	int size = 65536;
	Board* board = nullptr;
	std::vector<Entry> entries;

public:
	TranspositionTable(Board* myBoard);
	Move getStoredMove();
	int getStoredEval(int depth, int numPly, int alpha, int beta);
	void storeEntry(int eval, int depth, Move move, NodeType type, int numPly);
	int getIndex();
};