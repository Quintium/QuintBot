#pragma once

#include <cstddef>
#include <optional>
#include "Board.h"
#include "Score.h"

// different node types of entry
enum class NodeType
{
	EXACT,
	UPPER_BOUND,
	LOWER_BOUND
};

// an entry into the transposition table
struct Entry
{
	int eval;
	int depth;
	Move move;
	NodeType nodeType;
	U64 key;
};

// class for the transposition table
class TranspositionTable
{
	// maximum size of entry list and entry list
	int size = 100000;
	std::vector<Entry> entries;

	// board variable
	Board* board = nullptr;

public:
	// constructor and clear table
	TranspositionTable(Board* boardVar);
	void clear();

	// get stored entry, move and eval for current board position
	std::optional<Entry> getStoredEntry();
	Move getStoredMove();
	std::optional<int> getStoredEval(int depth, int numPly, int alpha, int beta);

	// store an entry for current position
	void storeEntry(int eval, int depth, Move move, NodeType nodeType, int numPly);

	// get current index in table
	int getIndex();
};