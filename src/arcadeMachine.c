/***********************************************************************************
 *
 * Emulates a Space Invaders Arcade Machine
 * @Author: Andrew Gunter
 *
***********************************************************************************/

#include "../src/arcadeEnvironment.h"

void playSpaceInvaders(ArcadeState *arcade);
unsigned int handleGameEvents(ArcadeState *arcade);

int main(int argc, char **argv)
{
    ArcadeState arcade = {
            .window = NULL,
            .renderer = NULL,
            .quitGame = 0,
    };

    // Initialize SDL
    initializeEnvironmentSDL(&arcade);

    playSpaceInvaders(NULL);

    // Tear down SDL
    destroyEnvironmentSDL(&arcade);

    return 0;
}

void playSpaceInvaders(ArcadeState *arcade)
{
    unsigned int quitGame = 0;
    while (!quitGame){
        //Start timer

        quitGame = handleGameEvents(arcade);

        // Clear screen

        // Load/render window image

        // Play any sounds

        // Update screen

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

