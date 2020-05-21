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
    if(initializeEnvironmentSDL(arcade) == 1 && loadAudio(arcade) == 1){
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
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
                                          SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

    if(successfulInit){
        // Initialize SDL_mixer
        if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
            logger("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
            successfulInit = 0;
        }
    }

    return successfulInit;
}

int loadAudio(ArcadeState *arcade)
{
    arcade->ufoMusic = Mix_LoadMUS("resources/ufo_lowpitch.wav");
    if(arcade->ufoMusic == NULL){
        logger("Failed to load UFO music! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }
    arcade->playerShootSfx = Mix_LoadWAV("resources/shoot.wav");
    if(arcade->playerShootSfx == NULL){
        logger("Failed to load player shoot sfx! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }
    arcade->playerDieSfx = Mix_LoadWAV("resources/explosion.wav");
    if(arcade->playerDieSfx == NULL){
        logger("Failed to load player died sfx! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }
    arcade->invaderDieSfx = Mix_LoadWAV("resources/invaderkilled.wav");
    if(arcade->invaderDieSfx == NULL){
        logger("Failed to load invader died sfx! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }
    arcade->fleetMove1Sfx = Mix_LoadWAV("resources/fastinvader1.wav");
    if(arcade->fleetMove1Sfx == NULL){
        logger("Failed to load fleet move 1 music! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }
    arcade->fleetMove2Sfx = Mix_LoadWAV("resources/fastinvader2.wav");
    if(arcade->fleetMove2Sfx == NULL){
        logger("Failed to load fleet move 2 music! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }
    arcade->fleetMove3Sfx = Mix_LoadWAV("resources/fastinvader3.wav");
    if(arcade->fleetMove3Sfx == NULL){
        logger("Failed to load fleet move 3 music! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }
    arcade->fleetMove4Sfx = Mix_LoadWAV("resources/fastinvader4.wav");
    if(arcade->fleetMove4Sfx == NULL){
        logger("Failed to load fleet move 4 music! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }
    arcade->ufoDieSfx = Mix_LoadWAV("resources/ufo_highpitch.wav");
    if(arcade->ufoDieSfx == NULL){
        logger("Failed to load UFO died sfx! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }

    return 1;
}

void destroyArcade(ArcadeState *arcade)
{
    // Free audio
    Mix_FreeMusic(arcade->ufoMusic);
    arcade->ufoMusic = NULL;
    Mix_FreeChunk(arcade->playerShootSfx);
    arcade->playerShootSfx = NULL;
    Mix_FreeChunk(arcade->playerDieSfx);
    arcade->playerDieSfx = NULL;
    Mix_FreeChunk(arcade->invaderDieSfx);
    arcade->invaderDieSfx = NULL;
    Mix_FreeChunk(arcade->fleetMove1Sfx);
    arcade->fleetMove1Sfx = NULL;
    Mix_FreeChunk(arcade->fleetMove2Sfx);
    arcade->fleetMove2Sfx = NULL;
    Mix_FreeChunk(arcade->fleetMove3Sfx);
    arcade->fleetMove3Sfx = NULL;
    Mix_FreeChunk(arcade->fleetMove4Sfx);
    arcade->fleetMove4Sfx = NULL;
    Mix_FreeChunk(arcade->ufoDieSfx);
    arcade->ufoDieSfx = NULL;
    // Destroy window
    SDL_DestroyWindow(arcade->window);
    arcade->window = NULL;
    // Destroy renderer
    SDL_DestroyRenderer(arcade->renderer);
    arcade->renderer = NULL;

    // Quit SDL and any related subsystems
    Mix_Quit();
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