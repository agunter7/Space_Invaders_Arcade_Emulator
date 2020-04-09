/**
 * Declares functions for Intel 8080 emulated instructions
 * Capitalized names indicate a function that implements a full 8080 instructions
 * camelCase names indicate a function that implements a portion of an instruction's effect(s)
 *
 * By convention, only fully-implemented 8080 functions (Capitalized names) will update the program counter.
 *
 * @author Andrew Gunter
 */

#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

#include "../src/cpuStructures.h"

/**
 The 8080 CALL instruction
 @param address - The address in 8080 memory to be called
 @param state - The 8080 state
*/
void CALL(uint16_t address, State8080 *state);

/**
 Technically a set of 8080 instructions: Increment Register Pair
 1) INX B
 2) INX D
 3) INX H
 Not for the instruction "INX SP", as it does not increment a pair

 @param highReg - Higher order bits register
 @param lowReg - Lower order bits register
 @param state - The 8080 state
*/
void INX_RP(uint8_t *highReg, uint8_t *lowReg, State8080 *state);

/**
 The 8080 JMP instruction
 @param address - The 8080 address to jump to
 @param state - The 8080 state
 */
void JMP(uint16_t address, State8080 *state);

/**
 Returns the 16-bit address yielded by concatenating the 'h' (high) register with the 'l' (low) register
 @param state - The 8080 state
 @return - Address bits in order (h)(l)
*/
uint16_t getAddressHL(State8080 *state);

/**
 Returns the 16-bit address yielded by concatenating the 'd' (high) register with the 'e' (low) register
 @param state - The 8080 state
 @return - Address bits in order (d)(e)
*/
uint16_t getAddressDE(State8080 *state);

/**
 For the set of register-based MOV-variants
 Copy data to an 8080 memory location determined by the contents of the H-L register pair
 @param data - Data to move
 @param state - The 8080 state
*/
void moveDataToHLMemory(uint8_t data, State8080 *state);

/**
 Sets/Resets the most common flags for arithmetic instructions:
 Zero, Sign, Parity flags
 Will not affect Carry or Auxillary Carry flags
 */
void checkStandardArithmeticFlags(uint8_t result, State8080 *state);

/**
 Add two values and set/reset Auxillary Carry flag.

 Intel 8080 System Manual comments on Auxillary Carry:

 "If the instruction caused a carry out of bit 3 and into 
 bit 4 of the resulting value, the auxillary carry is set;
 otherwise it is reset. This flag is affected by single
 precision additions, subtractions, increments, decrements, 
 comparisons, and logical operations, but is principally
 used with additions and increments preceding a
 DAA (Decimal Adjust Accumulator) instruction."

 @param op1 - 1st operand
 @param op2 - 2nd operand
 @return - sum of operands
 */
uint16_t addWithCheckAC(uint8_t op1, uint8_t op2, State8080 *state);

/**
 For auditing/debugging purposes.
 Simply changes 8080 memory to some value at some address.
 Makes life easier for tracking memory changes.
 @param address - 8080 memory address to edit
 @param value - Data to place at desired address
 @param state - 8080 state
 */
void editMem(uint16_t address, uint8_t value, State8080 *state);

/**
 For auditing/debugging purposes.
 Simply returns a byte from some 8080 memory address.
 Makes life easier for tracking memory changes.
 @param address - 8080 memory address to edit
 @param state - 8080 state
 @return - byte of data from requested address
 */
uint8_t getMem(uint16_t address, State8080 *state);

#endif  // INSTRUCTIONS_H_