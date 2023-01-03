#include "TranspositionTable.h"

// create the entries array and initialize board variable
TranspositionTable::TranspositionTable(Board& boardPar) : board(boardPar), entries(std::vector<Entry>(size))
{
	clear();
}

// clear all entries
void TranspositionTable::clear()
{
	for (int i = 0; i < size; i++)
	{
		entries[i].valid = false;
	}
}

// set size in megabytes
void TranspositionTable::setSizeMB(int sizeMB)
{
	// calculate entry count based on 16 bytes per entry
	size = sizeMB * 1000000 / 16;

	// resize entries vector
	entries.resize(size);
	entries.shrink_to_fit();
	clear();
}

// store empty in transposition
void TranspositionTable::storeEntry(int eval, int depth, Move move, int nodeType, int numPly)
{
	// only overwrite exact node if the new node is also exact
	Entry oldEntry = entries[getIndex()];
	if (oldEntry.valid && oldEntry.nodeType == EXACT_NODE && nodeType != EXACT_NODE)
	{
		return;
	}

	// create entry with corrected eval
	Entry entry = { board.getZobristKey(), Score::makeMateCorrection(eval, numPly), (unsigned int)std::min(depth, 255), (unsigned int)move.from, (unsigned int)move.to, (unsigned int)(move.promotion + 1), (unsigned int)nodeType, (unsigned int)true };

	// mark move as null by setting promotion to 15 (an unused value)
	if (Move::isNull(move))
	{
		entry.promotion = 15;
	}

	// store entry in array
	entries[getIndex()] = entry;
}

// get the stored move at this board position, parameter exact for whether node should be PV-node
std::optional<Move> TranspositionTable::getStoredMove(Board& board, bool exact)
{
	// check if entry key is the board zobrist key
	Entry entry = entries[getIndex()]; 
	if (entry.key == board.getZobristKey() && entry.valid && (!exact || entry.nodeType == EXACT_NODE))
	{
		// if yes -> return the move
		Move move = Move::loadFromSquares(entry.from, entry.to, board.getPiecesMB());
		move.promotion = (int)entry.promotion - 1;

		// null moves are stored with 15 as promotion (an unused value)
		if (entry.promotion == 15)
		{
			move = Move::nullmove();
		}

		return move;
	}

	// if no -> return nothing
	return std::optional<Move>();
}

// get the stored eval at this board position 
std::optional<int> TranspositionTable::getStoredEval(int depth, int numPly, int alpha, int beta)
{
	// check if entry key is the board zobrist key
	Entry entry = entries[getIndex()];
	if (entry.key == board.getZobristKey() && entry.valid)
	{
		// check if the position has been searched to a greater or equal depth than required
		if (entry.depth >= (unsigned int)depth)
		{
			// correct eval for mates
			int correctedEval = Score::makeMateCorrection(entry.eval, -numPly);

			// if it's an exact node, just return the eval
			if (entry.nodeType == EXACT_NODE)
			{
				// fail-hard, returned score has to be clamped  between alpha and beta
				return std::min(std::max(correctedEval, alpha), beta);
			}

			// if it's an upper bound node, only return it if it's smaller than the current lower bound
			if (entry.nodeType == UPPER_BOUND_NODE && correctedEval <= alpha)
			{
				return alpha;
			}

			// if it's a lower bound node, only return it if it's greater than the current upper bound
			if (entry.nodeType == LOWER_BOUND_NODE && correctedEval >= beta)
			{
				return beta;
			}
		}
	}

	// return an empty optional if no eval has been found
	return std::optional<int>();
}

// get array index from zobrist key
int TranspositionTable::getIndex()
{
	return board.getZobristKey() % size;
}