#include "Score.h"

// return if score is mate
bool Score::isMateScore(int score)
{
	return std::abs(score) > MATE_SCORE - 1000;
}

// return a mate in ply score for the side to play
int Score::getPositiveMate(int ply)
{
	return MATE_SCORE - ply;
}

// return a mate in ply score for the opponent
int Score::getNegativeMate(int ply)
{
	return -(MATE_SCORE - ply);
}

// return ply to mate
int Score::getMatePly(int score)
{
	if (isMateScore(score))
	{
		return MATE_SCORE - std::abs(score);
	}
	else
	{
		return -1;
	}
}

// remove the depth aspect of a mate score
int Score::makeMateCorrection(int score, int numPly)
{
	int sign = score > 0 ? 1 : -1;

	if (isMateScore(score))
	{
		return score + numPly * sign;
	}

	return score;
}

// convert score to uci string
std::string Score::toString(int score)
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
		int mateIn = (int)std::ceil(getMatePly(score) / 2.0);

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