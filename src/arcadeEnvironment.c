/***********************************************************************************
 *
 * Source for functions needed for the Space Invaders Arcade Machine
 * @Author: Andrew Gunter
 *
***********************************************************************************/

#include "arcadeEnvironment.h"

ArcadeState *initializeArcade()
{
    // Create an arcade to work with
    ArcadeState *arcade = malloc(sizeof(ArcadeState));
    arcade->cpu = initializeCPU();
    arcade->window = NULL;
    arcade->renderer = NULL;

    // Setup SDL for communicating with host machine API
    if(initializeEnvironmentSDL(arcade) == 1){
        return arcade;
    }else{
        destroyCPU(arcade->cpu);
        destroyArcade(arcade);
        return NULL;
    }
}

int initializeEnvironmentSDL(ArcadeState *arcade)
{
    int successfulInit = 1;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        logger( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        successfulInit = 0;
    }

    if (successfulInit){
        // Set texture filtering
        // 0 - nearest pixel sampling
        // 1 - linear filtering
        // 2 - anistropic filtering
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")){
            logger("Warning: Failure to manually set texture filtering!\n");
        }

        // Create a window
        arcade->window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS, SDL_WINDOW_SHOWN);
        if (arcade->window == NULL){
            logger( "Window could not be created! SDL Error: %s\n", SDL_GetError());
            successfulInit = 0;
        }
    }

    if (successfulInit){
        // Create renderer for window
        arcade->renderer = SDL_CreateRenderer(arcade->window, -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (arcade->renderer == NULL){
            logger( "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
            successfulInit = 0;
        }
    }

    return successfulInit;
}

void destroyArcade(ArcadeState *arcade)
{
    // Destroy window
    SDL_DestroyWindow(arcade->window);
    arcade->window = NULL;
    // Destroy renderer
    SDL_DestroyRenderer(arcade->renderer);
    arcade->renderer = NULL;

    // Quit SDL and any related subsystems
    SDL_Quit();
}