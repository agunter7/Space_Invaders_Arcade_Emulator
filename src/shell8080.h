/***********************************************************************************
 *
 * Provides an API for interacting with the Intel 8080 emulator
 * @Author: Andrew Gunter
 *
***********************************************************************************/

#ifndef INTEL_8080_EMULATOR_SHELL8080_H
#define INTEL_8080_EMULATOR_SHELL8080_H

#include "cpuStructures.h"

/**
 * Returns a pointer to an emulated Intel 8080 cpu state
 * All of the cpu's registers and memory will be zeroed, except for ROM, which contains Space Invaders
 * @return Initialized 8080 state pointer
 */
State8080 *initializeCPU();

/**
 * Frees memory allocated for a State8080 struct
 * @param state - The 8080 state
 */
void destroyCPU(State8080 *state);

/**
 * Have an emulated 8080 CPU execute instructions up to the point of completing a certain number of clock cycles.
 * May execute up to 17 more cycles than explicitly instructed.
 * @param numCyclesToRun - Clock cycle threshold to execute
 * @param state - The 8080 state
 */
void runForCycles(unsigned int numCyclesToRun, State8080 *state);

/**
 * Returns a pointer to a copy of the 8080's current VRAM
 * @param state - The 8080 state
 * @return VRAM copy pointer
 */
uint8_t *getVideoRAM(State8080 *state);

#endif //INTEL_8080_EMULATOR_SHELL8080_H
