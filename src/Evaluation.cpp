#include "Evaluation.h"

Evaluation::Evaluation(Board& boardPar, TranspositionTable& ttPar, std::map<std::string, double>& paramsPar) : board(boardPar), tt(ttPar), params(paramsPar)
{
	// map for converting piece ids to value
	pieceValues = {
		{KING, 1000},
		{QUEEN, (int)params["queen_value"]},
		{BISHOP, (int)params["bishop_value"]},
		{KNIGHT, (int)params["knight_value"]},
		{ROOK, (int)params["rook_value"]},
		{PAWN, (int)params["pawn_value"]},
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

// update parameters after change
void Evaluation::updateParams()
{
	// map for converting piece ids to value
	pieceValues = {
		{KING, 1000},
		{QUEEN, (int)params["queen_value"]},
		{BISHOP, (int)params["bishop_value"]},
		{KNIGHT, (int)params["knight_value"]},
		{ROOK, (int)params["rook_value"]},
		{PAWN, (int)params["pawn_value"]},
		{EMPTY, 0}
	};
}

// order list of moves from best to worst
void Evaluation::orderMoves(std::vector<Move>& moves)
{
	// save turn color and move saved in tt
	std::optional<Move> ttMove = tt.getStoredMove(board, false);
	int color = board.getTurnColor();

	// create map of enemy pawn attacks
	U64 pawnAttacks = BB::pawnAnyAttacks(board.getPiecesBB()[PAWN + !color], !color);

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
		if (ttMove.has_value() && move == *ttMove)
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

// count material of both colors
std::array<int, 2> Evaluation::countMaterial(std::array<PieceList, 12>& pieceLists)
{
	std::array<int, 2> material = { 0, 0 };
	for (int i = 0; i < 12; i++)
	{
		// add piece count times piece value
		if (Piece::typeOf(i) != KING)
		{
			material[Piece::colorOf(i)] += pieceLists[i].getCount() * pieceValues.at(Piece::typeOf(i));
		}
	}

	return material;
}

// get opening weight based on amount of moves in game
double Evaluation::getOpeningWeight()
{
	return 1 - std::min(1.0, (board.getMoveCount() - 1) / 10.0);
}

// get endgame weight based on material left
double Evaluation::getEndgameWeight(std::array<int, 2> material)
{
	return 1 - std::min(1.0, (material[WHITE] + material[BLACK]) / 3200.0);
}

// calculate piece square eval
int Evaluation::countPieceSquareEval(std::array<PieceList, 12>& pieceLists, int color, double endgameWeight)
{
	// add up all piece square scores of ally pieces and substract scores of enemy pieces
	int pieceSquareEval = 0;
	for (int i = 0; i < 12; i++)
	{
		PieceList pieceList = pieceLists[i];

		for (int j = 0; j < pieceList.getCount(); j++)
		{
			pieceSquareEval += (int)(pieceSquareTables.getScore(i, pieceList[j], endgameWeight) * (Piece::colorOf(i) == color ? 1 : -1) * params["piece_square_factor"]);
		}
	}

	return pieceSquareEval;
}

// calculate mop up eval
int Evaluation::countMopUpEval(std::array<PieceList, 12>& pieceLists, int materialEval, double endgameWeight)
{
	// get squares of white and black king, calculate their distance
	int whiteKing = pieceLists[WHITE + KING][0];
	int blackKing = pieceLists[BLACK + KING][0];
	int closeness = 14 - (std::abs(Square::fileOf(whiteKing) - Square::fileOf(blackKing)) + std::abs(Square::rankOf(whiteKing) - Square::rankOf(blackKing)));
	int mopUpEval = 0;

	// if the current color has a big lead, award close kings
	return (int)(closeness * endgameWeight * std::tanh(((double)materialEval) / 200) * params["mop_up_square"]);
}

// apply a penalty for knights in open positions
int Evaluation::countKnightPawnPenalty(std::array<PieceList, 12>& pieceLists, int color)
{
	std::array<int, 2> knightPawnPenalty = { 0, 0 };
	int pawnCount = pieceLists[WHITE + PAWN].getCount() + pieceLists[BLACK + PAWN].getCount();
	for (int col = 0; col < 2; col++)
	{
		knightPawnPenalty[col] = (int)(pieceLists[col + KNIGHT].getCount() * (16 - pawnCount) * params["knight_pawn"]);
	}
	return -(knightPawnPenalty[color] - knightPawnPenalty[!color]);
}

// apply a penalty for bishops on the same color as many pawns
int Evaluation::countBadBishopPenalty(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color)
{
	std::array<int, 2> badBishopPenalty = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		for (int i = 0; i < pieceLists[col + BISHOP].getCount(); i++)
		{
			U64 sameColorBB = Square::isLight(pieceLists[col + BISHOP][i]) ? 0xAA55AA55AA55AA55 : 0x55AA55AA55AA55AA;
			badBishopPenalty[col] += (int)((BB::popCount(piecesBB[col + PAWN] & sameColorBB) - 4) * params["bad_bishop"]);
		}
	}
	return -(badBishopPenalty[color] - badBishopPenalty[!color]);
}

// apply a reward for bishop pairs
int Evaluation::countBishopPairReward(std::array<PieceList, 12>& pieceLists, int color)
{
	std::array<int, 2> bishopPairReward = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		bishopPairReward[col] = pieceLists[col + BISHOP].getCount() >= 2 ? (int)params["bishop_pair"] : 0;
	}
	return bishopPairReward[color] - bishopPairReward[!color];
}

// apply a reward for rooks standing on open files
int Evaluation::countRookOpenFileReward(std::array<U64, 12>& piecesBB, int color)
{
	std::array<int, 2> openFileReward = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		// any pawns that are north of ally pawns are counted

		U64 openFiles = ~BB::fileFill(piecesBB[col + PAWN] | piecesBB[!col + PAWN]);
		openFileReward[col] = (int)(BB::popCount(piecesBB[col + ROOK] & openFiles) * params["rook_open_file"]);
	}
	return openFileReward[color] - openFileReward[!color];
}

// apply a penalty for every doubled pawn
int Evaluation::countDoubledPawnPenalty(std::array<U64, 12>& piecesBB, int color)
{
	std::array<int, 2> doubledPawnPenalty = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		// any pawns that are north of ally pawns are counted
		U64 doubledPawns = piecesBB[col + PAWN] & BB::dirFill(piecesBB[col + PAWN], NORTH, true);
		doubledPawnPenalty[col] = (int)(BB::popCount(doubledPawns) * params["doubled_pawn"]);
	}
	return -(doubledPawnPenalty[color] - doubledPawnPenalty[!color]);
}

// apply a penalty for isolated pawns
int Evaluation::countIsolatedPawnPenalty(std::array<U64, 12>& piecesBB, int color)
{
	std::array<int, 2> isolatedPawnPenalty = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		// pawns that are not on the sides of any ally pawns are counted
		U64 isolatedPawns = piecesBB[col + PAWN] & ~BB::fileFill(BB::shiftTwo(piecesBB[col + PAWN], WEST)) & ~BB::fileFill(BB::shiftTwo(piecesBB[col + PAWN], EAST));
		isolatedPawnPenalty[col] = (int)(BB::popCount(isolatedPawns) * params["isolated_pawn"]);
	}
	return -(isolatedPawnPenalty[color] - isolatedPawnPenalty[!color]);
}

// apply a reward for passed pawns
int Evaluation::countPassedPawnReward(std::array<U64, 12>& piecesBB, int color)
{
	std::array<int, 2> passedPawnReward = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		// pawns that are not in the front spans of enemy pawns are counted
		U64 allFrontSpans = BB::dirFill(piecesBB[!col + PAWN], col == WHITE ? SOUTH : NORTH, true);
		allFrontSpans |= BB::shiftTwo(allFrontSpans, WEST) | BB::shiftTwo(allFrontSpans, EAST);
		U64 passedPawns = piecesBB[col + PAWN] & ~allFrontSpans;
		passedPawnReward[col] = (int)(BB::popCount(passedPawns) * params["passed_pawn"]);
	}
	return passedPawnReward[color] - passedPawnReward[!color];
}

// apply a penalty for backward pawns
int Evaluation::countBackwardPawnPenalty(std::array<U64, 12>& piecesBB, int color)
{
	std::array<int, 2> backwardPawnPenalty = { 0, 0 };
	for (int col = 0; col < 2; col++)
	{
		// pawns that aren't backed by any ally pawns but can't pass due to enemy pawn attacks are counted
		U64 stops = BB::shiftTwo(piecesBB[col + PAWN], col == WHITE ? NORTH : SOUTH);
		U64 frontSpans = BB::dirFill(piecesBB[col + PAWN], col == WHITE ? NORTH : SOUTH, true);
		U64 attackSpans = BB::shiftTwo(frontSpans, WEST) | BB::shiftTwo(frontSpans, EAST);
		U64 enemyAttacks = BB::pawnAnyAttacks(piecesBB[!col + PAWN], !col);
		U64 backwardPawnStops = stops & ~attackSpans & enemyAttacks;
		backwardPawnPenalty[col] = (int)(BB::popCount(backwardPawnStops) * params["backward_pawn"]);
	}
	return -(backwardPawnPenalty[color] - backwardPawnPenalty[!color]);
}

// apply a reward for pawn shields
int Evaluation::countPawnShieldEval(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color, double openingWeight, double endgameWeight)
{
	// set up relevant variables
	int allyKingFile = Square::fileOf(pieceLists[color + KING][0]);
	int enemyKingFile = Square::fileOf(pieceLists[!color + KING][0]);
	int allyKingWing = allyKingFile / 4;
	int enemyKingWing = enemyKingFile / 4;
	bool allyKingInMiddle = allyKingFile > 2 && allyKingFile < 5;
	bool enemyKingInMiddle = enemyKingFile > 2 && enemyKingFile < 5;

	// calculate number of pawns shielding the kings
	int allyPawnShield = BB::popCount(pawnShieldBBs[color][allyKingWing] & piecesBB[color + PAWN]);
	int enemyPawnShield = BB::popCount(pawnShieldBBs[!color][enemyKingWing] & piecesBB[!color + PAWN]);

	// pawn shield eval should apply in the middle game when the king is on the side
	int pawnShieldEval = (int)((allyPawnShield - enemyPawnShield) * (allyKingInMiddle ? 0.5 : 1) * (enemyKingInMiddle ? 0.5 : 1) * params["pawn_shield"] * (1 - openingWeight) * std::max(0.0, 1 - endgameWeight * 1.5));
	return pawnShieldEval;
}

// apply a penalty for enemy pawn storms
int Evaluation::countPawnStormEval(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color, double endgameWeight)
{
	// count how many enemy pawns are near the kings
	int allyPawnStorm = BB::popCount(nearKingSquares[pieceLists[color + KING][0]] & piecesBB[!color + PAWN]);
	int enemyPawnStorm = BB::popCount(nearKingSquares[pieceLists[!color + KING][0]] & piecesBB[color + PAWN]);
	int pawnStormEval = (int)((enemyPawnStorm - allyPawnStorm) * params["pawn_storm"] * std::max(0.0, 1 - endgameWeight * 1.5));
	return pawnStormEval;
}

// main evaluation function
int Evaluation::evaluate()
{
	// save turn color and piece lists
	int color = board.getTurnColor();
	std::array<PieceList, 12> pieceLists = board.getPieceLists();
	std::array<U64, 12> piecesBB = board.getPiecesBB();

	// calculate helper functions
	std::array<int, 2> material = countMaterial(pieceLists);
	double openingWeight = getOpeningWeight();
	double endgameWeight = getEndgameWeight(material);

	// calculate eval parts
	int materialEval = material[color] - material[!color];
	int pieceSquareEval = countPieceSquareEval(pieceLists, color, endgameWeight);
	int mopUpEval = countMopUpEval(pieceLists, materialEval, endgameWeight);
	int knightPawnPenalty = countKnightPawnPenalty(pieceLists, color);
	int badBishopPenalty = countBadBishopPenalty(pieceLists, piecesBB, color);
	int bishopPairReward = countBishopPairReward(pieceLists, color);
	int rookOpenFileReward = countRookOpenFileReward(piecesBB, color);
	int doubledPawnPenalty = countDoubledPawnPenalty(piecesBB, color);
	int isolatedPawnPenalty = countIsolatedPawnPenalty(piecesBB, color);
	int passedPawnReward = countPassedPawnReward(piecesBB, color);
	int backwardPawnPenalty = countBackwardPawnPenalty(piecesBB, color);
	int pawnShieldEval = countPawnShieldEval(pieceLists, piecesBB, color, openingWeight, endgameWeight);
	int pawnStormEval = countPawnStormEval(pieceLists, piecesBB, color, endgameWeight);

	// return sum of eval parts
	return materialEval + pieceSquareEval + mopUpEval + knightPawnPenalty + badBishopPenalty + bishopPairReward + rookOpenFileReward + doubledPawnPenalty + isolatedPawnPenalty + passedPawnReward + backwardPawnPenalty + pawnShieldEval + pawnStormEval;
}