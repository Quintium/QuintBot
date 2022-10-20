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

	// convert score to uci string
	static std::string toString(int score)
	{
		// check if it's the lower or upper bound
		if (score == LOWEST_SCORE)
		{
			return std::string("lowerbound");
		}
		else if (score == HIGHEST_SCORE)
		{
			return std::string("upperbound");
		}
		// check if it's a mate in x
		else if (isMateScore(score))
		{
			// calculate number of moves until mate
			int mateIn = (int)std::ceil((MATE_SCORE - std::abs(score)) / 2.0);

			// return mate information
			if (score > 0)
			{
				return std::string("mate ") + std::to_string(mateIn);
			}
			else
			{
				return std::string("mate ") + std::to_string(-mateIn);
			}
		}
		else
		{
			// return search score in centipawns
			return std::string("cp ") + std::to_string(score);
		}
	}
};