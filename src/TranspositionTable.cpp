#include "TranspositionTable.h"

// create the entries array and initialize board variable
TranspositionTable::TranspositionTable(Board* boardVar)
{
	entries.resize(size);
	board = boardVar;
}

// clear all entries
void TranspositionTable::clear()
{
	for (int i = 0; i < size; i++)
	{
		entries[i].valid = false;
	}
}

// get the stored move at this board position
Move TranspositionTable::getStoredMove()
{
	// check if entry key is the board zobrist key
	Entry entry = entries[getIndex()];
	if (entry.key == board->getZobristKey() && entry.valid)
	{
		// if yes -> return the move
		return entry.move;
	}

	// if no -> return invalid move
	return Move::getInvalidMove();
}

// get the stored eval at this board position 
std::optional<int> TranspositionTable::getStoredEval(int depth, int numPly, int alpha, int beta)
{
	// check if entry key is the board zobrist key
	Entry entry = entries[getIndex()];
	if (entry.key == board->getZobristKey() && entry.valid)
	{
		// check if the entry depth is greater or equal to this depth
		if (entry.depth >= depth)
		{
			// correct eval for mates
			int correctedEval = Score::makeMateCorrection(entry.eval, -numPly);

			// if it's an exact node, just return the node type
			if (entry.nodeType == NodeType::EXACT)
			{
				return correctedEval;
			}

			// if it's an upper bound node, only return it if it's smaller than the current upper bound
			if (entry.nodeType == NodeType::UPPER_BOUND && correctedEval <= alpha)
			{
				return correctedEval;
			}

			// if it's a lower bound node, only return it if it's greater than the current lower bound
			if (entry.nodeType == NodeType::LOWER_BOUND && correctedEval >= beta)
			{
				return correctedEval;
			}
		}
	}

	// return an empty optional if no eval has been found
	return std::optional<int>();
}

// store empty in transposition
void TranspositionTable::storeEntry(int eval, int depth, Move move, NodeType nodeType, int numPly)
{
	// only overwrite exact node if the new node is also exact
	Entry oldEntry = entries[getIndex()];
	if (oldEntry.valid && oldEntry.nodeType == NodeType::EXACT && nodeType != NodeType::EXACT)
	{
		return;
	}

	// create entry with corrected eval and store it in the array
	Entry entry = { Score::makeMateCorrection(eval, numPly), depth, move, nodeType, board->getZobristKey(), true };
	entries[getIndex()] = entry;
}

// get array index from zobrist key
int TranspositionTable::getIndex()
{
	return board->getZobristKey() % size;
}