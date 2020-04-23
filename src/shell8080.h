//
// Created by Andrew on 2020-04-23.
//

#ifndef INTEL_8080_EMULATOR_SHELL8080_H
#define INTEL_8080_EMULATOR_SHELL8080_H

#include "cpuStructures.h"

State8080 *initializeCPU();
void destroyCPU(State8080 *state);

#endif //INTEL_8080_EMULATOR_SHELL8080_H
