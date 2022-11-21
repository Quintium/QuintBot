#include "Evaluation.h"

Evaluation::Evaluation(Board* boardVar)
{
	board = boardVar;

	// map for converting piece ids to value
	pieceValues = {
		{KING, 1000},
		{QUEEN, 900},
		{BISHOP, 330},
		{KNIGHT, 320},
		{ROOK, 500},
		{PAWN, 100},
		{EMPTY, 0}
	};

	// bitboards for pawn shields for each color and wing
	pawnShieldBBs[0][0] = 0x0007070000000000;
	pawnShieldBBs[1][0] = 0x0000000000070700;
	pawnShieldBBs[0][1] = 0x00e0e00000000000;
	pawnShieldBBs[1][1] = 0x0000000000e0e000;

	// initialize bitboards for each file
	fileBBs[0] = 0x0101010101010101;
	fileBBs[1] = 0x0202020202020202;
	fileBBs[2] = 0x0404040404040404;
	fileBBs[3] = 0x0808080808080808;
	fileBBs[4] = 0x1010101010101010;
	fileBBs[5] = 0x2020202020202020;
	fileBBs[6] = 0x4040404040404040;
	fileBBs[7] = 0x8080808080808080;

	// calculate near square bitboards for every square
	for (int i = 0; i < 64; i++)
	{
		U64 square = 0;

		for (int j = 0; j < 64; j++)
		{
			if (std::abs(Square::rankOf(i) - Square::rankOf(j)) <= 3 && std::abs(Square::fileOf(i) - Square::fileOf(j)) <= 2)
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
			move.score += 11 * pieceValues.at(Piece::typeOf(move.cPiece)) - pieceValues.at(Piece::typeOf(move.piece));
		}

		// award promotion with value of promotion piece
		if (move.promotion != EMPTY)
		{
			move.score += pieceValues.at(Piece::typeOf(move.promotion));
		}

		// if enemy pawn could take piece, penalize a more valuable piece
		if ((pawnAttacks & (U64(1) << move.to)) > 0)
		{
			move.score -= pieceValues.at(Piece::typeOf(move.piece));
		}

		// if this was the best move in the transposition table with a lower depth, examine it first
		if (move == tt->getStoredMove(board->getPiecesMB()))
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
		if (Piece::typeOf(i) != KING)
		{
			material[Piece::colorOf(i)] += pieceLists[i].getCount() * pieceValues.at(Piece::typeOf(i));
		}
	}

	// save piece advantage
	int materialEval = material[color] - material[!color];

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

	int pieceEval = 0;

	/*
	int knightPawnPenalty[2] = {0, 0};
	int pawnCount = pieceLists[WHITE + PAWN].getCount() + pieceLists[BLACK + PAWN].getCount();
	for (int col = 0; col < 2; col++)
	{
		knightPawnPenalty[col] = pieceLists[col + KNIGHT].getCount() * (16 - pawnCount) * 3;
	}
	pieceEval -= knightPawnPenalty[color] - knightPawnPenalty[!color];

	int badBishopPenalty[2] = {0, 0};
	for (int col = 0; col < 2; col++)
	{
		for (int i = 0; i < pieceLists[col + BISHOP].getCount(); i++)
		{
			U64 sameColorBB = Square::isLight(pieceLists[col + BISHOP][i]) ? 0xAA55AA55AA55AA55 : 0x55AA55AA55AA55AA;
			badBishopPenalty[col] += (BB::popCount(piecesBB[col + PAWN] & sameColorBB) - 8) * 10;
		}
	}
	pieceEval -= badBishopPenalty[color] - badBishopPenalty[!color];

	// apply a reward for the sides with a bishop pair
	int bishopPairReward[2] = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		bishopPairReward[col] = pieceLists[col + BISHOP].getCount() >= 2 ? 20 : 0;
	}
	pieceEval += bishopPairReward[color] - bishopPairReward[!color];
	*/

	int pawnStructureEval = 0;

	/*
	// apply a penalty for every doubled pawn
	int doubledPawnPenalty[2] = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		U64 doubledPawns = piecesBB[col + PAWN] & BB::dirFill(piecesBB[col + PAWN], NORTH, true);
		doubledPawnPenalty[col] = BB::popCount(doubledPawns) * 30;
	}
	pawnStructureEval -= doubledPawnPenalty[color] - doubledPawnPenalty[!color];

	// apply a penalty for isolated pawns
	int isolatedPawnPenalty[2] = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		U64 isolatedPawns = piecesBB[col + PAWN] & ~BB::fileFill(BB::shiftTwo(piecesBB[col + PAWN], WEST)) & ~BB::fileFill(BB::shiftTwo(piecesBB[col + PAWN], EAST));
		isolatedPawnPenalty[col] = BB::popCount(isolatedPawns) * 20;
	}
	pawnStructureEval -= isolatedPawnPenalty[color] - isolatedPawnPenalty[!color];

	// apply a reward for passed pawns
	int passedPawnReward[2] = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		U64 allFrontSpans = BB::dirFill(piecesBB[!col + PAWN], col == WHITE ? SOUTH : NORTH, true);
		allFrontSpans |= BB::shiftTwo(allFrontSpans, WEST) | BB::shiftTwo(allFrontSpans, EAST);
		U64 passedPawns = piecesBB[col + PAWN] & ~allFrontSpans;
		passedPawnReward[col] = BB::popCount(passedPawns) * 20;
	}
	pawnStructureEval += passedPawnReward[color] - passedPawnReward[!color];

	// apply a penalty for backward pawns
	int backwardPawnPenalty[2] = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		U64 stops = BB::shiftTwo(piecesBB[col + PAWN], col == WHITE ? NORTH : SOUTH);
		U64 frontSpans = BB::dirFill(piecesBB[col + PAWN], col == WHITE ? NORTH : SOUTH, true);
		U64 attackSpans = BB::shiftTwo(frontSpans, WEST) | BB::shiftTwo(frontSpans, EAST);
		U64 enemyAttacks = BB::pawnAnyAttacks(piecesBB[!col + PAWN], !col);
		U64 backwardPawnStops = stops & ~attackSpans & enemyAttacks;
		backwardPawnPenalty[col] = BB::popCount(backwardPawnStops) * 20;
	}
	pawnStructureEval -= backwardPawnPenalty[color] - backwardPawnPenalty[!color];
	*/

	int kingEval = 0;

	/*
	int allyKingFile = pieceLists[color + KING][0] % 8;
	int enemyKingFile = pieceLists[!color + KING][0] % 8;
	int allyKingWing = allyKingFile / 4;
	int enemyKingWing = enemyKingFile / 4;
	bool allyKingInMiddle = allyKingFile > 2 && allyKingFile < 5;
	bool enemyKingInMiddle = enemyKingFile > 2 && enemyKingFile < 5;

	int allyPawnShield = BB::popCount(pawnShieldBBs[color][allyKingWing] & piecesBB[color + PAWN]);
	int enemyPawnShield = BB::popCount(pawnShieldBBs[!color][enemyKingWing] & piecesBB[!color + PAWN]);
	int pawnShieldEval = (allyPawnShield - enemyPawnShield) * (allyKingInMiddle ? 0.5 : 1) * (enemyKingInMiddle ? 0.5 : 1) * 50 * (1 - endgameWeight) * (1 - openingWeight);
	kingEval += pawnShieldEval;

	int allyPawnStorm = BB::popCount(nearKingSquares[pieceLists[color + KING][0]] & piecesBB[!color + PAWN]);
	int enemyPawnStorm = BB::popCount(nearKingSquares[pieceLists[!color + KING][0]] & piecesBB[color + PAWN]);
	int pawnStormEval = (enemyPawnStorm - allyPawnStorm) * 40;
	kingEval += pawnStormEval;
	*/

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

	// return sum of different evals
	return materialEval + pieceSquareEval + pieceEval + pawnStructureEval + kingEval + mopUpEval;
}