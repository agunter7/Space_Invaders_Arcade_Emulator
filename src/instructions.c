/**
 * Implements functions for Intel 8080 emulated instructions
 * Capitalized names indicate a function that implements a full 8080 instructions
 * camelCase names indicate a function that implements a portion of an instruction's effect(s)
 *
 * @author Andrew Gunter
 */

#include "../src/cpuStructures.h"
#include "../src/instructions.h"
#include "../src/helpers.h"

/**
 CALL addr
 Memory[SP-1] = PCH
 Memory[SP-2] = PCL
 SP = SP-2
 PC = (byte 3)(byte 2)
*/
void CALL(uint16_t address, State8080 *state)
{
    uint8_t pcHigh;  // program counter high 8 bits
    uint8_t pcLow;
    uint16_t pc = state->pc;
    uint16_t sp = state->sp;

    pcHigh = (uint8_t)(pc >> 8);
    pcLow = (uint8_t)(pc & 0x0F);

    editMem(sp-1, pcHigh, state);
    editMem(sp-2, pcLow, state);
    state->sp -= 2;
    state->pc = address;
}

/**
 INX rp
 (rh)(rl) = (rh)(rl)+1
*/
void INX_RP(uint8_t *highReg, uint8_t *lowReg, State8080 *state)
{
    uint16_t concatRegValue = (((uint16_t)(*highReg)) << 8) | ((uint16_t)(*lowReg));
    concatRegValue++;
    *highReg = (uint8_t)(concatRegValue >> 8);
    *lowReg = (uint8_t)concatRegValue;
    state->pc++;
}

/**
 JMP addr
 Jump to address
 pc = address
 */
void JMP(uint16_t address, State8080 *state)
{
    state->pc = address;
}

void moveDataToHLMemory(uint8_t data, State8080 *state)
{
    uint16_t destinationAddress = getAddressHL(state);
    editMem(destinationAddress, data, state);
}

uint16_t getAddressHL(State8080 *state)
{
    uint16_t highBits = (((uint16_t)(state->h))<<8);
    uint16_t lowBits = (uint16_t)(state->l);
    return (highBits | lowBits);
}

uint16_t getAddressDE(State8080 *state)
{
    uint16_t highBits = (((uint16_t)(state->d))<<8);
    uint16_t lowBits = (uint16_t)(state->e);
    return (highBits | lowBits);
}

void editMem(uint16_t address, uint8_t value, State8080 *state)
{
    /*if(address == 0x23ff || address == 0x23fe){
        logger("Change address 0x%04x to value 0x%02x\n", address, value);
    }*/
    state->memory[address] = value;
}

uint8_t getMem(uint16_t address, State8080 *state)
{
    /*if(address == 0x23ff || address == 0x23fe){
        logger("Read address 0x%04x to value 0x%02x\n", address, state->memory[address]);
    }*/
    return state->memory[address];
}

uint16_t addWithCheckAC(uint8_t op1, uint8_t op2, State8080 *state)
{
    // Perform lower-order 4-bit addition
    uint8_t nibble1 = op1 & 0x0F;
    uint8_t nibble2 = op2 & 0x0F;
    uint8_t nibbleResult = op1 + op2;

    if((nibbleResult & 0x10) == 0x10){
        // Carry occurred from bit 3 to bit 4
        state->flags.auxillaryCarry = 1;
    }else{
        state->flags.auxillaryCarry = 0;
    }

    // Return lossless result from 8-bit addition
    return ((uint16_t)op1 + (uint16_t)op2);
}

void checkStandardArithmeticFlags(uint8_t result, State8080 *state)
{
    // Check zero flag
    if ((result & 0xff) == 0){  
        state->flags.zero = 1;
    }else{
        state->flags.zero = 0;
    }

    // Check sign flag
    // Sign flag set when MSB (bit 7) is set, else reset
    // 0x80 == 1000 0000
    if((result & 0x80) == 0x80){
        state->flags.sign = 1;
    }else{
        state->flags.sign = 0;
    }

    //Check parity flag
    uint8_t mask = 0x01;
    unsigned int sum = 0;
    // Get sum of 1s in the 8-bit result
    for(int shift = 0; shift < 8; shift++){  // Each iteration targets a different bit from result
        uint8_t maskedResult = result & (mask << shift);
        sum += (maskedResult >> shift);
    }
    // Set for even parity, reset for odd parity
    if (sum % 2 == 0){
        state->flags.parity = 1;
    }else{
        state->flags.parity = 0;
    }
}