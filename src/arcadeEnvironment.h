/***********************************************************************************
 *
 * Header for functions and data needed for the Space Invaders Arcade Machine
 * @Author: Andrew Gunter
 *
***********************************************************************************/

#ifndef INTEL_8080_EMULATOR_ARCADEENVIRONMENT_H
#define INTEL_8080_EMULATOR_ARCADEENVIRONMENT_H

#include <stdio.h>
#include <math.h>
#include "sdl_sources/SDL.h"
#include "helpers.h"
#include "cpuStructures.h"
#include "shell8080.h"

#define SCREEN_WIDTH_PIXELS 224
#define SCREEN_HEIGHT_PIXELS 256
#define BYTES_PER_PIXEL 4
#define FPS 60
#define CYCLES_PER_FRAME floor(CYCLES_PER_SECOND_8080/FPS)
#define MIDSCREEN_INTERRUPT_LINE 96


/**
 * Holds the parameters for the arcade machine
 */
typedef struct ArcadeState{
    State8080 *cpu;
    SDL_Window *window;  /**< The game window */
    SDL_Renderer *renderer;  /**< The renderer for the game window */
    SDL_Texture *prerenderTexture; /**< The texture to be drawn on prior to rendering to the game screen */
    // Input ports, read from by 8080
    uint8_t inputPort0;
    uint8_t inputPort1;
    uint8_t inputPort2;
    uint8_t inputPort3;
    // Output ports, written to by 8080
    uint8_t outputPort2;  /** Write ports start counting at 2 for some unknown reason
                             (Source: http://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html) */
    uint8_t outputPort3;
    uint8_t outputPort4;
    uint8_t outputPort5;
    uint8_t outputPort6;
    uint16_t shiftRegister;  /** Custom hardware, found in arcade cabinet, for performing multi-bit shifts */
} ArcadeState;

/**
 * Sets up an arcade for emulation
 * @return ArcadeState* - initialized arcade, or NULL if initialization failed
 */
ArcadeState *initializeArcade();

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
void destroyArcade(ArcadeState *arcade);

/**
 * Synchronizes the arcade machine's and 8080's I/O
 * Arcade machine emulator has "ports" and 8080 emulator has "buffers"
 * Data flow is:
 * Input - Arcade machine emulator -> Input port -> Input buffer -> 8080 CPU
 * Output - 8080 CPU -> Output buffer -> Output port -> Arcade machine emulator
 * @param arcade
 */
void synchronizeIO(ArcadeState *arcade);

#endif //INTEL_8080_EMULATOR_ARCADEENVIRONMENT_H
