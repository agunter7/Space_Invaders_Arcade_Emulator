#include "../src/cpuStructures.h"

void CALL(uint16_t address, State8080 *state)
{
	// CALL addr
	/*
		Memory[SP-1] = PCH
		Memory[SP-2] = PCL
		SP = SP-2
		PC = (byte 3)(byte 2)
	*/
	uint8_t pcHigh;
	uint8_t pcLow;

	pcHigh = state->pc >> 8;
	pcLow = state->pc & 0x0F;

	state->memory[sp-1] = pcHigh;
	state->memory[sp-2] = pcLow;
	state->sp -= 2;
	state->pc = address;

	printf("CALL\n");
}