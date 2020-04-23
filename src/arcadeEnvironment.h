/***********************************************************************************
 *
 * Header for functions and data needed for the Space Invaders Arcade Machine
 * @Author: Andrew Gunter
 *
***********************************************************************************/

#ifndef INTEL_8080_EMULATOR_ARCADEENVIRONMENT_H
#define INTEL_8080_EMULATOR_ARCADEENVIRONMENT_H

#include <stdio.h>
#include "sdl_sources/SDL.h"
#include "helpers.h"

#define SCREEN_WIDTH 224
#define SCREEN_HEIGHT 256

/**
 * Holds the parameters for the arcade machine
 */
typedef struct ArcadeParams{
    SDL_Window *window;  /**< The game window */
    SDL_Renderer *renderer;  /**< The renderer for the game window */
    unsigned int quitGame; /**< Boolean, 1 = end application, 0 = keep running */
} ArcadeState;

/**
 * Sets up the SDL environment.
 * Must be called before any other SDL actions.
 * @param arcade - arcade machines parameters
 * @return int - 1 if initialization was successful, 0 otherwise
 */
int initializeEnvironmentSDL(ArcadeState *arcade);

/**
 * Tears down the SDL environment.
 * Should be called after all SDL actions are complete.
 * @param arcade - arcade machine parameters
 * @return void
 */
void destroyEnvironmentSDL(ArcadeState *arcade);

#endif //INTEL_8080_EMULATOR_ARCADEENVIRONMENT_H
