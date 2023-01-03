#include "UCI.h"

// main function
int main(int argc, char* argv[])
{
	UCI uci;
	return uci.execute();
}

// game constructor
UCI::UCI() : ai(AI(board))
{
	// load board position
	board.loadStartPosition();
}

// main function
int UCI::execute()
{
	bool running = true;
	while (running)
	{
		// get input
		std::string input;
		std::getline(std::cin, input);

		// if input is "uci", output all id and option information
		if (input == "uci")
		{
			std::cout << "id name QuintBot\n";
			std::cout << "id author Quintium\n";

			std::cout << "option name Hash type spin default 64 min 1 max 32000\n";
			std::cout << "option name OwnBook type check default true\n";
			std::cout << "option name Move Overhead type spin default 10 min 0 max 10000\n";

			std::cout << "uciok\n";
		}

		// respond to "isready" with "readyok"
		if (input == "isready")
		{
			std::cout << "readyok\n";
		}

		// set up new game
		if (input == "ucinewgame")
		{
			ai.newGame();
		}

		// exit program on command
		if (input == "quit")
		{
			return true;
		}

		// if input starts with setoption
		if (input.rfind("setoption name", 0) == 0)
		{
			uciSetOption(input);
		}

		// if input starts with "position", set up position
		if (input.rfind("position", 0) == 0)
		{
			uciPosition(input);
		}

		// if input starts with "go", find best move
		if (input.rfind("go", 0) == 0)
		{
			uciGo(input);
		}

		// speed test on position with depth 8
		if (input == "speed test")
		{
			ai.newGame();
			board.loadFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 15");
			ai.getBestMove(-1, 0, 8, -1);
		}

		// evaluation for debugging reasons
		if (input == "eval")
		{
			int eval = ai.evaluate();
			std::cout << "Evaluation: " << eval << "\n";
		}

		// print out fen of board for debugging reasons
		if (input == "fen")
		{
			std::cout << "Fen: " << board.getFen() << "\n";
		}
	}

	return 0;
}

// handle setoption uci command
void UCI::uciSetOption(std::string input)
{
	// strip off the "setoption name "
	input = input.substr(15);

	// find position of "value" in the input
	size_t valuePos = input.find(" value ");

	// get option name and value
	std::string optionName = input.substr(0, valuePos);
	std::string optionValue = input.substr(valuePos + 7);

	// set hash option
	if (optionName == "Hash")
	{
		ai.setHash(std::stoi(optionValue));
	}

	// set own book option
	if (optionName == "OwnBook")
	{
		ai.setOwnBook(optionValue == "true");
	}

	// set move overhead option
	if (optionName == "Move Overhead")
	{
		ai.setMoveOverhead(std::stoi(optionValue));
	}
}

// handle position uci command
void UCI::uciPosition(std::string input)
{
	// strip off the "position "
	input = input.substr(9);

	// find position of "moves" in the input
	size_t movePos = input.find(" moves ");

	// if input starts with "fen", load fen
	if (input.rfind("fen", 0) == 0)
	{
		if (movePos != std::string::npos)
		{
			board.loadFromFen(input.substr(4, movePos - 4));
		}
		else
		{
			board.loadFromFen(input.substr(4));
		}
	}
	// if input starts with "startpos", load start position
	if (input.rfind("startpos", 0) == 0)
	{
		board.loadStartPosition();
	}

	// if "moves" was found
	if (movePos != std::string::npos)
	{
		// place move pos after " moves "
		movePos += 7;

		// loop through all moves
		while (movePos < input.size())
		{
			// find next space
			size_t nextSpace = input.find(" ", movePos);
			if (nextSpace == std::string::npos)
			{
				nextSpace = input.size();
			}

			// get move until the next space and make that move
			std::string moveStr = input.substr(movePos, nextSpace - movePos);
			board.makeMove(Move::loadFromNotation(moveStr, board.getPiecesMB()));
			movePos = nextSpace + 1;
		}
	}
}

// handle go uci command
void UCI::uciGo(std::string input)
{
	size_t index;

	index = input.find("perft");
	if (index != std::string::npos)
	{
		index += 6;
		size_t spaceIndex = input.find(" ", index);
		int perftDepth = std::stoi(input.substr(index, spaceIndex - index));
		runPerft(perftDepth, false);
	}
	else
	{
		// default time and increment values
		int timeLeft = -1, increment = 0, exactTime = -1;

		// check for exact time left
		index = input.find("movetime");
		if (index != std::string::npos)
		{
			index += 9;
			size_t spaceIndex = input.find(" ", index);
			exactTime = std::stoi(input.substr(index, spaceIndex - index));
		}

		// strings for time and increment
		std::string timeString = board.getTurnColor() == WHITE ? "wtime" : "btime";
		std::string incString = board.getTurnColor() == WHITE ? "winc" : "binc";

		// find "wtime" or "btime" and set time to value after it
		index = input.find(timeString);
		if (index != std::string::npos)
		{
			index += 6;
			size_t spaceIndex = input.find(" ", index);
			timeLeft = std::stoi(input.substr(index, spaceIndex - index));
		}

		// find "winc" or "binc" and set increment to value after it
		index = input.find(incString);
		if (index != std::string::npos)
		{
			index += 5;
			size_t spaceIndex = input.find(" ", index);
			increment = std::stoi(input.substr(index, spaceIndex - index));
		}

		// find "depth" and set depth to value after it
		int depth = -1;
		index = input.find("depth");
		if (index != std::string::npos)
		{
			index += 6;
			size_t spaceIndex = input.find(" ", index);
			depth = std::stoi(input.substr(index, spaceIndex - index));
		}

		// get the best move and print it out
		Move move = ai.getBestMove(timeLeft, increment, depth, exactTime);
		std::cout << "bestmove " << move.getNotation() << "\n";
	}
}

// run performance test
void UCI::runPerft(int depth, bool divide)
{
	// save the time at the start
	auto start = std::chrono::system_clock::now();

	// calculate the nodes searched at given depth
	long long nodes = tree(depth, divide);

	// save end time and calculate time Passed
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	double timePassed = diff.count();

	// print out perft stats
	std::cout << "Depth " << depth << ": " << nodes << " nodes searched.\n";
	std::cout << "Time needed: " << timePassed << "s\n";
	std::cout << "Nodes per second: " << nodes / timePassed << "\n";
}

// calculate game tree for performance test
long long UCI::tree(int depth, bool divide)
{
	long long nodes = 0;

	// generate moves and save them
	board.generateMoves();
	std::vector<Move> currentMoveList = board.getMoveList();

	// return the number of moves if depth is 1
	if (depth == 1)
	{
		return (int)currentMoveList.size();
	}

	// loop through moves
	for (const Move& move : currentMoveList)
	{
		// make the move and calculate the nodes after this position with a lower depth
		board.makeMove(move);
		long long change = tree(depth - 1, false);

		// print out number of nodes after each position if divide argument is true
		if (divide)
		{
			std::cout << Square::toString(move.from) + Square::toString(move.to) << ": " << change << "\n";
		}

		// add change to the nodes count and unmake move
		nodes += change;
		board.unmakeMove(move);
	}

	// return the number of nodes
	return nodes;
}