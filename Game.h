#pragma once

// import relevent libraries
#include <future>
#include "AI.h"
#include "Board.h"
#include "Texture.h"
#include <SDL_mixer.h>

// class for the game
class Game
{
private:
	// game mode (commands/gui)
	bool uciMode;

	// game settings
	const int aiColor = WHITE;
	const int aiCount = 1;
	const int perspective = BLACK;

	// the chess board, ai
	Board board;
	AI* ai;

	// rendering elements
	SDL_Renderer* renderer = nullptr;
	TTF_Font* font = nullptr;
	TTF_Font* smallFont = nullptr;
	Texture piecesImage;
	Texture textTexture;
	SDL_Color black = { 0, 0, 0, 255 };

	// sounds
	Mix_Chunk* moveSound = nullptr;
	Mix_Chunk* captureSound = nullptr;
	Mix_Chunk* endSound = nullptr;

	// variables for dragging and dropping
	int dragX = -1;
	int dragY = -1;
	int dragSquare = -1;
	int dragPiece = -1;

	// last move made
	Move lastMove = Move::getInvalidMove();

	// current game state
	int state = PLAY;

	// performance tests
	int perft(int depth, bool divide);
	void runPerft(int depth, bool divide);

	// play given move
	void playMove(Move move);

public:
	// constructor
	Game(SDL_Renderer* myRenderer, bool mode);

	// load media, render, events and cleanup
	bool loadMedia();
	void render();
	void loop();
	void handleEvent(SDL_Event* event);
	void cleanup();
};