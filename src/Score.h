#pragma once
#include <string>
#include <cmath>

enum Scores
{
	DRAW_SCORE = 0,
	MATE_SCORE = 100000,
	LOWEST_SCORE = -1000000,
	HIGHEST_SCORE = 1000000
};

// class for handling position scores
class Score
{
public:
	// handle mate scores
	static bool isMateScore(int score);
	static int getPositiveMate(int ply);
	static int getNegativeMate(int ply);
	static int getMatePly(int score);
	static int makeMateCorrection(int score, int numPly);

	// convert score to uci string
	static std::string toString(int score);
};