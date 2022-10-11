#include "Game.h"

// game constructor with renderer and font
Game::Game(SDL_Renderer* myRenderer, bool mode)
{
	// save renderer and font
	renderer = myRenderer;

	// save mode
	uciMode = mode;

	// load board position
	board.loadStartPosition();

	// generate next moves
	board.generateMoves();

	// initialize ai
	ai = new AI(&board, assetsPath);
}

// load all media
bool Game::loadMedia()
{
	if (!uciMode)
	{
		// load PNG texture, return false if failed
		if (!piecesImage.loadFromFile(assetsPath + "/Images/Chess_Pieces.png", renderer)) return false;

		// open the font, print error message if failed
		font = TTF_OpenFont((assetsPath + "/Fonts/OpenSans.ttf").c_str(), 60);
		if (font == nullptr)
		{
			printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
			return false;
		}

		// open the font, print error message if failed
		smallFont = TTF_OpenFont((assetsPath + "/Fonts/OpenSans.ttf").c_str(), 12);
		if (smallFont == nullptr)
		{
			printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
			return false;
		}

		// load sound effects
		moveSound = Mix_LoadWAV((assetsPath + "/Sounds/Move.wav").c_str());
		if (moveSound == nullptr)
		{
			printf("Failed to load move sound effect! SDL_mixer Error: %s\n", Mix_GetError());
			return false;
		}

		captureSound = Mix_LoadWAV((assetsPath + "/Sounds/Capture.wav").c_str());
		if (captureSound == nullptr)
		{
			printf("Failed to load capture sound effect! SDL_mixer Error: %s\n", Mix_GetError());
			return false;
		}

		endSound = Mix_LoadWAV((assetsPath + "/Sounds/End.wav").c_str());
		if (endSound == nullptr)
		{
			printf("Failed to load end sound effect! SDL_mixer Error: %s\n", Mix_GetError());
			return false;
		}
	}

	return true;
}

// render the board
void Game::render() {
	if (!uciMode)
	{
		// initialize the squares that are attacked
		std::vector<int> attackSquares;

		// if piece is being dragged
		if (dragPiece != -1)
		{
			// load moves and loop through them
			std::vector<Move> moves = board.getMoveList();

			for (size_t i = 0; i < moves.size(); i++)
			{
				// if the move is of the piece being dragged, add it
				Move move = moves[i];
				if (dragSquare == move.from)
				{
					attackSquares.push_back(move.to);
				}
			}
		}

		// loop through squares
		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				// calculate if square is light
				int square = Square::perspective(Square::fromXY(x, y), perspective);
				bool isLightSquare = Square::isLight(square);

				// set draw color to red if attacked
				if (std::find(attackSquares.begin(), attackSquares.end(), square) != attackSquares.end())
				{
					if (isLightSquare)
					{
						SDL_SetRenderDrawColor(renderer, 190, 52, 55, 0xFF);;
					}
					else
					{
						SDL_SetRenderDrawColor(renderer, 169, 38, 47, 0xFF);
					}
				}
				// set draw color to yellow if last move was from or to this square or the player is dragging from this square
				else if ((dragPiece != EMPTY && (square == dragSquare)) || (lastMove.from == square) || (lastMove.to == square))
				{
					if (isLightSquare)
					{
						SDL_SetRenderDrawColor(renderer, 206, 160, 76, 0xFF);
					}
					else
					{
						SDL_SetRenderDrawColor(renderer, 208, 143, 76, 0xFF);
					}
				}
				// set draw color to brown if square is normal
				else
				{
					if (isLightSquare)
					{
						SDL_SetRenderDrawColor(renderer, 240, 216, 192, 0xFF);
					}
					else
					{
						SDL_SetRenderDrawColor(renderer, 168, 121, 101, 0xFF);
					}
				}

				// create destination rect and fill it
				SDL_Rect dest = { x * 100, y * 100, 100, 100 };
				SDL_RenderFillRect(renderer, &dest);

				// if piece isn't being dragged from this square
				if (dragPiece == EMPTY || dragSquare != square)
				{
					// get piece on that square
					int piece = board.getPiecesMB()[square];

					// if there's a piece
					if (piece != EMPTY)
					{
						// create clip rect of the original piece image based on piece id
						SDL_Rect clip = { Piece::typeOf(piece) / 2 * 320, Piece::colorOf(piece) * 320, 320, 320 };

						// render the piece
						piecesImage.render(&dest, &clip);
					}
				}
			}
		}

		// loop through all ranks and files
		for (int i = 0; i < 8; i++)
		{
			// initialize char, width, height and destination rect
			char c;
			int w, h;
			SDL_Rect dest;

			// get the rank char
			c = Square::toChar(Square::coordPerspective(i, perspective), true);

			// load texture from text, get width/height and render it on the right
			textTexture.loadFromRenderedText(std::string(1, c), black, smallFont, renderer);
			w = textTexture.getWidth(), h = textTexture.getHeight();
			dest = { 800 - (w + 5), i * 100 + 2, w, h };
			textTexture.render(&dest);

			// get the file char
			c = Square::toChar(Square::coordPerspective(i, perspective), false);

			// load texture from text, get width/height and render it on the bottom
			textTexture.loadFromRenderedText(std::string(1, c), black, smallFont, renderer);
			w = textTexture.getWidth(), h = textTexture.getHeight();
			dest = { i * 100 + 5, 800 - (h + 5), w, h };
			textTexture.render(&dest);
		}

		// if there's a piece being dragged
		if (dragPiece != EMPTY)
		{
			// draw dragged piece at dragX and dragY
			SDL_Rect dest = { dragX - 50, dragY - 50, 100, 100 };
			SDL_Rect clip = { Piece::typeOf(dragPiece) / 2 * 320, Piece::colorOf(dragPiece) * 320, 320, 320 };
			piecesImage.render(&dest, &clip);
		}

		// if the game ended
		if (state != PLAY)
		{
			// initialize message
			std::string message;

			// create message based on state
			switch (state)
			{
			case WHITE_WIN:
				message = "White wins!";
				break;
			case BLACK_WIN:
				message = "Black wins!";
				break;
			case DRAW:
				message = "Draw!";
				break;
			}

			// load texture from text and draw it on the right
			textTexture.loadFromRenderedText(message, black, font, renderer);
			SDL_Rect dest = { 1000 - textTexture.getWidth() / 2, 400 - textTexture.getHeight() / 2, textTexture.getWidth(), textTexture.getHeight() };
			textTexture.render(&dest);
		}
	}
}

// game events
void Game::handleEvent(SDL_Event* event)
{
	// only check if game is played
	if (state == PLAY && aiCount < 2 && ((board.getTurnColor() != aiColor) || aiCount == 0) && !uciMode)
	{
		// check event type
		switch (event->type)
		{
		// if clicked
		case SDL_MOUSEBUTTONDOWN:
			// if there's no piece being dragged
			if (dragPiece == EMPTY)
			{
				// calculate chess square from mouse position
				dragX = event->motion.x;
				dragY = event->motion.y;
				dragSquare = Square::perspective(Square::fromXY(dragX / 100, dragY / 100), perspective);

				// get piece on square
				int piece = board.getPiecesMB()[dragSquare];
				if (piece != EMPTY && ((board.getTurnColor() == WHITE) != (Piece::colorOf(piece) == BLACK)))
				{
					// start dragging piece if there's a piece and it's that color's turn
					dragPiece = piece;
				}
			}

			break;

		// if unclicked
		case SDL_MOUSEBUTTONUP:
			// only if piece is being dragged
			if (dragPiece != EMPTY)
			{
				// calculate square of mouse position
				int endSquare = Square::perspective(Square::fromXY(event->button.x / 100, event->button.y / 100), perspective);

				// load available moves
				std::vector<Move> moves = board.getMoveList();

				// loop through moves
				for (size_t i = 0; i < moves.size(); i++)
				{
					Move move = moves[i];

					// if move matches start and end square
					if (move.from == dragSquare && move.to == endSquare)
					{
						// make the move, generate next moves and save last move
						playMove(move);

						break;
					}
				}

				// end dragging
				dragPiece = EMPTY;
			}

			break;

		// if mouse was moved and piece is being dragged, change drag position
		case SDL_MOUSEMOTION:
			if (dragPiece != EMPTY)
			{
				dragX = event->motion.x;
				dragY = event->motion.y;
			}
			break;
		}
	}
}

// main loop function, returns whether program should be exited
bool Game::loop()
{
	if (!uciMode)
	{
		// if it's the ai's trurn
		if (state == PLAY && aiCount > 0 && ((aiColor == board.getTurnColor()) || aiCount == 2))
		{
			// get best move
			Move move = ai->getBestMove(-1, 0, -1, aiTime);

			// play that move
			playMove(move);
		}
	}
	else
	{
		// get input
		std::string input;
		std::getline(std::cin, input);

		// if input is "uci", output all id information
		if (input == "uci")
		{
			std::cout << "id name QuintiumBot\n";
			std::cout << "id author Quintium\n";
			std::cout << "uciok\n";
		}

		// respond to "isready" with "readyok"
		if (input == "isready")
		{
			std::cout << "readyok\n";
		}

		if (input == "ucinewgame")
		{

		}

		// exit program on command
		if (input == "quit")
		{
			return true;
		}

		// if input starts with "position"
		if (input.rfind("position", 0) == 0)
		{
			// strip off the "position"
			input = input.substr(9);

			// find position of "moves" in the input
			size_t movePos = input.find("moves");

			// if input starts with "fen", load fen
			if (input.rfind("fen", 0) == 0)
			{
				if (movePos != std::string::npos)
				{
					board.loadFromFen(input.substr(4, movePos - 1 - 4));
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
				// place move pos after "moves "
				movePos += 6;

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

		// if input starts with "go"
		if (input.rfind("go", 0) == 0)
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
				Move move = ai->getBestMove(timeLeft, increment, depth, exactTime);
				std::cout << "bestmove " << move.getNotation() << "\n";
			}
		}

		// speed test on position 2 with depth 7
		if (input == "speed test")
		{
			board.loadFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 15");
			ai->getBestMove(-1, 0, 7, -1);
		}

		// evaluation for debugging reasons
		if (input == "eval")
		{
			Evaluation evaluation(&board);
			int eval = evaluation.evaluate();
			std::cout << "Evaluation: " << eval << "\n";
		}

		// print out fen of board for debugging reasons
		if (input == "fen")
		{
			std::cout << "Fen: " << board.getFen() << "\n";
		}
	}

	return false; // program should not exit under normal circumstances
}

// play a move
void Game::playMove(Move move)
{
	// make the move, generate new moves, save last move
	board.makeMove(move);
	board.generateMoves();
	lastMove = move;

	// get the new game state
	state = board.getState();

	// play capture or move sound
	if (move.cPiece != EMPTY)
	{
		Mix_PlayChannel(-1, captureSound, 0);
	}
	else
	{
		Mix_PlayChannel(-1, moveSound, 0);
	}

	// play end sound if game ended
	if (state != PLAY)
	{
		Mix_PlayChannel(-1, endSound, 0);
	}
}

// run performance test
void Game::runPerft(int depth, bool divide)
{
	// save the time at the start
	auto start = std::chrono::system_clock::now();

	// calculate the nodes searched at given depth
	long long nodes = perft(depth, divide);

	// print out the nodes searched
	std::cout << "\n";
	std::cout << "Depth " << depth << ": " << nodes << " nodes searched.\n";

	// save end time and calculate time Passed
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	double timePassed = diff.count();

	// print out time passed and nodes searched per second
	std::cout << "Time needed: " << timePassed << "s\n";
	std::cout << "Nodes per second: " << nodes / timePassed << "\n";
	std::cout << "\n";
}

// performance test function
long long Game::perft(int depth, bool divide)
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
		int change = perft(depth - 1, false);

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
// clean up all images
void Game::cleanup()
{
	if (!uciMode)
	{
		// free all sounds
		Mix_FreeChunk(moveSound);
		moveSound = nullptr;

		Mix_FreeChunk(captureSound);
		captureSound = nullptr;

		Mix_FreeChunk(endSound);
		endSound = nullptr;

		// free textures
		piecesImage.free();
		textTexture.free();

		// free font
		TTF_CloseFont(font);
		font = nullptr;

		// free font
		TTF_CloseFont(smallFont);
		smallFont = nullptr;
	}
}