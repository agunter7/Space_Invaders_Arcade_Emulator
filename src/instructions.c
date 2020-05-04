/**
 * Implements functions for Intel 8080 emulated instructions
 * Capitalized names indicate a function that implements a full 8080 instructions
 * camelCase names indicate a function that implements a portion of an instruction's effect(s)
 *
 * By convention, only fully-implemented 8080 functions (Capitalized names) will update the program counter and
 * increase the number of machine cycles completed
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

    writeMem(sp-1, pcHigh, state);
    writeMem(sp-2, pcLow, state);
    state->sp -= 2;
    state->pc = address;
    state->cyclesCompleted += 17;
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
    state->cyclesCompleted += 5;
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

    writeMem(sp-1, highReg, state);
    writeMem(sp-2, lowReg, state);
    state->sp = sp-2;

    state->pc += 1;
    state->cyclesCompleted += 11;
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
    *lowReg = readMem(sp, state);
    *highReg = readMem(sp+1, state);
    state->sp = sp+2;

    state->pc += 1;
    state->cyclesCompleted += 10;
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
    state->cyclesCompleted += 10;
}

/**
 JMP addr
 Jump to address
 pc = address
 */
void JMP(uint16_t address, State8080 *state)
{
    state->pc = address;
    state->cyclesCompleted += 10;
}

/**
 XRA r
 Exclusive Or Accumulator with a register
 A = A XOR r
 Flags: z,s,p,cy(reset),ac(reset)
 */
void XRA_R(uint8_t data, State8080 *state)
{
    xorWithAccumulator(data, state);

    state->pc += 1;
    state->cyclesCompleted += 4;
}

/**
 ANA r
 AND Accumulator with register
 A = A AND r
 Flags: z,s,p,cy(reset),ac
 */
void ANA_R(uint8_t data, State8080 *state)
{
    andWithAccumulator(data, state);

    state->pc += 1;
    state->cyclesCompleted += 1;
}

/**
 * RST n
 * Restart with subroutine 'n'
 * PC = PC + 1
 * memory[sp-1] = PCH
 * memory[sp-2] = PCL
 * SP = SP - 2
 * PC = 8 * n
 */
void RST(uint8_t restartNumber, State8080 *state)
{
    uint8_t pcHigh;  // program counter higher 8 bits
    uint8_t pcLow;  // program counter lower 8 bits
    uint16_t pcToStore = (state->pc) + 1;
    uint16_t sp = state->sp;

    pcHigh = (uint8_t)(pcToStore >> 8);
    pcLow = (uint8_t)pcToStore;

    writeMem(sp-1, pcHigh, state);
    writeMem(sp-2, pcLow, state);
    state->sp -= 2;
    state->pc = 8 * restartNumber;
    state->cyclesCompleted += 11;
}

/**
 * RET
 * Return
 * PCL = memory[sp]
 * PCH = memory[sp+1]
 * SP = SP + 2
 */
void RET(State8080 *state)
{
    uint8_t lowByte = readMem(state->sp, state);
    uint8_t highByte = readMem((state->sp)+1, state);
    uint16_t newValuePC = (((uint16_t)highByte) << 8) | (uint16_t)lowByte;
    state->pc = newValuePC;

    state->sp += 2;
    state->cyclesCompleted += 10;
}

void xorWithAccumulator(uint8_t data, State8080 *state)
{
    state->a = state->a ^ data;

    checkStandardArithmeticFlags(state->a, state);
    state->flags.carry = 0;
    state->flags.auxiliaryCarry = 0;
}

/**
 * TODO: Figure out AC check
 */
void andWithAccumulator(uint8_t data, State8080 *state)
{
    state->a = state->a & data;

    checkStandardArithmeticFlags(state->a, state);
    state->flags.carry = 0;
    state->flags.auxiliaryCarry = 0;  // No clue if hard-resetting is correct, but I can't see how it would be set by AND
}

void moveDataToHLMemory(uint8_t data, State8080 *state)
{
    uint16_t destinationAddress = getValueHL(state);
    writeMem(destinationAddress, data, state);
}

void moveDataFromHLMemory(uint8_t *destination, State8080 *state)
{
    uint16_t sourceAddress = getValueHL(state);
    *destination = readMem(sourceAddress, state);
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

void writeMem(uint16_t address, uint8_t value, State8080 *state)
{
    if(address >= ROM_LIMIT_8080){
        state->memory[address] = value;
    }else{
        logger("Warning: Attempted to write to Intel 8080 ROM! Write attempt rejected!\n");
        logger("Address 0x%04x; Value 0x%02x\n", address, value);
    }
}

uint8_t readMem(uint16_t address, State8080 *state)
{
    return state->memory[address];
}

uint16_t addWithCheckAC(uint8_t op1, uint8_t op2, State8080 *state)
{
    // Perform lower-order 4-bit addition
    uint8_t nibbleResult = op1 + op2;

    if((nibbleResult & 0x10) == 0x10){
        // Carry occurred from bit 3 to bit 4
        state->flags.auxiliaryCarry = 1;
    }else{
        state->flags.auxiliaryCarry = 0;
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