#pragma once

// include libraries
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include "Texture.h"
#include "Game.h"

// main class for the SDL app
class App
{
    private:
        // boolean to stop the program once needed
        bool running;

        // is uci or gui mode
        bool uciMode = false;

        // window, screen and renderer variables
        SDL_Window* window = nullptr;
        SDL_Surface* screenSurface = nullptr;
        SDL_Renderer* renderer = nullptr;

        // game variable
        Game* game = nullptr;

        // event functions
        bool onInit();

        bool loadMedia();

        void onEvent(SDL_Event* event);

        void onLoop();

        void onRender();

        void onCleanup();

    public:
        // constructor and main function
        App();

        int onExecute();
};

// screen dimension constants
const int S_WIDTH = 1200;
const int S_HEIGHT = 800;