// import relevent libraries
#include "Engine.h"
#include "Board.h"

// class for uci handling
class UCI
{
private:
	// the chess engine
	Engine engine;

	// uci commands
	void uciSetOption(std::string input);
	void uciPosition(std::string input);
	void uciGo(std::string input);

	// performance tests
	void runPerft(int depth, bool divide);
	long long tree(int depth, bool divide);

public:
	// constructor, main function
	UCI();
	int execute();
};