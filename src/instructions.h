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
 Technically a set of 8080 instructions: Push Register Pair
 1) PUSH B
 2) PUSH D
 3) PUSH H

 Also used for PUSH PSW as that instruction pushes: 1) The 8080
 flags assembled into a byte and 2) The accumulator. 
 However, the flags and accumulator are not considered 
 a register pair in the 8080 system manual.

 @param highReg - Higher order bits register
 @param lowReg - Lower order bits register
 @param state - The 8080 state
 */
void PUSH_RP(uint8_t highReg, uint8_t lowReg, State8080 *state);

/**
 Technically a set of 8080 instructions: Pop Register Pair
 1) POP B
 2) POP D
 3) POP H
 Not for the instruction "POP PSW"

 @param highReg - Higher order bits register
 @param lowReg - Lower order bits register
 @param state - The 8080 state
 */
void POP_RP(uint8_t *highReg, uint8_t *lowReg, State8080 *state);

/**
 Technically a set of 8080 instructions: Double Precision Add Register Pair (to HL)
 1) DAD B
 2) DAD D
 3) DAD H

 Flags: CY

 @param highReg - Higher order bits register
 @param lowReg - Lower order bits register
 @param state - The 8080 state 
 */
void DAD_RP(uint8_t highReg, uint8_t lowReg, State8080 *state);

/**
 The 8080 JMP instruction
 @param address - The 8080 address to jump to
 @param state - The 8080 state
 */
void JMP(uint16_t address, State8080 *state);

/**
 A set of 8080 instructions covering register variants of XOR with Accumulator:
 1) XRA A
 2) XRA B
 3) XRA C
 4) XRA D
 5) XRA E
 6) XRA H
 7) XRA L

 Flags: z,s,p,cy(reset),ac(reset)

 @param data - The 8 bits to XOR with accumulator
 @param state - The 8080 state
 */
void XRA_R(uint8_t data, State8080 *state);

/**
 A set of 8080 instructions covering register variants of AND with Accumulator:
 1) ANA A
 2) ANA B
 3) ANA C
 4) ANA D
 5) ANA E
 6) ANA H
 7) ANA L

 Flags: z,s,p,cy(reset),ac

 @param data - The 8 bits to AND with accumulator
 @param state - The 8080 state
 */
void ANA_R(uint8_t data, State8080 *state);

/**
 * A set of 8080 instructions for Restarting with different subroutines:
 * RST n
 * n: 0 -> 7
 *
 * @param restartNumber - "n", the subroutine number to execute
 * @param state - The 8080 state
 */
void RST(uint8_t restartNumber, State8080 *state);

/**
 * RET
 * Return to address on stack
 *
 * @param state - The 8080 state
 */
void RET(State8080 *state);

/**
 * A set of 8080 instructions covering register variants of OR with Accumulator:
 * 1) ORA A
 * 2) ORA B
 * 3) ORA C
 * 4) ORA D
 * 5) ORA E
 * 6) ORA H
 * 7) ORA L
 *
 * @param data - The 8 bits to OR with Accumulator
 * @param state - The 8080 state
 */
void ORA_R(uint8_t data, State8080 *state);

/**
 * A set of 8080 instructions for decrementing register pairs:
 * 1) DCX B
 * 2) DCX D
 * 3) DCX H
 *
 * Flags: none
 *
 * @param highReg - Higher order bits register
 * @param lowReg - Lower order bits register
 * @param state - The 8080 state
 */
void DCX_RP(uint8_t *highReg, uint8_t *lowReg, State8080 *state);

/**
 * A set of 8080 instructions for moving data from one register to another
 *
 * r2 = r1
 * r1: A,B,C,D,E,H,L
 * r2: A,B,C,D,E,H,L
 *
 * @param destReg - destination register
 * @param sourceReg - source register
 * @param state - The 8080 state
 */
void MOV_R1_R2(uint8_t *r1, uint8_t *r2, State8080 *state);

/**
 * A set of 8080 instructions for moving immediate data into a register:
 * 1) MVI A, D8
 * 2) MVI B, D8
 * 3) MVI C, D8
 * 4) MVI D, D8
 * 5) MVI E, D8
 * 6) MVI H, D8
 * 7) MVI L, D8
 *
 * @param destReg - destination register
 * @param value - immediate value
 * @param state - The 8080 state
 */
void MVI_R(uint8_t *destReg, uint8_t value, State8080 *state);

/**
 * A set of 8080 instructions for incrementing a register's value:
 * 1) INR A
 * 2) INR B
 * 3) INR C
 * 4) INR D
 * 5) INR E
 * 6) INR H
 * 7) INR L
 *
 * @param reg - Target register
 * @param state - The 8080 state
 */
void INR_R(uint8_t *reg, State8080 *state);

/**
 * Performs the function of OR with the Accumulator
 *
 * Flags: z,s,p,cy(reset),ac(reset)
 *
 * @param data - The 8 bits to OR with Accumulator
 * @param state - The 8080 state
 */
void orWithAccumulator(uint8_t data, State8080 *state);

/**
 Performs the function of XOR with the Accumulator

 Flags: z,s,p,cy(reset),ac(reset)

 @param data - The 8 bits to XOR with Accumulator
 @param state - The 8080 state
 */
void xorWithAccumulator(uint8_t data, State8080 *state);

/**
 Performs the function of AND with the Accumulator

 Flags: z,s,p,cy(reset),ac

 @param data - The 8 bits to AND with Accumulator
 @param state - The 8080 state
 */
void andWithAccumulator(uint8_t data, State8080 *state);

/**
 Returns the 16-bit value yielded by concatenating the 'h' (high) register with the 'l' (low) register
 @param state - The 8080 state
 @return - Address bits in order (h)(l)
*/
uint16_t getValueHL(State8080 *state);

/**
 Returns the 16-bit value yielded by concatenating the 'd' (high) register with the 'e' (low) register
 @param state - The 8080 state
 @return - Address bits in order (d)(e)
*/
uint16_t getValueDE(State8080 *state);

/**
 For the set of register-based MOV-variants
 Copy data to an 8080 memory location determined by the contents of the H-L register pair
 @param data - Data to move
 @param state - The 8080 state
*/
void moveDataToHLMemory(uint8_t data, State8080 *state);

/**
 For the set of memory-based MOV-variants
 Load data from an 8080 memory location determined by the contents of the H-L register pair
 
 @param destination - pointer to the host machine address to store data
 @param
 */
void moveDataFromHLMemory(uint8_t *destination, State8080 *state);

/**
 Performs subtraction of two numbers while setting/resetting the carry flag.

 @param minuend - the value to be subtracted from
 @param subtrahend - the value to subtract
 @return - the difference between the minuend and subtrahend, will only capture 8 bits
 */
uint8_t subWithCheckCY(int8_t minuend, int8_t subtrahend, State8080 *state);

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
 Add two values and set/reset Carry flag.

 @param op1 - 1st operand
 @param op2 - 2nd operand
 @return - sum of operands
 */
uint16_t addWithCheckCY(uint8_t op1, uint8_t op2, State8080 *state);

/**
 For auditing/debugging purposes.
 Simply changes 8080 memory to some value at some address.
 Makes life easier for tracking memory changes.
 Will not allow changes to ROM (0x0000 - 0x1FFF).
 @param address - 8080 memory address to edit
 @param value - Data to place at desired address
 @param state - 8080 state
 */
void writeMem(uint16_t address, uint8_t value, State8080 *state);

/**
 For auditing/debugging purposes.
 Simply returns a byte from some 8080 memory address.
 Makes life easier for tracking memory changes.
 @param address - 8080 memory address to edit
 @param state - 8080 state
 @return - byte of data from requested address
 */
uint8_t readMem(uint16_t address, State8080 *state);

#endif  // INSTRUCTIONS_H_