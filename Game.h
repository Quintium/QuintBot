#pragma once

#include "Ai.h"
#include "Board.h"
#include "Texture.h"

class Game
{
private:
	// the chess board, ai
	Board board;
	AI* ai = nullptr;
	int aiColor = BLACK;

	// renderer, font and image for pieces
	SDL_Renderer* renderer = nullptr;
	TTF_Font* font = nullptr;
	Texture piecesImage;

	// variables for dragging and dropping
	int dragX = -1;
	int dragY = -1;
	int dragSquare = -1;
	int dragPiece = -1;

	// last move for rendering
	Move lastMove = { EMPTY, EMPTY, EMPTY, EMPTY, false, false, EMPTY};

	// current game state
	int state = PLAY;

	// performance tests
	int perft(int depth, bool divide);
	void runPerft(int depth, bool divide);

public:
	// constructor
	Game(SDL_Renderer* myRenderer, TTF_Font* myFont);

	// load media, render, events and cleanup
	bool loadMedia();
	void render();
	void loop();
	void handleEvent(SDL_Event* event);
	void cleanup();
};