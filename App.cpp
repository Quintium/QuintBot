#include "App.h"

// app constructor
App::App() 
{
    running = true;
}

// main function
int App::onExecute() 
{
    // try to initialize app: if failed - return -1
    if (!onInit()) 
    {
        return -1;
    }
    
    // create new event to check for future events
    SDL_Event event;

    while (running)
    {
        // loop through event queue and pass them on to onEvent()
        while (SDL_PollEvent(&event))
        {
            onEvent(&event);
        }

        // call the main loop and render function
        onRender();
        onLoop();
    }

    // clean the app up before finishing
    onCleanup();

    return 0;
}

// initialize the app
bool App::onInit()
{
    // initialize sdl, print error message if failed
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // create window, print error message if failed
    window = SDL_CreateWindow("Chess", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, S_WIDTH, S_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // create renderer for window, print error message if failed
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    // initialize image library, print error message if failed
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) 
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    // initialize SDL_ttf, print error message if failed
    if (TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    // get window surface
    screenSurface = SDL_GetWindowSurface(window);

    // load media
    if (!loadMedia()) return false;

    // return true if initialization was successful
    return true;
}

// load media (pictures, songs...)
bool App::loadMedia() 
{
    // create new game
    game = new Game(renderer);

    // load game media
    if (!game->loadMedia()) return false;

    // return true if media loading was successful
    return true;
}

// function for handling events
void App::onEvent(SDL_Event* event)
{
    // stop running if the event type is quit
    if (event->type == SDL_QUIT)
    {
        running = false;
    }
    else
    {
        game->handleEvent(event);
    }
}

// loop for handling the game other than events and rendering
void App::onLoop()
{
    game->loop();
}

void App::onRender() 
{
    // initialize renderer color
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    // clear screen
    SDL_RenderClear(renderer);

    // render the game
    game->render();

    // update screen
    SDL_RenderPresent(renderer);
}

void App::onCleanup() 
{
    // clean up the game
    game->cleanup();

    // destroy window and renderer 
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    window = nullptr;
    renderer = nullptr;

    // quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

// main function
int main(int argc, char* argv[])
{
    App theApp;

    // execute app
    return theApp.onExecute();
}