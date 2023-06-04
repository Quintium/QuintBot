#include "Engine.h"
#include "Board.h"

// class for handling UCI protocol requests
class UCI
{
private:
	Engine engine;

	// uci commands
	void uciSetOption(std::string input);
	void uciPosition(std::string input);
	void uciGo(std::string input);

	// performance test
	void runPerft(int depth, bool divide);
	long long tree(int depth, bool divide);

public:
	UCI();
	int execute();
};