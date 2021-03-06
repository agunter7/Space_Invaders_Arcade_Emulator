/***********************************************************************************
 *
 * Emulates a Space Invaders Arcade Machine
 * @Author: Andrew Gunter
 *
***********************************************************************************/

#include "../src/arcadeEnvironment.h"

void playSpaceInvaders(ArcadeState *arcade);
unsigned int handleGameEvents(ArcadeState *arcade);
uint32_t *getCurrentFramePixels(ArcadeState *arcade);

int main(int argc, char **argv)
{
    ArcadeState *arcade = initializeArcade();

    if(arcade != NULL){
        playSpaceInvaders(arcade);
        destroyArcade(arcade);
    }

    return 0;
}

/**
 * Starts the main game loop.
 * @param arcade - The arcade state
 */
void playSpaceInvaders(ArcadeState *arcade)
{
    unsigned int quitGame = 0;

    // The texture below is initialized in ABGR32 pixel format,
    // but this will actually lead to writing pixel data
    // in RGBA32 format. Maybe due to endianness of the
    // host machine's video RAM?
    // Is this platform-dependent?
    SDL_Texture *texture = SDL_CreateTexture(arcade->renderer, SDL_PIXELFORMAT_ABGR32,
            SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);

    uint32_t *currentFramePixels;
    while (!quitGame){

        quitGame = handleGameEvents(arcade);

        // Clear screen
        SDL_RenderClear(arcade->renderer);

        // Load/render window image
        currentFramePixels = getCurrentFramePixels(arcade);
        SDL_UpdateTexture(texture, NULL, currentFramePixels, SCREEN_WIDTH_PIXELS*BYTES_PER_PIXEL);
        SDL_RenderCopy(arcade->renderer, texture, NULL, NULL);
        free(currentFramePixels);

        // Update screen
        SDL_RenderPresent(arcade->renderer);
    }
}

/**
 * Processes all game actions for a single frame.
 * @param arcade - The arcade state
 * @return - 1 if the game should end, 0 otherwise
 */
unsigned int handleGameEvents(ArcadeState *arcade)
{
    resetPortsIO(arcade);

    // Get keyboard state to check for continuously-pressed keys
    const uint8_t *keyboardState = SDL_GetKeyboardState(NULL);
    if(keyboardState[SDL_SCANCODE_LEFT]){
        arcade->inputPort0 |= MOVE_LEFT_MASK;
        arcade->inputPort1 |= MOVE_LEFT_MASK;
        arcade->inputPort2 |= MOVE_LEFT_MASK;
    }
    if(keyboardState[SDL_SCANCODE_RIGHT]){
        arcade->inputPort0 |= MOVE_RIGHT_MASK;
        arcade->inputPort1 |= MOVE_RIGHT_MASK;
        arcade->inputPort2 |= MOVE_RIGHT_MASK;
    }
    if(keyboardState[SDL_SCANCODE_SPACE]){
        arcade->inputPort0 |= SHOOT_MASK;
        arcade->inputPort1 |= SHOOT_MASK;
        arcade->inputPort2 |= SHOOT_MASK;
    }
    if(keyboardState[SDL_SCANCODE_1]){
        arcade->inputPort1 |= P1_START_MASK;
    }
    if(keyboardState[SDL_SCANCODE_2]){
        arcade->inputPort1 |= P2_START_MASK;
    }

    // Check for newly-pressed keys
    SDL_Event currentEvent;
    while(SDL_PollEvent(&currentEvent) != 0){
        if(currentEvent.type == SDL_QUIT){
            logger("Quitting game\n");
            return 1;
        }

        // User inputs placed inside input ports
        if(currentEvent.type == SDL_KEYDOWN){  // key was pressed
            switch(currentEvent.key.keysym.sym){
                case SDLK_LEFT:
                    arcade->inputPort0 |= MOVE_LEFT_MASK;
                    arcade->inputPort1 |= MOVE_LEFT_MASK;
                    arcade->inputPort2 |= MOVE_LEFT_MASK;
                    break;
                case SDLK_RIGHT:
                    arcade->inputPort0 |= MOVE_RIGHT_MASK;
                    arcade->inputPort1 |= MOVE_RIGHT_MASK;
                    arcade->inputPort2 |= MOVE_RIGHT_MASK;
                    break;
                case SDLK_SPACE:
                    arcade->inputPort0 |= SHOOT_MASK;
                    arcade->inputPort1 |= SHOOT_MASK;
                    arcade->inputPort2 |= SHOOT_MASK;
                    break;
                case SDLK_1:
                    arcade->inputPort1 |= P1_START_MASK;
                    break;
                case SDLK_2:
                    arcade->inputPort1 |= P2_START_MASK;
                    break;
                case SDLK_3:
                    arcade->colourProfile = BlackAndWhite;
                    break;
                case SDLK_4:
                    arcade->colourProfile = Original;
                    break;
                case SDLK_5:
                    arcade->colourProfile = Inverted;
                    break;
                case SDLK_6:
                    arcade->colourProfile = Spectrum1;
                    break;
                case SDLK_7:
                    arcade->colourProfile = Spectrum2;
                    break;
                case SDLK_8:
                    arcade->colourProfile = Spectrum3;
                    break;
                case SDLK_9:
                    arcade->colourProfile = Spectrum4;
                    break;
                case SDLK_0:
                    arcade->colourProfile = Rainbow;
                    break;
                case SDLK_c:
                    arcade->inputPort1 |= CREDIT_MASK;
                    break;
                case SDLK_d:
                    if(arcade->colourProfile > Original){
                        arcade->darkModeOn = !(arcade->darkModeOn);
                    }
            }
        }
    }

    updateShiftRegister(arcade);

    // The physical Space Invaders hardware used analog audio
    // This means the signal triggering a given sfx was high (1) for
    // the full duration of the sfx
    // Playing sfx on every frame a signal is high would lead to repeated noises
    // Hence, we check for rising edges (0 -> 1) to indicate that sfx needs to be played
    bool ufoRisingEdge = false;
    bool ufoFallingEdge = false;  // UFO music loops, so a falling edge indicates music should stop
    bool playerShootRisingEdge = false;
    bool playerDieRisingEdge = false;
    bool invaderDieRisingEdge = false;
    bool fleetMove1RisingEdge = false;
    bool fleetMove2RisingEdge = false;
    bool fleetMove3RisingEdge = false;
    bool fleetMove4RisingEdge = false;
    bool ufoDieRisingEdge = false;
    // Get signals' states at start of frame
    // If signal low, assume rising edge and prove/disprove at end of frame
    // UFO Music Only: If signal high, assume falling edge and prove/disprove at end of frame
    if(((arcade->outputPort3) & UFO_MASK)  == 0x00){
        ufoRisingEdge = true;
    }
    if(((arcade->outputPort3) & UFO_MASK)  == UFO_MASK){
        ufoFallingEdge = true;
    }
    if(((arcade->outputPort3) & PLAYER_SHOOT_MASK)  == 0x00){
        playerShootRisingEdge = true;
    }
    if(((arcade->outputPort3) & PLAYER_DIE_MASK)  == 0x00){
        playerDieRisingEdge = true;
    }
    if(((arcade->outputPort3) & INVADER_DIE_MASK)  == 0x00){
        invaderDieRisingEdge = true;
    }
    if(((arcade->outputPort5) & FLEET_MOVE_1_MASK)  == 0x00){
        fleetMove1RisingEdge = true;
    }
    if(((arcade->outputPort5) & FLEET_MOVE_2_MASK)  == 0x00){
        fleetMove2RisingEdge = true;
    }else{
    }
    if(((arcade->outputPort5) & FLEET_MOVE_3_MASK)  == 0x00){
        fleetMove3RisingEdge = true;
    }
    if(((arcade->outputPort5) & FLEET_MOVE_4_MASK)  == 0x00){
        fleetMove4RisingEdge = true;
    }
    if(((arcade->outputPort5) & UFO_DIE_MASK)  == 0x00){
        ufoDieRisingEdge = true;
    }

    // Emulate cpu up to the known point of the mid-screen render interrupt
    // Screen width is used here, rather than height, as the Space Invaders screen is rotated 90 degrees and is
    // thus rendering vertical lines rather than horizontal lines
    unsigned int numCyclesFirstHalf = CYCLES_PER_FRAME*((float)MIDSCREEN_INTERRUPT_LINE/(float)SCREEN_WIDTH_PIXELS);
    runForCpuCycles(numCyclesFirstHalf, arcade);

    // Trigger mid-screen interrupt
    generateInterrupt(0x01, arcade->cpu);  // mid-screen

    // Emulate cpu up to the end of the frame
    unsigned int numCyclesSecondHalf = CYCLES_PER_FRAME-numCyclesFirstHalf;
    runForCpuCycles(numCyclesSecondHalf, arcade);

    // Trigger end-of-screen vertical blank interrupt
    generateInterrupt(0x02, arcade->cpu);

    // Stop UFO background music if a falling edge is confirmed
    if(ufoFallingEdge && (((arcade->outputPort3) & UFO_MASK) == 0x00)){
        if(Mix_PlayingMusic()){  // Check that music is playing, to be safe
            Mix_HaltMusic();
        }
    }
    // Play sound clips or start UFO background music if audio signal rising edges are confirmed
    if(ufoRisingEdge && (((arcade->outputPort3) & UFO_MASK)  == UFO_MASK)){
        Mix_PlayMusic(arcade->ufoMusic, -1);
    }
    if(playerShootRisingEdge && (((arcade->outputPort3) & PLAYER_SHOOT_MASK)  == PLAYER_SHOOT_MASK)){
        Mix_PlayChannel(-1, arcade->playerShootSfx, 0);
    }
    if(playerDieRisingEdge && (((arcade->outputPort3) & PLAYER_DIE_MASK)  == PLAYER_DIE_MASK)){
        Mix_PlayChannel(-1, arcade->playerDieSfx, 0);
    }
    if(invaderDieRisingEdge && (((arcade->outputPort3) & INVADER_DIE_MASK)  == INVADER_DIE_MASK)){
        Mix_PlayChannel(-1, arcade->invaderDieSfx, 0);
    }
    if(fleetMove1RisingEdge && (((arcade->outputPort5) & FLEET_MOVE_1_MASK)  == FLEET_MOVE_1_MASK)){
        Mix_PlayChannel(-1, arcade->fleetMove1Sfx, 0);
    }
    if(fleetMove2RisingEdge && (((arcade->outputPort5) & FLEET_MOVE_2_MASK)  == FLEET_MOVE_2_MASK)){
        Mix_PlayChannel(-1, arcade->fleetMove2Sfx, 0);
    }
    if(fleetMove3RisingEdge && (((arcade->outputPort5) & FLEET_MOVE_3_MASK)  == FLEET_MOVE_3_MASK)){
        Mix_PlayChannel(-1, arcade->fleetMove3Sfx, 0);
    }
    if(fleetMove4RisingEdge && (((arcade->outputPort5) & FLEET_MOVE_4_MASK)  == FLEET_MOVE_4_MASK)){
        Mix_PlayChannel(-1, arcade->fleetMove4Sfx, 0);
    }
    if(ufoDieRisingEdge && (((arcade->outputPort5) & UFO_DIE_MASK)  == UFO_DIE_MASK)){
        Mix_PlayChannel(-1, arcade->ufoDieSfx, 0);
    }

    return 0;
}

/**
 * Returns a pointer to data (32-bits-per-pixel) for the current frame to be rendered (by extracting from 8080 VRAM)
 * @param arcade - The arcade state
 * @return Pointer to pixel data, ready to be rendered directly by SDL
 */
uint32_t *getCurrentFramePixels(ArcadeState *arcade)
{
    // get rotated pixel data from cpu
    // 1 bit per pixel
    uint8_t *rotatedPixels = getVideoRAM(arcade->cpu);

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
    // The render-order pixel indices are on the left of each colon while the rotated versions from the 8080 VRAM
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
    uint32_t *currentFramePixels = mallocSet(numPixelBytes);  // Pointer to data describing 32-bit pixels for current frame

    // Get 32-bit pixel data for entire frame by iterating through pixels to be rendered on screen
    unsigned int I_1;
    unsigned int y;
    unsigned int x;
    unsigned int W = SCREEN_WIDTH_PIXELS;
    unsigned int H = SCREEN_HEIGHT_PIXELS;
    uint8_t currentByte = 0x00;
    bool currentPixelBit = 0;
    unsigned int byteIndex = 0;
    unsigned int bitIndexWithinByte = 0;  // MSB index == 7, LSB index == 0
    // top-left pixel = index 0, top-right = index 223, bottom-right = 57,343
    for(unsigned int I_2 = 0; I_2 < numPixels; I_2++){
        // Get the rotated index for the desired bit in VRAM that corresponds with the current pixel
        x = I_2%W;
        y = floor(I_2/W);
        I_1 = (x+1)*H - (y+1);

        // Get the byte that contains the desired bit
        byteIndex = floor(I_1 / 8);
        currentByte = rotatedPixels[byteIndex];

        // Get desired bit
        bitIndexWithinByte = I_1 % 8;
        currentPixelBit = (currentByte >> bitIndexWithinByte) & 0x01;

        // Expand bit to 32 bits RGBA info
        // Insert colour data depending on active colour profile
        if(currentPixelBit == 1){
            // Pixel is on
            uint32_t R;
            uint32_t G;
            uint32_t B;
            switch(arcade->colourProfile){
                case BlackAndWhite:
                    currentFramePixels[I_2] = WHITE_PIXEL;
                    break;
                case Inverted:
                    currentFramePixels[I_2] = BLACK_PIXEL;
                    break;
                case Original:
                    // True to Space Invaders arcade machine with transparent colour overlays
                    if(y<64 && y > 31){  // UFO
                        currentFramePixels[I_2] = RED_PIXEL;
                    }else if(y>191){  // Player and Shields
                        currentFramePixels[I_2] = GREEN_PIXEL;
                    }else{
                        currentFramePixels[I_2] = WHITE_PIXEL;
                    }
                    break;
                case Spectrum1:
                    // Vary R vertically, G horizontally, B diagonally
                    R = ((uint32_t)y) << 24;
                    G = (0x000000ff & ((((uint32_t)x) * 255)/223)) << 16;
                    B = (((255.0-(float)y)) + ((223.0-(float)x))) * (255.0/478.0);
                    if(B > 255.0){
                        B = 0x0000ff00;
                    }else{
                        B = ((uint32_t)B) << 8;
                    }
                    currentFramePixels[I_2] = R | G | B ;
                    break;
                case Spectrum2:
                    // Vary R horizontally, G diagonally, B vertically
                    R = (0x000000ff & ((((uint32_t)x) * 255)/223)) << 24;
                    G = (((255.0-(float)y)) + ((223.0-(float)x))) * (255.0/478.0);
                    B = ((uint32_t)y) << 8;
                    if(G > 255.0){
                        G = 0x00ff0000;
                    }else{
                        G = ((uint32_t)G) << 16;
                    }
                    currentFramePixels[I_2] = R | G | B ;
                    break;
                case Spectrum3:
                    // Vary R diagonally, G vertically, B horizontally
                    R = (((255.0-(float)y)) + ((223.0-(float)x))) * (255.0/478.0);
                    G = ((uint32_t)y) << 16;
                    B = (0x000000ff & ((((uint32_t)x) * 255)/223)) << 8;
                    if(R > 255.0){
                        R = 0xff000000;
                    }else{
                        R = ((uint32_t)R) << 24;
                    }
                    currentFramePixels[I_2] = R | G | B ;
                    break;
                case Spectrum4:
                    // Color Map
                    //   y     R    G     B
                    //   0    215   45   125
                    //  20    255   85    85
                    //  21    255   87    83
                    //  62    173  169    1
                    //  63    171  171    1
                    //  105    87  255    85
                    //  106    85  255    87
                    //  148    1   171    171
                    //  149    1   169    173
                    //  190    83   87    255
                    //  191    85   85    255
                    //  233   169   1     171
                    //  234   234   1     169
                    //  255   213   43    127
                    //
                    // For each colour dimension (R,G,B),
                    // the value changes by either +2 or -2 between
                    // adjacent vertical pixels. Rows are constant.
                    if(y <= 20){
                        R = 215 + (2*y);
                        G = 45 + (2*y);
                        B = 125 - (2*y);
                    }else if(y <= 62){
                        R = 297 - (2*y);
                        G = 45 + (2*y);
                        B = 125 - (2*y);
                    }else if(y <= 105){
                        R = 297 - (2*y);
                        G = 45 + (2*y);
                        B = (-125) + (2*y);
                    }else if(y <= 148){
                        R = 297 - (2*y);
                        G = 467 - (2*y);
                        B = (-125) + (2*y);
                    }else if(y <= 190){
                        R = (-297) + (2*y);
                        G = 467 - (2*y);
                        B = (-125) + (2*y);
                    }else if(y <= 233){
                        R = (-297) + (2*y);
                        G = 467 - (2*y);
                        B = 637 - (2*y);
                    }else{
                        R = (-297) + (2*y);
                        G = (-467) + (2*y);
                        B = 637 - (2*y);
                    }
                    R <<= 24;
                    G <<= 16;
                    B <<= 8;
                    currentFramePixels[I_2] = R | G | B ;
                    break;
                case Rainbow:
                    if(y < 36){
                        currentFramePixels[I_2] = VIOLET_PIXEL;
                    }else if(y <= 72){
                        currentFramePixels[I_2] = INDIGO_PIXEL;
                    }else if(y <= 106){
                        currentFramePixels[I_2] = BLUE_PIXEL;
                    }else if(y <= 143){
                        currentFramePixels[I_2] = GREEN_PIXEL;
                    }else if(y <= 178){
                        currentFramePixels[I_2] = YELLOW_PIXEL;
                    }else if(y <= 214){
                        currentFramePixels[I_2] = ORANGE_PIXEL;
                    }else{
                        currentFramePixels[I_2] = RED_PIXEL;
                    }
                    break;
            }
        }else{
            // Pixel is off
            if(arcade->colourProfile == Inverted){
                currentFramePixels[I_2] = WHITE_PIXEL;
            }else if(arcade->colourProfile == BlackAndWhite){
                currentFramePixels[I_2] = BLACK_PIXEL;
            }else if(arcade->colourProfile == Original){
                currentFramePixels[I_2] = BLACK_PIXEL;
            }else{
                if(arcade->darkModeOn){
                    currentFramePixels[I_2] = BLACK_PIXEL;
                }else{
                    currentFramePixels[I_2] = WHITE_PIXEL;
                }
            }
        }
    }

    free(rotatedPixels);
    return currentFramePixels;
}