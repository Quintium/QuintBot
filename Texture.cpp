#include "Texture.h"

Texture::Texture()
{
    // initialize variables
    texture = nullptr;
    renderer = nullptr;
    width = 0;
    height = 0;
}

Texture::~Texture()
{
    // deallocate
    free();
}

bool Texture::loadFromFile(std::string path, SDL_Renderer* appRenderer) 
{
    // save renderer
    renderer = appRenderer;

    // get rid of preexisting texture
    free();

    // the final texture
    SDL_Texture* newTexture = nullptr;

    // load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) 
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
        return false;
    }

    // create texture from surface pixels
    newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (newTexture == nullptr) 
    {
        printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
        return false;
    }

    // get image dimensions
    width = loadedSurface->w;
    height = loadedSurface->h;

    // get rid of old loaded surface
    SDL_FreeSurface(loadedSurface);

    // return success
    texture = newTexture;
    return texture != nullptr;
}

bool Texture::loadFromRenderedText(std::string textureText, SDL_Color textColor, TTF_Font* font, SDL_Renderer* appRenderer)
{
    // save renderer
    renderer = appRenderer;

    // get rid of preexisting texture
    free();

    // render text surface
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, textureText.c_str(), textColor);
    if (textSurface == nullptr)
    {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    // create texture from surface pixels
    texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (texture == nullptr) 
    {
        printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    // get image dimensions
    width = textSurface->w;
    height = textSurface->h;

    // get rid of old surface
    SDL_FreeSurface(textSurface);

    // return success
    return true;
}

void Texture::free()
{
    // free texture if it exists
    if (texture != nullptr)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        width = 0;
        height = 0;
    }
}

void Texture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
    // modulate texture
    SDL_SetTextureColorMod(texture, red, green, blue);
}

void Texture::setBlendMode(SDL_BlendMode blending)
{
    // set blending function
    SDL_SetTextureBlendMode(texture, blending);
}

void Texture::setAlpha(Uint8 alpha)
{
    // modulate texture alpha
    SDL_SetTextureAlphaMod(texture, alpha);
}

// render texture
void Texture::render(SDL_Rect* dest, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
    // if renderer was not initialized return error message
    if (renderer == nullptr) 
    {
        printf("Texture wasn't initialized!\n");
        return;
    }

    //Render to screen
    SDL_RenderCopyEx(renderer, texture, clip, dest, angle, center, flip);
}

// return width and height of texture
int Texture::getWidth()
{
    return width;
}

int Texture::getHeight()
{
    return height;
}