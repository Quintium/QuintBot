#include "UCI.h"

int main(int argc, char* argv[])
{
	UCI uci;
	return uci.execute();
}

UCI::UCI() : engine(Engine())
{
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

		// ready information for syncing
		if (input == "isready")
		{
			std::cout << "readyok\n";
		}

		// set up new game
		if (input == "ucinewgame")
		{
			engine.newGame();
		}

		// exit program on command
		if (input == "quit")
		{
			return true;
		}

		// set UCI option
		if (input.rfind("setoption name", 0) == 0)
		{
			uciSetOption(input);
		}

		// set up position on board
		if (input.rfind("position", 0) == 0)
		{
			uciPosition(input);
		}

		// find best move
		if (input.rfind("go", 0) == 0)
		{
			uciGo(input);
		}

		// speed test on position with depth 8
		if (input == "speed test")
		{
			engine.newGame();
			engine.loadFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 15");
			engine.getBestMove(-1, 0, 8, -1);
		}

		// evaluation for debugging reasons
		if (input == "eval")
		{
			int eval = engine.evaluate();
			std::cout << "Evaluation: " << eval << "\n";
		}

		// print out fen of board for debugging reasons
		if (input == "fen")
		{
			std::cout << "Fen: " << engine.getBoard().getFen() << "\n";
		}
	}

	return 0;
}

// handle setoption uci command
void UCI::uciSetOption(std::string input)
{
	// strip off the "setoption name "
	input = input.substr(15);

	// get option name and value
	size_t valuePos = input.find(" value ");
	std::string optionName = input.substr(0, valuePos);
	std::string optionValue = input.substr(valuePos + 7);

	if (optionName == "Hash")
	{
		engine.setHash(std::stoi(optionValue));
	}

	if (optionName == "OwnBook")
	{
		engine.setOwnBook(optionValue == "true");
	}

	if (optionName == "Move Overhead")
	{
		engine.setMoveOverhead(std::stoi(optionValue));
	}
}

// handle position uci command
void UCI::uciPosition(std::string input)
{
	// strip off the "position "
	input = input.substr(9);

	size_t movePos = input.find(" moves ");

	// if input starts with "fen", load fen
	if (input.rfind("fen", 0) == 0)
	{
		if (movePos != std::string::npos)
		{
			engine.loadFromFen(input.substr(4, movePos - 4));
		}
		else
		{
			engine.loadFromFen(input.substr(4));
		}
	}
	// if input starts with "startpos", load start position
	if (input.rfind("startpos", 0) == 0)
	{
		engine.loadStartPosition();
	}

	// if "moves" was found
	if (movePos != std::string::npos)
	{
		// place move pos after " moves "
		movePos += 7;

		// loop through all moves
		while (movePos < input.size())
		{
			size_t nextSpace = input.find(" ", movePos);
			if (nextSpace == std::string::npos)
			{
				nextSpace = input.size();
			}

			// get move until the next space and make that move
			std::string moveStr = input.substr(movePos, nextSpace - movePos);
			engine.makeMove(Move::loadFromNotation(moveStr, engine.getBoard().getPiecesMB()));
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
		// execute perft
		index += 6;
		size_t spaceIndex = input.find(" ", index);
		int perftDepth = std::stoi(input.substr(index, spaceIndex - index));
		runPerft(perftDepth, true);
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

		// strings for time and increment based on turn color
		std::string timeString = engine.getBoard().getTurnColor() == WHITE ? "wtime" : "btime";
		std::string incString = engine.getBoard().getTurnColor() == WHITE ? "winc" : "binc";

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
		Move move = engine.getBestMove(timeLeft, increment, depth, exactTime);
		std::cout << "bestmove " << move.getNotation() << "\n";
	}
}

// run performance test
void UCI::runPerft(int depth, bool divide)
{
	auto start = std::chrono::system_clock::now();

	// calculate the nodes searched at given depth
	long long nodes = tree(depth, divide);

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

	engine.getBoard().generateMoves();
	std::vector<Move> currentMoveList = engine.getBoard().getMoveList();

	// return the number of moves if depth is 1
	if (depth == 1)
	{
		return (int)currentMoveList.size();
	}

	// loop through all legal moves
	for (const Move& move : currentMoveList)
	{
		// make the move and calculate the nodes in the game tree after this move
		engine.makeMove(move);
		long long change = tree(depth - 1, false);

		// print out number of nodes after each move
		if (divide)
		{
			std::cout << Square::toString(move.from) + Square::toString(move.to) << ": " << change << "\n";
		}

		nodes += change;
		engine.unmakeMove(move);
	}

	return nodes;
}