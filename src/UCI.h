// import relevent libraries
#include "AI.h"
#include "Board.h"

// class for uci handling
class UCI
{
private:
	// the chess board, ai
	Board board;
	AI ai;

	// uci commands
	void uciSetOption(std::string input);
	void uciPosition(std::string input);
	void uciGo(std::string input);

	// performance tests
	void runPerft(int depth, bool divide);
	long long tree(int depth, bool divide);

public:
	// constructor, main function
	UCI(std::vector<int> argsPar);
	int execute();
};