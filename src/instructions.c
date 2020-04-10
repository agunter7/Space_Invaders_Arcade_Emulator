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
 PC = PC + 3
 Memory[SP-1] = PCH
 Memory[SP-2] = PCL
 SP = SP-2
 PC = (byte 3)(byte 2)
*/
void CALL(uint16_t address, State8080 *state)
{
    uint8_t pcHigh;  // program counter high 8 bits
    uint8_t pcLow;
    uint16_t pcToStore = (state->pc) + 3;
    uint16_t sp = state->sp;

    pcHigh = (uint8_t)(pcToStore >> 8);
    pcLow = (uint8_t)pcToStore;

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
 PUSH rp
 Push register pair onto the stack
 memory[sp-1] = rh
 memory[sp-2] = rl
 sp = sp-2;
 */
void PUSH_RP(uint8_t highReg, uint8_t lowReg, State8080 *state)
{
    uint16_t sp = state->sp;

    editMem(sp-1, highReg, state);
    editMem(sp-2, lowReg, state);
    state->sp = sp-2;

    state->pc += 1;
}

/**
 POP rp
 Pop 2 bytes from the stack and store in a register pair
 rl = memory[sp]
 rh = memory[sp+1]
 sp = sp+2
 */
void POP_RP(uint8_t *highReg, uint8_t *lowReg, State8080 *state)
{
    uint16_t sp = state->sp;
    *lowReg = getMem(sp, state);
    *highReg = getMem(sp+1, state);
    state->sp = sp+2;

    state->pc += 1;
}

/**
 DAD rp
 Concatenate the values from a register pair
 Concatenate the values from register H and register L
 Add the two concatenated values
 Store the result in register pair HL
 (H)(L) = (H)(L) + (rh)(rl)
 */
void DAD_RP(uint8_t highReg, uint8_t lowReg, State8080 *state)
{
    uint32_t result = 0;
    uint16_t augend = getValueHL(state);
    uint16_t addend = ((uint16_t)highReg)<<8 | (uint16_t)lowReg;
    uint8_t newValueH;
    uint8_t newValueL;
    
    // Lossless addition
    result = (uint32_t)augend + (uint32_t)addend;

    // Carry check for overflow of 16-bit Addition
    // Do not set high for 8-bit overflow (which is the typical case in the 8080)
    if ((result & 0x00010000) == 0x00010000){
        state->flags.carry = 1;
    }else{
        state->flags.carry = 0;
    }

    newValueH = (uint8_t)(((uint16_t)result)>>8);
    newValueL = (uint8_t)result;
    state->h = newValueH;
    state->l = newValueL;

    state->pc += 1;
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
    uint16_t destinationAddress = getValueHL(state);
    editMem(destinationAddress, data, state);
}

uint16_t getValueHL(State8080 *state)
{
    uint16_t highBits = (((uint16_t)(state->h))<<8);
    uint16_t lowBits = (uint16_t)(state->l);
    return (highBits | lowBits);
}

uint16_t getValueDE(State8080 *state)
{
    uint16_t highBits = (((uint16_t)(state->d))<<8);
    uint16_t lowBits = (uint16_t)(state->e);
    return (highBits | lowBits);
}

void editMem(uint16_t address, uint8_t value, State8080 *state)
{
    /*if(address == 0x23ff || address == 0x23fe){
        logger("Change address 0x%04x to value 0x%02x\n", address, value);
    }*/ //debugCode
    state->memory[address] = value;
}

uint8_t getMem(uint16_t address, State8080 *state)
{
    /*if(address == 0x23ff || address == 0x23fe){
        logger("Read address 0x%04x to value 0x%02x\n", address, state->memory[address]);
    }*/  //debugCode
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

uint16_t addWithCheckCY(uint8_t op1, uint8_t op2, State8080 *state)
{
    uint16_t result = (uint16_t)op1 + (uint16_t)op2;

    if( result > 0xff){
        state->flags.carry = 1;
    }else{
        state->flags.carry = 0;
    }

    return result;
}

/**
 Perform subtraction by taking the 2's complement of the subtrahend and add it to the minuend.
 
 The 8080 system manual seems to imply that it performs subtraction differently:
 "All subtraction operations are performed via two's complement arithmetic and set the 
  carry flag to one to indicate a borrow and clear it to indicate no borrow."

  "Borrowing" doesn't really happen in 2's complement subtraction? I must be misunderstanding
  in some way, but I believe normally one performs subtraction via addition of a negated value.
  In this case, no "Borrowing" occurs, only carries. In such cases, a carry into the sign bit indicates
  an overflow and thus would trigger a sign change. This is the implementation used herein.
 */
uint8_t subWithCheckCY(int8_t minuend, int8_t subtrahend, State8080 *state)
{
    uint8_t result = 0x0000;
    // Change variable names for the sake of matching addition
    uint8_t augend = minuend;
    uint8_t addend = twosComplement(subtrahend);

    result = augend+addend;

    if(augend>>7 == 0x00 && addend>>7 == 0x00 && result>>7 != 0x00){
        // Addition of two positives did not yield positive, overflow occurred
        state->flags.carry = 1;
    }else if(augend>>7 == 0x01 && addend>>7 == 0x01 && result>>7 != 0x01){
        // Addition of two negatives did not yield negative, overflow occurred
        state->flags.carry = 1;
    }else{
        // Addition of a negative and positive, cannot overflow OR
        // Addition of pos+pos/neg+neg yielded pos/neg respectively, no overflow
        state->flags.carry = 0;
    }

    return result;
}

void checkStandardArithmeticFlags(uint8_t result, State8080 *state)
{
    // Check zero flag
    if (result == 0){  
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
    uint8_t maskedResult = 0;
    // Get sum of 1s in the 8-bit result
    for(int shift = 0; shift < 8; shift++){  // Each iteration targets a different bit from result
        maskedResult = result & (mask << shift);
        sum += (maskedResult >> shift);
    }
    // Set for even parity, reset for odd parity
    if (sum % 2 == 0){
        state->flags.parity = 1;
    }else{
        state->flags.parity = 0;
    }
}