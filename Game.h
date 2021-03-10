#pragma once

#include "Ai.h"
#include "Board.h"
#include "Texture.h"

class Game
{
private:
	// game settings
	std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	//std::string startPosition = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
	int aiColor = WHITE;
	int aiCount = 1;
	int perspective = BLACK;

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
	Move lastMove = { EMPTY, EMPTY, EMPTY, EMPTY, false, false, EMPTY};

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