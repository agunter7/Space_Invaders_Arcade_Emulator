/***********************************************************************************
 *
 * Emulates a Space Invaders Arcade Machine
 * @Author: Andrew Gunter
 *
***********************************************************************************/

#include "../src/arcadeEnvironment.h"
#include "shell8080.h"

void playSpaceInvaders(ArcadeState *arcade);
unsigned int handleGameEvents(ArcadeState *arcade);

int main(int argc, char **argv)
{
    ArcadeState *arcade = initializeArcade();

    if(arcade != NULL){
        playSpaceInvaders(arcade);
        destroyArcade(arcade);
    }

    return 0;
}

void playSpaceInvaders(ArcadeState *arcade)
{
    unsigned int quitGame = 0;


    uint32_t screenPixels[256][224];
    memset(screenPixels, 0xff, 256*224*4);
    SDL_Texture *texture = SDL_CreateTexture(arcade->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_UpdateTexture(texture, NULL, screenPixels, 224*4);
    //runForCycles(10, arcade->cpu);
    logger("Entering loop\n");
    while (!quitGame){
        //Start timer

        quitGame = handleGameEvents(arcade);

        // Clear screen
        SDL_RenderClear(arcade->renderer);

        // Load/render window image
        SDL_RenderCopy(arcade->renderer, texture, NULL, NULL);

        // Play any sounds

        // Update screen
        SDL_RenderPresent(arcade->renderer);

        // If frame time < (1/60)s, then stall
    }
}

unsigned int handleGameEvents(ArcadeState *arcade)
{
    SDL_Event currentEvent;

    while(SDL_PollEvent(&currentEvent) != 0){
        // Events to check: quit, game input

        if(currentEvent.type == SDL_QUIT){
            return 1;
        }

        // User inputs become interrupts

        // Process interrupts

        // Process (33,333 - numInterruptCycles) cycles
    }

    return 0;
}

