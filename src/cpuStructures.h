#ifndef CPUSTRUCTURES_H_
#define CPUSTRUCTURES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* 
Intel 8080 condition codes are held in an 8-bit register.
The various bits of this register correspond with different flags/conditions.
*/
typedef struct ConditionCodes {
    // Define register bit-field
    uint8_t    zero:1;  
    uint8_t    sign:1;  
    uint8_t    parity:1;    
    uint8_t    carry:1;    
    uint8_t    auxillaryCarry:1;    
    uint8_t    unusedBits:3;
} ConditionCodes;

typedef struct State8080 {
    uint8_t *memory;
    ConditionCodes flags;
    // Registers
    uint8_t    a;    
    uint8_t    b;    
    uint8_t    c;    
    uint8_t    d;    
    uint8_t    e;    
    uint8_t    h;    
    uint8_t    l;    
    uint16_t    sp;
    uint16_t    pc;
} State8080;

#endif  // CPUSTRUCTURES_H_