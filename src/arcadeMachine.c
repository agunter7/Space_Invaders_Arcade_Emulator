/***********************************************************************************
 *
 * Emulates a Space Invaders Arcade Machine
 * @Author: Andrew Gunter
 *
***********************************************************************************/

#include "../src/arcadeEnvironment.h"

void playSpaceInvaders(ArcadeState *arcade);
unsigned int handleGameEvents(ArcadeState *arcade);
uint32_t *getCurrentFramePixels(State8080 *cpu);

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
    /*for(int j=0; j<256; j+=10){
        for(int i=0; i< (224*4); i+=4){
            memset(&(((uint8_t*)(screenPixels[j]))[i+2]), 0x00, 1);
        }
    }*/
    SDL_Texture *texture = SDL_CreateTexture(arcade->renderer, SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);
    logger("Entering loop\n");
    while (!quitGame){
        //Start timer

        quitGame = handleGameEvents(arcade);
        //logger("Handled game events\n");

        // Clear screen
        SDL_RenderClear(arcade->renderer);
        //logger("Cleared screen\n");

        // Load/render window image
        SDL_UpdateTexture(texture, NULL, getCurrentFramePixels(arcade->cpu), SCREEN_WIDTH_PIXELS*BYTES_PER_PIXEL);
        //logger("Updated texture\n");
        SDL_RenderCopy(arcade->renderer, texture, NULL, NULL);
        //logger("Copied texture to renderer\n");

        // Play any sounds

        // Update screen
        SDL_RenderPresent(arcade->renderer);
        //logger("Presented renderer\n");

        // If frame time < (1/60)s, then stall
        //logger("Finished a main loop\n");
    }
}

unsigned int handleGameEvents(ArcadeState *arcade)
{
    SDL_Event currentEvent;

    unsigned int numInterruptCycles = 0;
    while(SDL_PollEvent(&currentEvent) != 0){
        // Events to check: quit, game input

        if(currentEvent.type == SDL_QUIT){
            logger("Quitting game\n");
            return 1;
        }

        // User inputs become interrupts

        // Process interrupts
        numInterruptCycles += 0;
    }

    runForCycles(CYCLES_PER_FRAME - numInterruptCycles, arcade->cpu);
    logger("Cycles ran\n");

    return 0;
}

/**
 * Returns a pointer to the 32-bits-per-pixel data for the current frame to be rendered (by extracting from 8080 VRAM)
 * @param cpu - The 8080 state
 * @return Pointer to pixel data, ready to be rendered directly by SDL
 */
uint32_t *getCurrentFramePixels(State8080 *cpu)
{
    // get rotated pixel data from cpu
    // 1 bit per pixel
    uint8_t *rotatedPixels = getVideoRAM(cpu);

    // At this point, a byte contains data for 8 pixels. However, the order of the bytes is counter-clockwise.
    // This is because the original Space Invaders cabinet used a rotated CRT, so the developers accounted for this.
    // Hence, if we read the bits in-order, we will be reading columnar data.
    // By bit-index, with height=256 & width=224, our (corrected) screen is:
    /*
    255 - 511 - 767 - ... - 57343
     |  -  |  -  |  - ... -   |
    251 - 510 - 766 - ... - 57342
     |  -  |  -  |  - ... -   |
    ||| - ||| - ||| - ... -  |||
     |  -  |  -  |  - ... -   |
     1  - 257 - 513 - ... - 57089
     |  -  |  -  |  - ... -   |
     0  - 256 - 512 - ... - 57088
    */
    // This poses 2 problems:
    // 1) The renderer is 32 bits per pixel
    // 2) The renderer operates top-left pixel to bottom-right pixel, row-by-row.
    //
    // Fixing problem #1 is easy, we just expand each bit to 32 bits and we can even add RGBA info as we desire.
    // Fixing problem #2 is messy. If we read from our data as currently ordered, we would have the pixel that should
    // be in the bottom-leftmost position ending up rendered in the top-leftmost position.
    // To fix this, we can create a map from the render-order pixel indices to the rotated ones that we have now:
    /*
      0  : 255 -  1  : 511 -    ...   - 223 :57343
         |     -     |     -     |    -     |
     224 : 251 - 225 : 510 -    ...   - 447 :57342
         |     -     |     -     |    -     |
        |||    -    |||    -    |||   -    |||
         |     -     |     -     |    -     |
    56896:  1  -56897: 257 -    ...   -57119:57089
         |     -     |     -     |    -     |
    57120:  0  -57121: 256 -    ...   -57343:57088
    */
    // The render-order pixels are on the left of each colon while the rotated versions from the 8080 VRAM
    // are on the right of each colon.
    // We can call the render-order indices I_2 and the rotated indices I_1
    // Thus our goal is to find a mapping function of the form I_1 = f(I_2)
    //
    // If we give each pixel a coordinate of the form (x,y) and assert that Width=W and Height=H
    // We may note the following by inspection:
    // I_1 = (x+1)H - (y+1);  x = I_2%W;  y = floor(I_2/W);
    // Substituting as appropriate yields:
    // I_1 = ((I_2%W)+1)H - (floor(I_2/W)+1);

    // Prepare to expand pixels
    // each bit from cpu will become 32 bits (RGBA format)
    // 8 bits for each of: red, green, blue, alpha
    unsigned int numPixels = SCREEN_HEIGHT_PIXELS*SCREEN_WIDTH_PIXELS;
    unsigned int numPixelBytes = numPixels*BYTES_PER_PIXEL;
    uint32_t *currentFramePixels = malloc(numPixelBytes);  // Pointer to data describing 32-bit pixels for current frame

    // Get 32-bit pixel data for entire frame by iterating through pixels to be rendered on screen
    unsigned int I_1 = 0;
    unsigned int W = SCREEN_WIDTH_PIXELS;
    unsigned int H = SCREEN_HEIGHT_PIXELS;
    uint8_t currentByte = 0x00;
    bool currentPixelBit = 0;
    unsigned int byteIndex = 0;
    unsigned int bitIndexWithinByte = 0;  // MSB index == 7, LSB index == 1
    // top-left pixel = index 0, top-right = index 223, bottom-right = 57,343
    for(unsigned int I_2 = 0; I_2 < numPixels; I_2++){
        // Get the rotated index for the desired bit in VRAM that corresponds with the current pixel
        I_1 = ((I_2%W)+1)*H - (floor(I_2/W)+1);

        // Get the byte that contains the desired bit
        byteIndex = floor(I_1 / 8);
        currentByte = rotatedPixels[byteIndex];

        // Get desired bit
        bitIndexWithinByte = I_1 % 8;
        currentPixelBit = (currentByte >> bitIndexWithinByte) & 0x01;

        // Expand bit to 32 bits RGBA info
        if(currentPixelBit == 1){
            currentFramePixels[I_2] = 0xffffffff;
        }else{
            currentFramePixels[I_2] = 0x00000000;
        }
    }

    return currentFramePixels;
}