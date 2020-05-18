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
    ArcadeState *arcade = mallocSet(sizeof(ArcadeState));
    arcade->cpu = initializeCPU();
    arcade->window = NULL;
    arcade->renderer = NULL;

    resetPortsIO(arcade);
    synchronizeIO(arcade);


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

void synchronizeIO(ArcadeState *arcade)
{
    // Grab shorthand reference to cpu
    State8080 *cpu = arcade->cpu;

    // Copy arcade machine input port data to 8080 input buffers
    cpu->inputBuffers[0] = arcade->inputPort0;
    cpu->inputBuffers[1] = arcade->inputPort1;
    cpu->inputBuffers[2] = arcade->inputPort2;
    cpu->inputBuffers[3] = arcade->inputPort3;

    // Copy 8080 output buffer data to arcade machine output ports
    arcade->outputPort2 = cpu->outputBuffers[2];
    arcade->outputPort3 = cpu->outputBuffers[3];
    arcade->outputPort4 = cpu->outputBuffers[4];
    arcade->outputPort5 = cpu->outputBuffers[5];
    arcade->outputPort6 = cpu->outputBuffers[6];
}

void resetPortsIO(ArcadeState *arcade)
{
    // Set default CPU input port values
    arcade->inputPort0 = 0x0e;  // Bits 1-3 are always 1 by specification
    arcade->inputPort1 = 0x08;  // Bit 3 always 1 by specification
    arcade->inputPort2 = 0x00;
    arcade->inputPort3 = 0x00;

    // Reset CPU output ports
    arcade->outputPort2 = 0x00;
    arcade->outputPort3 = 0x00;
    arcade->outputPort4 = 0x00;
    arcade->outputPort5 = 0x00;
    arcade->outputPort6 = 0x00;
}

void updateShiftRegister(ArcadeState *arcade)
{
    synchronizeIO(arcade);  // Ensure sync with CPU

    uint8_t shiftRegUpperByte = (uint8_t)((arcade->shiftRegister)>>8);
    if(shiftRegUpperByte != arcade->outputPort4){
        // Shift register needs to be updated

        // Move shift reg upper byte to lower byte, then move output port 4 to shift reg upper byte
        arcade->shiftRegister >>= 8;
        arcade->shiftRegister |= (((uint16_t)(arcade->outputPort4))<<8);
    }else{
        // No change, no need to update
    }

    // Derive an offset value from shift register and place this value in input port 3
    uint8_t offset = (arcade->outputPort2) & 0x07;  // shift amount contained in output port 2 bits 0-2
    uint8_t result = (uint8_t)((arcade->shiftRegister)>>(8-offset));
    arcade->inputPort3 = result;

    // Re-sync with CPU
    synchronizeIO(arcade);
}

void runForCpuCycles(unsigned int numCyclesToRun, ArcadeState *arcade)
{
    unsigned int startingCycles = arcade->cpu->cyclesCompleted;
    while((arcade->cpu->cyclesCompleted - startingCycles) < numCyclesToRun){
        updateShiftRegister(arcade);
        executeNextInstruction(arcade->cpu);
    }
}