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

void checkStandardArithmeticFlags(uint16_t result, State8080 *state)
{
	// Check zero flag
	if (result == 0){
		state->flags.zero = 1;
	}else{
		state->flags.zero = 0;
	}

	// Check sign flag
	// A "true" 8080 arithmetic result is only 8 bits, but emulation here uses 16 bits
	// Using 16 bit results allows for easier carry handling/checking
	// Sign flag set when MSB (8th bit) is set, else reset
	// 0x0080 == 0000 0000 1000 0000
	if((result & 0x0080) == 0x0080){
		state->flags.sign = 1;
	}else{
		state->flags.sign = 0;
	}

	//Check parity flag
	uint8_t trueResult = (uint8_t)result;
	uint8_t mask = 0x01;
	unsigned int sum = 0;
	// Get sum of 1s in the 8-bit result
	for(int shift = 0; shift < 8; shift++){  // Each iteration targets a different bit from trueResult
		uint8_t maskedResult = trueResult & (mask << shift);
		sum += (maskedResult >> shift);
	}
	// Set for even parity, reset for odd parity
	if (sum % 2 == 0){
		state->flags.parity = 1;
	}else{
		state->flags.parity = 0;
	}
}