#include "Game.h"

// game constructor with renderer and font
Game::Game(SDL_Renderer* myRenderer, TTF_Font* myFont)
{
	// save renderer and font
	renderer = myRenderer;
	font = myFont;

	// load board position
	board.loadFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	//board.loadFromFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");

	// run performance test
	//runPerft(5, true);

	// generate next moves
	board.generateMoves();

	// initialize ai
	ai = new AI(&board, aiColor);
}

// load all media
bool Game::loadMedia()
{
	// load PNG texture, return false if failed
	if (!piecesImage.loadFromFile("Images/Chess_Pieces.png", renderer)) return false;
	return true;
}

// render the board
void Game::render() {
	// initialize the squares that are attacked
	std::vector<int> attackSquares;

	// if piece is being dragged
	if (dragPiece != -1)
	{
		// load moves and loop through them
		std::vector<Move> moves = *board.getMoveList();

		for (int i = 0; i < moves.size(); i++)
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
			bool isLightSquare = (x + y) % 2 == 0;

			if (std::find(attackSquares.begin(), attackSquares.end(), Square::fromXY(x, y)) != attackSquares.end())
			{
				// set draw color to red if attacked
				if (isLightSquare)
				{
					SDL_SetRenderDrawColor(renderer, 190, 52, 55, 0xFF);;
				}
				else
				{
					SDL_SetRenderDrawColor(renderer, 169, 38, 47, 0xFF);
				}
			}
			else if ((dragPiece != EMPTY && (Square::fromXY(x, y) == dragSquare)) || (lastMove.from == Square::fromXY(x, y)))
			{
				// set draw color to yellow if last move was from this square or dragging is from this square
				SDL_SetRenderDrawColor(renderer, 208, 143, 76, 0xFF);
			}
			else if (lastMove.to == Square::fromXY(x, y))
			{
				// set draw color to green if last move was to this square
				SDL_SetRenderDrawColor(renderer, 206, 160, 76, 0xFF);
			}
			else
			{
				// set draw color to white/black if square is normal
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
			if (dragPiece == EMPTY || dragSquare != Square::fromXY(x, y))
			{
				// get piece on that square
				int piece = board.getPiecesMB()[Square::fromXY(x, y)];

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
		// initialize text texture, message and color
		Texture textTexture;
		std::string message;
		SDL_Color color = { 0, 0, 0, 255 };

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
		textTexture.loadFromRenderedText(message, color, font, renderer);
		SDL_Rect dest = { 1000 - textTexture.getWidth() / 2, 400 - textTexture.getHeight() / 2, textTexture.getWidth(), textTexture.getHeight() };
		textTexture.render(&dest);
	}
}

// game events
void Game::handleEvent(SDL_Event* event)
{
	// only check if game is played
	if (state == PLAY && (board.getTurnColor() != aiColor))
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
				dragSquare = Square::fromXY(dragX / 100, dragY / 100);

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
				int endSquare = Square::fromXY(event->button.x / 100, event->button.y / 100);

				// load available moves
				std::vector<Move> moves = *board.getMoveList();

				// loop through moves
				for (int i = 0; i < moves.size(); i++)
				{
					Move move = moves[i];

					// if move matches start and end square
					if (move.from == dragSquare && move.to == endSquare)
					{
						// make the move, generate next moves and save last move
						board.makeMove(&move);
						board.generateMoves();
						lastMove = move;

						// if no moves are available
						if ((*board.getMoveList()).size() == 0)
						{
							if (board.getCheck())
							{
								// if in check, end the game on a win
								if (board.getTurnColor() == WHITE)
								{
									state = BLACK_WIN;
								}
								else
								{
									state = WHITE_WIN;
								}
							}
							// end the game on a draw if stalemate
							else
							{
								state = DRAW;
							}
						}
						// end the game on a draw if 50-move-rule
						else if (board.getHalfMoveClock() >= 100)
						{
							state = DRAW;
						}

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

void Game::loop()
{
	if ((state == PLAY) && (aiColor == board.getTurnColor()))
	{
		// get best move
		Move move = ai->getBestMove(4);
		board.makeMove(&move);
		board.generateMoves();
		lastMove = move;

		// if no moves are available
		if ((*board.getMoveList()).size() == 0)
		{
			if (board.getCheck())
			{
				// if in check, end the game on a win
				if (board.getTurnColor() == WHITE)
				{
					state = BLACK_WIN;
				}
				else
				{
					state = WHITE_WIN;
				}
			}
			// end the game on a draw if stalemate
			else
			{
				state = DRAW;
			}
		}
		// end the game on a draw if 50-move-rule
		else if (board.getHalfMoveClock() >= 100)
		{
			state = DRAW;
		}
	}
}

// run performance test
void Game::runPerft(int depth, bool divide)
{
	// save the time at the start
	auto start = std::chrono::system_clock::now();

	// calculate the nodes searched at given depth
	int nodes = perft(depth, divide);

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
}

// performance test function
int Game::perft(int depth, bool divide)
{
	int nodes = 0;

	// generate moves and save them
	board.generateMoves();
	std::vector<Move> currentMoveList = *board.getMoveList();

	// return the number of moves if depth is 1
	if (depth == 1)
	{
		return currentMoveList.size();
	}

	// loop through moves
	for (Move move : currentMoveList)
	{
		// make the move and calculate the nodes after this position with a lower depth
		board.makeMove(&move);
		int change = perft(depth - 1, false);

		// print out number of nodes after each position if divide argument is true
		if (divide)
		{
			std::cout << Square::toString(move.from) + Square::toString(move.to) << ": " << change << "\n";
		}

		// add change to the nodes count and unmake move
		nodes += change;
		board.unmakeMove(&move);
	}

	// return the number of nodes
	return nodes;
}
// clean up all images
void Game::cleanup()
{
	piecesImage.free();
}