/***********************************************************************************
 *
 * Emulates a Space Invaders Arcade Machine
 * @Author: Andrew Gunter
 *
***********************************************************************************/

#include "../src/arcadeEnvironment.h"

int main(int argc, char **argv)
{
    ArcadeParams arcade = {
            .window = NULL,
            .renderer = NULL,
    };

    // Initialize SDL
    initializeEnvironmentSDL(&arcade);

    // Tear down SDL
    destroyEnvironmentSDL(&arcade);

    return 0;
}

