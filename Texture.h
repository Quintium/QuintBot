#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>

// texture wrapper class
class Texture 
{
    public:
        // initializes variables
        Texture();

        // deallocates memory
        ~Texture();

        // loads image at specified path
        bool loadFromFile(std::string path, SDL_Renderer* appRenderer);

        //Creates image from font string
        bool loadFromRenderedText(std::string textureText, SDL_Color textColor, TTF_Font* font, SDL_Renderer* appRenderer);

        // deallocates texture
        void free();

        // set color modulation
        void setColor(Uint8 red, Uint8 green, Uint8 blue);

        // set blending
        void setBlendMode(SDL_BlendMode blending);

        // set alpha modulation
        void setAlpha(Uint8 alpha);

        // renders texture at given point
        void render(SDL_Rect* dest, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

        // gets image dimensions
        int getWidth();
        int getHeight();

    private:
        // the actual hardware texture
        SDL_Texture* texture;

        // image dimensions
        int width;
        int height;

        // renderer variable
        SDL_Renderer* renderer;
};