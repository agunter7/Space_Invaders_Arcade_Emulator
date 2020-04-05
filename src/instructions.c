/**
 * Implements functions for Intel 8080 emulated instructions
 * Capitalized names indicate a function that implements a full 8080 instructions
 * camelCase names indicate a function that implements a portion of an instruction's effect(s)
 *
 * @author Andrew Gunter
 */

#include "../src/cpuStructures.h"
#include "../src/instructions.h"

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

	pcHigh = pc >> 8;
	pcLow = pc & 0x0F;

	state->memory[sp-1] = pcHigh;
	state->memory[sp-2] = pcLow;
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
	*highReg = concatRegValue >> 8;
	*lowReg = (uint8_t)concatRegValue;
	state->pc++;
}

void moveDataToHLMemory(uint8_t data, State8080 *state)
{
	uint16_t destinationAddress = getAddressHL(state);
	state->memory[destinationAddress] = data;
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