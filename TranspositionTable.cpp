#include "TranspositionTable.h"

TranspositionTable::TranspositionTable(Board* boardVar)
{
	entries.resize(size);
	board = boardVar;
}

Move TranspositionTable::getStoredMove()
{
	Entry entry = entries[getIndex()];
	if (entry.key == board->getZobristKey())
	{
		return entry.move;
	}

	return Move::getInvalidMove();
}

int TranspositionTable::getStoredEval(int depth, int numPly, int alpha, int beta)
{
	failed = false;

	Entry entry = entries[getIndex()];
	if (entry.key == board->getZobristKey())
	{
		if (entry.depth >= depth)
		{
			int correctedEval = Score::unmakeMateCorrection(entry.eval, numPly);

			if (entry.nodeType == NodeType::EXACT)
			{
				return correctedEval;
			}

			if (entry.nodeType == NodeType::UPPER_BOUND && correctedEval <= alpha)
			{
				return correctedEval;
			}

			if (entry.nodeType == NodeType::LOWER_BOUND && correctedEval >= beta)
			{
				return correctedEval;
			}
		}
	}

	failed = true;

	return 0;
}

void TranspositionTable::storeEntry(int eval, int depth, Move move, NodeType nodeType, int numPly)
{
	Entry entry = { Score::makeMateCorrection(eval, numPly), depth, move, nodeType, board->getZobristKey() };
	entries[getIndex()] = entry;
}

int TranspositionTable::getIndex()
{
	return board->getZobristKey() % size;
}

bool TranspositionTable::didSearchFail()
{
	return failed;
}