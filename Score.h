#pragma once
#include <cstdlib>

// scores for different events
enum Scores
{
	DRAW_SCORE = 0,
	MATE_SCORE = 100000,
	LOWEST_SCORE = -1000000,
	HIGHEST_SCORE = 1000000
};

// class for handling scores
class Score
{
public:
	// return if score is mate
	static bool isMateScore(int score)
	{
		return std::abs(score) > MATE_SCORE - 1000;
	}

	// remove the depth aspect of a mate score
	static int makeMateCorrection(int score, int numPly)
	{
		int sign = score > 0 ? 1 : -1;

		if (isMateScore(score))
		{
			return score + numPly * sign;
		}

		return score;
	}
};