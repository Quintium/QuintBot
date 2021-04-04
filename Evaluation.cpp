#include "Evaluation.h"

Evaluation::Evaluation(Board* boardVar)
{
	board = boardVar;
}

// order list of moves from best to worst
void Evaluation::orderMoves(std::vector<Move>& moves, TranspositionTable* tt)
{
	// save turn color
	int color = board->getTurnColor();

	// create map of enemy pawn attacks
	U64 pawnAttacks = BB::pawnAnyAttacks(board->getPiecesBB()[PAWN + !color], !color);

	// loop through all moves
	std::vector<Move> newMoves;
	for (Move& move : moves)
	{
		move.score = 0;

		// if there's a capture award more valuable captured piece and less valuable moved piece
		if (move.cPiece != EMPTY)
		{
			move.score += 10 * Piece::valueOf(move.cPiece) - Piece::valueOf(move.piece);
		}

		// award promotion with value of promotion piece
		if (move.promotion != EMPTY)
		{
			move.score += Piece::valueOf(move.promotion);
		}

		// if enemy pawn could take piece, penalize a more valuable piece
		if ((pawnAttacks & (U64(1) << move.to)) > 0)
		{
			move.score -= Piece::valueOf(move.piece);
		}

		// if this was the best move in the transposition table with a lower depth, examine it first
		if (tt != nullptr && (move == tt->getStoredMove()))
		{
			move.score = 10000;
		}

		// insert it in the right place in the sorted array
		size_t i;
		for (i = 0; (i < newMoves.size()) && (move.score < newMoves[i].score); i++);
		newMoves.insert(newMoves.begin() + i, move);
	}

	// replace the original array with the sorted array
	moves = newMoves;
}

int Evaluation::evaluate()
{
	// save turn color
	int color = board->getTurnColor();

	// count material of both colors
	PieceList* pieceLists = board->getPieceLists();
	int material[2] = { 0, 0 };
	for (int i = 0; i < 12; i++)
	{
		material[Piece::colorOf(i)] += pieceLists[i].getCount() * Piece::valueOf(i);
	}

	// save piece advantage
	int pieceEval = material[color] - material[!color];

	// calculate endgame weight, endgame weight starts rising after both sides have less material than two queens, two rooks and four pawns
	float endgameWeight = 1 - std::min(1.0f, (material[color] + material[!color]) / 3200.0f);

	// add all piece square scores of ally pieces and substract scores of enemy pieces
	int pieceSquareEval = 0;
	for (int i = 0; i < 12; i++)
	{
		PieceList pieceList = pieceLists[i];

		for (int j = 0; j < pieceList.getCount(); j++)
		{
			pieceSquareEval += pieceSquareTables.getScore(i, pieceList[j], endgameWeight) * (Piece::colorOf(i) == color ? 1 : -1);
		}
	}

	// get squares of white and black king, calculate their distance
	int whiteKing = pieceLists[WHITE + KING][0];
	int blackKing = pieceLists[BLACK + KING][0];
	int closeness = 14 - (std::abs(Square::fileOf(whiteKing) - Square::fileOf(blackKing)) + std::abs(Square::rankOf(whiteKing) - Square::rankOf(blackKing)));
	int mopUpEval = 0;

	// if the current color has a big lead, award close kings
	if (pieceEval > 100)
	{
		mopUpEval = (int)(closeness * endgameWeight * 4);
	}
	// if the other color has a big lead, award far kings
	else if (pieceEval < -100)
	{
		mopUpEval = (int)(closeness * endgameWeight * -4);
	}

	// return sum of different evals
	return pieceEval + pieceSquareEval + mopUpEval;
}

