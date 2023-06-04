#pragma once

#include <optional>
#include <cmath>
#include "Board.h"
#include "TranspositionTable.h"
#include "PieceSquareTables.h"


class Evaluation
{
	Board& board;
	TranspositionTable& tt;

	std::map<int, int> pieceValues;
	PieceSquareTables pieceSquareTables;

	// pawn shield bitboards for each color and wing
	std::array<std::array<U64, 2>, 2> pawnShieldBBs;
	std::array<U64, 64> nearKingSquares;

	std::array<U64, 8> fileBBs;

	// evaluation terms
	double oldEndgameWeight;
	std::array<int, 2> material;
	int whitePieceSquareEval;
	std::stack<std::array<int, 2>> materialHistory;
	std::stack<int> whitePieceSquareHistory;
	std::stack<double> endgameWeightHistory;

public:
	Evaluation(Board& boardPar, TranspositionTable& ttPar);

	// initial eval in new position, actions to change eval
	void reloadEval();
	void makeMove(Move move);
	void unmakeMove(Move move);

	void orderMoves(std::vector<Move>& moves);

	// evaluation helper functions
	std::array<int, 2> countMaterial(std::array<PieceList, 12>& pieceLists);
	double getOpeningWeight();
	double getEndgameWeight(std::array<int, 2> material);

	// part evaluation functions
	int countPieceSquareEval(std::array<PieceList, 12>& pieceLists, int color, double endgameWeight);
	int countMopUpEval(std::array<PieceList, 12>& pieceLists, int materialEval, double endgameWeight);
	int countKnightPawnPenalty(std::array<PieceList, 12>& pieceLists, int color);
	int countBadBishopPenalty(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color);
	int countBishopPairReward(std::array<PieceList, 12>& pieceLists, int color);
	int countRookOpenFileReward(std::array<U64, 12>& piecesBB, int color);
	int countDoubledPawnPenalty(std::array<U64, 12>& piecesBB, int color);
	int countIsolatedPawnPenalty(std::array<U64, 12>& piecesBB, int color);
	int countPassedPawnReward(std::array<U64, 12>& piecesBB, int color);
	int countBackwardPawnPenalty(std::array<U64, 12>& piecesBB, int color);
	int countPawnShieldEval(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color, double openingWeight, double endgameWeight);
	int countPawnStormEval(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color, double endgameWeight);

	// main evaluation function
	int evaluate();
};