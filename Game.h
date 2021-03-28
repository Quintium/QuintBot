#pragma once

// import relevent libraries
#include "AI.h"
#include "Board.h"
#include "Texture.h"
#include <SDL_mixer.h>

// class for the game
class Game
{
private:
	// game settings
    //const std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	//const std::string startPosition = "8/b7/5k2/8/8/B7/4b3/4K3 w - - 0 1";
	const std::string startPosition = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	const int aiColor = WHITE;
	const int aiCount = 1;
	const int perspective = WHITE;

	// the chess board, ai
	Board board;
	AI* ai = nullptr;
	AI* ai2 = nullptr;

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
	Game(SDL_Renderer* myRenderer);

	// load media, render, events and cleanup
	bool loadMedia();
	void render();
	void loop();
	void handleEvent(SDL_Event* event);
	void cleanup();
};