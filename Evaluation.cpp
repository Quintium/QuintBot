#include "Evaluation.h"

Evaluation::Evaluation(Board* boardVar)
{
	board = boardVar;

	// bitboards for pawn shields for each color and wing
	pawnShieldBBs[0][0] = 0x0007070000000000;
	pawnShieldBBs[1][0] = 0x0000000000070700;
	pawnShieldBBs[0][1] = 0x00e0e00000000000;
	pawnShieldBBs[1][1] = 0x0000000000e0e000;

	for (int i = 0; i < 64; i++)
	{
		U64 square = 0;

		for (int j = 0; j < 64; j++)
		{
			if (std::abs(i / 8 - j / 8) <= 4 && std::abs(i % 8 - j % 8) <= 2)
			{
				square |= U64(1) << j;
			}
		}

		nearKingSquares[i] = square;
	}
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

		move.score = -move.score;

		// if this was the best move in the transposition table with a lower depth, examine it first
		if (move == tt->getStoredMove())
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
	// save turn color and piecesBB
	int color = board->getTurnColor();
	U64* piecesBB = board->getPiecesBB();

	// count material of both colors
	PieceList* pieceLists = board->getPieceLists();
	int material[2] = { 0, 0 };
	for (int i = 0; i < 12; i++)
	{
		material[Piece::colorOf(i)] += pieceLists[i].getCount() * Piece::valueOf(i);
	}

	// save piece advantage
	int materialEval = material[color] - material[!color];

	/*
	int pieceEval = 0;

	int knightPawnPenalty[2] = { 0, 0 };
	int pawnCount = pieceLists[WHITE + PAWN].getCount() + pieceLists[BLACK + PAWN].getCount();
	for (int col = 0; col < 2; col++)
	{
		knightPawnPenalty[col] = pieceLists[col + KNIGHT].getCount() * (8 - pawnCount) * 5;
	}
	pieceEval -= knightPawnPenalty[color] - knightPawnPenalty[!color];

	int badBishopPenalty[2] = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		for (int i = 0; i < pieceLists[col + BISHOP].getCount(); i++)
		{
			U64 sameColorBB = Square::isLight(pieceLists[col + BISHOP][i]) ? 0xAA55AA55AA55AA55 : 0x55AA55AA55AA55AA;
			badBishopPenalty[col] += (int)__popcnt64(piecesBB[col + PAWN] & sameColorBB) * 10;
		}
	}
	pieceEval -= badBishopPenalty[color] - badBishopPenalty[!color];

	int bishopPairReward[2] = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		bishopPairReward[col] += pieceLists[col + BISHOP].getCount() == 2 ? 50 : 0;
	}
	pieceEval += bishopPairReward[color] - bishopPairReward[!color];*/

	// calculate game phase weight
	double openingWeight = 1 - std::min(1.0, board->getMoveCount() / 10.0);
	double endgameWeight = 1 - std::min(1.0, (material[color] + material[!color]) / 3200.0);

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
	if (materialEval > 100)
	{
		mopUpEval = (int)(closeness * endgameWeight * 4);
	}
	// if the other color has a big lead, award far kings
	else if (materialEval < -100)
	{
		mopUpEval = (int)(closeness * endgameWeight * -4);
	}

	/*
	int allyKingWing = pieceLists[color + KING][0] % 8 / 4;
	int enemyKingWing = pieceLists[!color + KING][0] % 8 / 4;
	bool allyKingInMiddle = (piecesBB[color + KING] & middleFiles) > 0;
	bool enemyKingInMiddle = (piecesBB[!color + KING] & middleFiles) > 0;

	int allyPawnShield = (int)__popcnt64(pawnShieldBBs[color][allyKingWing] & piecesBB[color + PAWN]);
	int enemyPawnShield = (int)__popcnt64(pawnShieldBBs[!color][enemyKingWing] & piecesBB[!color + PAWN]);
	int pawnShieldEval = (allyPawnShield - enemyPawnShield) * (allyKingInMiddle ? 0.5 : 1) * (enemyKingInMiddle ? 0.5 : 1) * 50 * std::max(1 - 2 * endgameWeight, 0.0) * (1 - openingWeight);

	int allyPawnStorm = (int)__popcnt64(nearKingSquares[pieceLists[color + KING][0]] & piecesBB[!color + PAWN]);
	int enemyPawnStorm = (int)__popcnt64(nearKingSquares[pieceLists[!color + KING][0]] & piecesBB[color + PAWN]);
	int pawnStormEval = (enemyPawnStorm - allyPawnStorm) * 25;
	//std::cout << "Pawn storm eval: " << pawnStormEval << "\n";

	int kingEval = pawnShieldEval + pawnStormEval;*/

	// return sum of different evals
	return -(materialEval + pieceSquareEval + mopUpEval);
}