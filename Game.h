#pragma once

#include "Ai.h"
#include "Board.h"
#include "Texture.h"

class Game
{
private:
	// game settings
	//const std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	const std::string startPosition = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	const int aiColor = WHITE;
	const int aiCount = 1;
	const int perspective = WHITE;

	// the chess board, ai
	Board board;
	AI* ai = nullptr;
	AI* ai2 = nullptr;

	// renderer, font and image for pieces
	SDL_Renderer* renderer = nullptr;
	TTF_Font* font = nullptr;
	TTF_Font* smallFont = nullptr;
	Texture piecesImage;
	Texture textTexture;
	SDL_Color black = { 0, 0, 0, 255 };

	// variables for dragging and dropping
	int dragX = -1;
	int dragY = -1;
	int dragSquare = -1;
	int dragPiece = -1;

	// last move for rendering
	Move lastMove = Move::getInvalidMove();

	// current game state
	int state = PLAY;

	// performance tests
	int perft(int depth, bool divide);
	void runPerft(int depth, bool divide);

public:
	// constructor
	Game(SDL_Renderer* myRenderer);

	// load media, render, events and cleanup
	bool loadMedia();
	void render();
	void loop();
	void handleEvent(SDL_Event* event);
	void cleanup();
};