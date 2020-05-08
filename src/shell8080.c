/***********************************************************************************
* Emulates an Intel 8080 CPU's internal state and instruction execution
* @Author: Andrew Gunter
***********************************************************************************/

#include "../src/instructions.h"
#include "../src/cpuStructures.h"
#include "../src/helpers.h"

// Global variable definitions and function prototypes
bool debug = 0;
char instructions[256][20];
char instructionSizes[256];
char instructionFlags[256][20];
char instructionFunctions[256][100];
void initializeGlobals();
uint8_t *getRomBuffer(FILE *romFile);
void executeInstructionByOpcode(uint8_t opcode, uint8_t *operands, State8080 *state);
void runCodeFromBuffer(uint8_t *romBuffer);
void printInstructionInfo(uint8_t opcode);
void executeNextInstruction(State8080 *state);


State8080 *initializeCPU()
{
    uint8_t *romBuffer;  // Buffer for storing bytes read from Space Invaders ROM
    initializeGlobals();

    // Open Space Invaders ROM file as binary-read-only and store contents in a buffer
    FILE *invadersFile = fopen("resources/invaders", "rb");
    if(invadersFile == NULL){
        logger("Failed to open Space Invaders ROM.");
        return NULL;
    }
    romBuffer = getRomBuffer(invadersFile);

    // Initialize an 8080 state variable
    State8080 *state = malloc(sizeof(State8080));
    state->memory = malloc(MEMORY_SIZE_8080);  // Intel 8080 uses 16-bit byte-addressable memory, 2^16=65536
    ConditionCodes cc = {0};
    state->flags = cc;
    state->inputBuffers = malloc(NUM_INPUT_DEVICES);
    state->outputBuffers = malloc(NUM_OUTPUT_DEVICES);
    state->a = 0;
    state->b = 0;
    state->c = 0;
    state->d = 0;
    state->e = 0;
    state->h = 0;
    state->l = 0;
    state->sp = 0;
    state->pc = 0;
    state->cyclesCompleted = 0;
    state->interruptsEnabled = 0;
    // Set 8080 memory to known value
    memset(state->memory, 0, MEMORY_SIZE_8080);
    // Place ROM buffer data into CPU memory
    memcpy(state->memory, romBuffer, ROM_LIMIT_8080);

    free(romBuffer);
    return state;
}

void destroyCPU(State8080 *state)
{
    free(state->memory);
    free(state->inputBuffers);
    free(state->outputBuffers);
    free(state);
}

void runForCycles(unsigned int numCyclesToRun, State8080 *state)
{
    unsigned int startingCycles = state->cyclesCompleted;
    while((state->cyclesCompleted - startingCycles) < numCyclesToRun){
        executeNextInstruction(state);
        /*printf("%d\n", startingCycles);
        printf("%d\n", state->cyclesCompleted);*/
    }
}

void executeNextInstruction(State8080 *state)
{
    if(state->pc < ROM_LIMIT_8080){
        uint8_t operation = 0;  // next instruction opcode
        uint8_t operands[2] = {0xff, 0xff};  // next instruction operands, default 0xff as it would standout more than 0x00
        unsigned int instructionSize = 0;
        bool loggerFlag = 0;

        // Get next operation
        operation = state->memory[state->pc];

        // Get operands depending on instruction size
        instructionSize = instructionSizes[operation];  // Array is ordered based on opcode
        uint16_t lastOperandAddress = (state->pc)+instructionSize-1;
        int operandNum;
        for(uint16_t operandAddress = state->pc+1; operandAddress <= lastOperandAddress; operandAddress++){
            operandNum = operandAddress-(state->pc+1);
            operands[operandNum] = state->memory[operandAddress];
        }

        // Perform some debugging logging if desired. Triggered by global flag.
        if(debug){
            /*logger("%d\n", instrCount);
            if (instrCount == 100000){  // Helps to isolate a desired program section, just change value to meet needs
                loggerFlag = 1;
            }*/
            loggerFlag = 1;
            if(loggerFlag){
                // Print some status info
                logger("Operation: 0x%02x  %02x %02x\n", operation, operands[0], operands[1]);
                logger("A: 0x%02x, B: 0x%02x, C: 0x%02x, D: 0x%02x, E: 0x%02x, H: 0x%02x, L: 0x%02x\n",
                       state->a, state->b, state->c, state->d, state->e, state->h, state->l);
                logger("PC: 0x%04x, SP: 0x%04x, FLAGS (z,s,p,ac, c): ", state->pc, state->sp);
                logger("%1x%1x%1x%1x%1x\n",
                       state->flags.zero, state->flags.sign, state->flags.parity,
                       state->flags.auxiliaryCarry, state->flags.carry);
                /*char garbage[100];
                scanf("%s", garbage);*/
            }
        }

        executeInstructionByOpcode(operation, operands, state);
    }
}

uint8_t *getVideoRAM(State8080 *state)
{
    uint8_t *vramContents = malloc(VRAM_SIZE_8080);

    memcpy(vramContents, &(state->memory[VRAM_START_ADDR_8080]), VRAM_SIZE_8080);

    return vramContents;
}

void generateInterrupt(uint8_t interruptNum, State8080 *state)
{
    if(interruptNum < 8){
        // Will need to trigger instruction RST n
        // n = interrupt num

        // Opcode for RST instructions is of form (11NNN111)b
        // NNN is the binary representation of the interrupt number
        uint8_t interruptOpcode = 0xc7 | (interruptNum << 3);  // 0xc7 == (11000111)b
        uint8_t fakeOperands[2] = {0xff, 0xff};

        executeInstructionByOpcode(interruptOpcode, fakeOperands, state);
    }else{
        logger("Warning: Invalid interrupt attempted!\n");
    }
}

void runCodeFromBuffer(uint8_t *romBuffer)
{
    State8080 state = {
        .memory = malloc(MEMORY_SIZE_8080),
        .flags = {0},
        .a = 0,
        .b = 0,
        .c = 0,
        .d = 0,
        .e = 0,
        .h = 0,
        .l = 0,
        .sp = 0,
        .pc = 0,
    };  // Intel 8080 CPU initial state

    // Place ROM buffer data into CPU memory
    memcpy(state.memory, romBuffer, 0x2000);

    // Get next instruction and execute
    uint8_t operation = 0;
    uint8_t operands[2] = {0, 0};
    unsigned int instructionSize = 0;
    unsigned int instrCount = 0;
    bool loggerFlag = 0;
    while(state.pc < 0x2000){  // Keep within bounds of ROM data for 8080 memory map
        // Reset operands, 0xff chosen as it will likely standout as a reset value more than 0x00 would
        operands[0] = 0xff;
        operands[1] = 0xff;

        // Get next operation
        operation = romBuffer[state.pc];

        // Get operands depending on instruction size
        instructionSize = instructionSizes[operation];  // Array is ordered based on opcode
        int lastOperandAddress = state.pc+instructionSize-1;
        int operandNum;
        for(int operandAddress = state.pc+1; operandAddress <= lastOperandAddress; operandAddress++){
              operandNum = operandAddress-(state.pc+1);
              operands[operandNum] = romBuffer[operandAddress];
		}
        
        // Perform some debugging logging if desired. Triggered by global flag.
        if(debug){
            logger("%d\n", instrCount);
            if (instrCount == 100000){  // Helps to isolate a desired program section, just change value to meet needs
                loggerFlag = 1;
		    }
            if(loggerFlag){
                // Print some status info
                logger("Operation: 0x%02x  %02x %02x\n", operation, operands[0], operands[1]);
                logger("A: 0x%02x, B: 0x%02x, C: 0x%02x, D: 0x%02x, E: 0x%02x, H: 0x%02x, L: 0x%02x\n",
                        state.a, state.b, state.c, state.d, state.e, state.h, state.l);
                logger("PC: 0x%04x, SP: 0x%04x, FLAGS (z,s,p,ac, c): ", state.pc, state.sp);
                logger("%1x%1x%1x%1x%1x\n",
                        state.flags.zero, state.flags.sign, state.flags.parity,
                        state.flags.auxiliaryCarry, state.flags.carry);
                char garbage[100];
                scanf("%s", garbage);
	        }
        }

        executeInstructionByOpcode(operation, operands, &state);
        instrCount++;
	}

    free(state.memory);
}

/**
 * Returns a pointer to the stored binary derived from an input FILE
 * @param romFile - pointer to FILE whose data should be stored
 * @return - uint8_t pointer to stored contents
 */
uint8_t *getRomBuffer(FILE *romFile)
{
    int romSizeInBytes = 0;
    uint8_t *romBuffer;

    // Get ROM size
    fseek(romFile, 0, SEEK_END);
    romSizeInBytes = ftell(romFile);
    fseek(romFile, 0, SEEK_SET);

    // Allocate memory for ROM
    romBuffer = malloc(romSizeInBytes);

    // Read ROM into buffer
    fread(romBuffer, 1, romSizeInBytes, romFile);

    return romBuffer;
}

/**
 Prints information about an instruction (and immediately quits the program thereafter) 
 to help guide emulator development.
 Intended to be used to print out information for unimplemented instructions.
 
 @param opcode - The opcode for the instruction to print info for
 */
void printInstructionInfo(uint8_t opcode)
{
    logger("Opcode: 0x%02x\n", opcode);
    logger("%s\n", instructions[opcode]);
    logger("%d\n", instructionSizes[opcode]);
    logger("%s\n", instructionFunctions[opcode]);
    logger("%s\n\n", instructionFlags[opcode]);
    exit(0);
}

/**
 * Given an opcode and operands, perform the resulting state changes of the 8080 CPU
 */
void executeInstructionByOpcode(uint8_t opcode, uint8_t *operands, State8080 *state)
{
    uint16_t orderedOperands = ((uint16_t)operands[1] << 8) | (uint16_t)operands[0];  // Convert from little-endian
    uint16_t result = 0;  // For temporarily storing computational results losslessly
    uint8_t resultByte = 0;  // For temporarily storing 8-bit results
    char garbage[100];  // For reading from scanf, helps debugging

    // variable declaration for usage in some cases of switch
    uint8_t tempL;  // A temporary place to hold the value of the L register
    uint8_t tempH;  // A temporary place to hold the value of the H register

    switch(opcode){
        case 0x00:
            // NOP
            state->pc += 1;
            state->cyclesCompleted += 4;
            break;
        case 0x01: 
            // LXI B, D16
            // Load immediate into register pair BC
            // B = byte 3, C = byte 2
            state->b = operands[1];
            state->c = operands[0];
            state->pc += 3;
            state->cyclesCompleted += 10;
            break;
        case 0x02: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x03: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x04: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x05: 
            // DCR B
            // B = B-1
            // Flags: zero, sign, parity, auxillary carry
            state->b = addWithCheckAC(state->b, -1, state);
            checkStandardArithmeticFlags(state->b, state);
            state->pc += 1;
            state->cyclesCompleted += 5;
            break;
        case 0x06: 
            // MVI B; D8
            // Move immediate to register B
            state->b = operands[0];
            state->pc += 2;
            state->cyclesCompleted += 7;
            break;
        case 0x07: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x08: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x09: 
            // DAD B
            // Double-precision Add register pair BC to HL
            DAD_RP(state->b, state->c, state);
            break;
        case 0x0A: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x0B: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x0C: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x0D: 
            // DCR C
            // Decrement register C
            // C = C-1
            // Flags: z,s,p,ac
            state->c = addWithCheckAC(state->c, (uint8_t)(-1), state);
            checkStandardArithmeticFlags(state->c, state);
            state->pc += 1;
            state->cyclesCompleted += 5;
            break;
        case 0x0E: 
            // MVI C, D8
            // MoVe Immediate to register C
            // C = D8
            state->c = operands[0];
            state->pc += 2;
            state->cyclesCompleted += 7;
            break;
        case 0x0F: 
            // RRC
            // Rotate accumulator Right (and bypass Carry)
            // (As opposed to through Carry)
            // CY = A:0 
            // A:n = A:(n+1); A:7 = A:0
            // Flags: CY
            state->flags.carry = (state->a)&0x01;
            state->a = (state->a)>>1;
            if(state->flags.carry == 1){
                state->a = state->a | 0x80;  // A:7 = 1
            }else{
                // A:7 = 0
            }
            state->pc += 1;
            state->cyclesCompleted += 4;
            break;
        case 0x10: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x11: 
            // LXI D, D16
            // Load Immediate into Register Pair D-E
            // D = byte 3; E = byte 2
            state->d = operands[1];
            state->e = operands[0];
            state->pc += 3;
            state->cyclesCompleted += 10;
            break;
        case 0x12: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x13: 
            // INX D
            // (D)(E) = (D)(E)+1
            INX_RP(&state->d, &state->e, state);
            break;
        case 0x14: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x15: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x16: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x17: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x18: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x19: 
            // DAD D
            // Double precision Add register pair DE to register pair HL
            DAD_RP(state->d, state->e, state);
            break;
        case 0x1A: 
            // LDAX D
            // Load Accumulator indirect from register pair D-E
            // A = memory[(D)(E)]
            ;  // workaround C99 quirk where a label cannot precede a declaration
            uint16_t sourceAddress = getValueDE(state);
            state->a = readMem(sourceAddress, state);
            state->pc += 1;
            state->cyclesCompleted += 7;
            break;
        case 0x1B: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x1C: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x1D: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x1E: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x1F: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x20: 
            // RIM
            // Read Interrupt Mask
            // This instruction is actually unimplemented on the 8080
            // The instruction is functional on the 8085
            // Equivalent to NOP
            state->pc += instructionSizes[opcode];
            state->cyclesCompleted += 4;  // Did not find a reference for this, but 4 is the minimum
            break;
        case 0x21: 
            // LXI H, D16
            // Load Immediate into register pair H-L
            // H = byte 3; L = byte 2
            state->h = operands[1];
            state->l = operands[0];
            state->pc += 3;
            state->cyclesCompleted += 10;
            break;
        case 0x22: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x23: 
            // INX H
            // (H)(L) = (H)(L)+1
            INX_RP(&state->h, &state->l, state);
            break;
        case 0x24: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x25: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x26: 
            // MVI H, D8
            // MoVe Immediate into register H
            state->h = operands[0];
            state->pc += 2;
            state->cyclesCompleted += 7;
            break;
        case 0x27: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x28: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x29: 
            // DAD H
            // Double-precision Add HL to HL
            DAD_RP(state->h, state->l, state);
            break;
        case 0x2A: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x2B: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x2C: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x2D: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x2E: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x2F: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x30: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x31: 
            // LXI SP, D16
            // Load Immediate into Stack Pointer
            state->sp = orderedOperands;
            state->pc += 3;
            state->cyclesCompleted += 10;
            break;
        case 0x32: 
            // STA addr
            // STore Accumulator directly in memory addres
            // memory[address] = A
            writeMem(orderedOperands, state->a, state);
            state->pc += 3;
            state->cyclesCompleted += 13;
            break;
        case 0x33: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x34: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x35: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x36: 
            // MVI M; D8
            // memory[(H)(L)] = D8
            moveDataToHLMemory(operands[0], state);
            state->pc += 2;
            state->cyclesCompleted += 7;
            break;
        case 0x37: 
            // STC
            // Set Carry flag
            state->flags.carry = 1;
            state->pc += 1;
            state->cyclesCompleted += 4;
            break;
        case 0x38: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x39: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x3A: 
            // LDA addr
            // Load address directly into Accumulator
            // A = address
            state->a = orderedOperands;
            state->pc += 3;
            state->cyclesCompleted += 13;
            break;
        case 0x3B: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x3C: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x3D: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x3E: 
            // MVI A, D8
            // Move Immediate into register A
            // A = D8
            state->a = operands[0];
            state->pc += 2;
            state->cyclesCompleted += 7;
            break;
        case 0x3F: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x40: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x41: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x42: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x43: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x44: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x45: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x46:
            // MOV B, M
            // Move from memory into register B
            // B = memory[(H)(L)]
            moveDataFromHLMemory(&(state->b), state);
            state->pc += 1;
            state->cyclesCompleted += 7;
            break;
        case 0x47: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x48: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x49: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x4A: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x4B: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x4C: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x4D: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x4E: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x4F:
            // MOV C, A
            // Move the contents of register A into register C
            state->c = state->a;
            state->pc += 1;
            state->cyclesCompleted += 5;
            break;
        case 0x50: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x51: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x52: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x53: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x54: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x55: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x56: 
            // MOV D, M
            // Move from memory into register D
            // D = memory[(H)(L)]
            moveDataFromHLMemory(&(state->d), state);
            state->pc += 1;
            state->cyclesCompleted += 7;
            break;
        case 0x57: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x58: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x59: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x5A: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x5B: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x5C: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x5D: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x5E: 
            // MOV E, M
            // Move from memory into register E
            // E = memory[(H)(L)]
            moveDataFromHLMemory(&(state->e), state);
            state->pc += 1;
            state->cyclesCompleted += 7;
            break;
        case 0x5F: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x60: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x61: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x62: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x63: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x64: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x65: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x66: 
            // MOV H, M
            // Move from memory into register H
            // H = memory[(H)(L)]
            moveDataFromHLMemory(&(state->h), state);
            state->pc += 1;
            state->cyclesCompleted += 7;
            break;
        case 0x67: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x68: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x69: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x6A: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x6B: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x6C: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x6D: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x6E: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x6F: 
            // MOV L, A
            // Move the contents of register A into register L
            state->l = state->a;
            state->pc += 1;
            state->cyclesCompleted += 5;
            break;
        case 0x70: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x71: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x72: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x73: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x74: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x75: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x76: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x77: 
            // MOV M, A
            // memory[(H)(L)] = A
            moveDataToHLMemory(state->a, state);
            state->pc++;
            state->cyclesCompleted += 7;
            break;
        case 0x78: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x79:
            // MOV A, C
            // Move contents of register C into register A
            state->a = state->c;
            state->pc += 1;
            state-> cyclesCompleted += 5;
            break;
        case 0x7A: 
            // MOV A, D
            // Move contents of register D into register A
            // A = D
            state->a = state->d;
            state->pc += 1;
            state->cyclesCompleted += 5;
            break;
        case 0x7B: 
            // MOV A, E
            // Move the contents of register E into register A
            // A = E
            state->a = state->e;
            state->pc += 1;
            state->cyclesCompleted += 5;
            break;
        case 0x7C: 
            // MOV A, H
            // A = H
            state->a = state->h;
            state->pc += 1;
            state->cyclesCompleted += 5;
            break;
        case 0x7D: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x7E: 
            // MOV A, M
            // Move from memory into register A
            // A = memory[(H)(L)]
            moveDataFromHLMemory(&(state->a), state);
            state->pc += 1;
            state->cyclesCompleted += 7;
            break;
        case 0x7F: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x80: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x81:
            // ADD C
            // Add C to Accumulator
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x82: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x83: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x84: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x85: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x86: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x87: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x88: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x89: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x8A: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x8B: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x8C: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x8D: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x8E: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x8F: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x90: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x91: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x92: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x93: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x94: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x95: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x96: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x97: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x98: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x99: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x9A: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x9B: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x9C: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x9D: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x9E: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0x9F: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xA0: 
            // ANA B
            // AND Accumulator with Register B
            // A = A & B
            // Flags: z,s,p,cy(reset),ac
            ANA_R(state->b, state);
            break;
        case 0xA1: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xA2: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xA3: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xA4: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xA5: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xA6: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xA7:
            // ANA A
            // AND Accumulator with Accumulator
            // A = A & A
            // Flags: z,s,p,cy(reset),ac
            ANA_R(state->a, state);
            break;
        case 0xA8: 
            // XRA B
            // Exclusive OR Accumulator with Register B
            // A = A XOR B
            // Flags: z,s,p,cy(reset),ac(reset);
            XRA_R(state->b, state);
            break;
        case 0xA9: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xAA: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xAB: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xAC: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xAD: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xAE: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xAF: 
            // XRA A
            // Exclusive Or register A with register A
            // A = A XOR A
            // Flags: z,s,p,cy(reset),ac(reset);
            XRA_R(state->a, state);
            break;
        case 0xB0:
            // ORA B
            // OR Accumulator with register B
            ORA_R(state->b, state);
            break;
        case 0xB1: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xB2: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xB3: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xB4: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xB5: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xB6: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xB7: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xB8: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xB9: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xBA: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xBB: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xBC: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xBD: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xBE: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xBF: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xC0:
            // RNZ
            // Return if Not Zero
            if(!(state->flags.zero)){
                RET(state);
                state->cyclesCompleted += 1;
            }else{
                state->pc += 1;
                state->cyclesCompleted += 5;
            }
            break;
        case 0xC1: 
            // POP B
            // Pop from stack into register pair BC
            POP_RP(&state->b, &state->c, state);
            break;
        case 0xC2: 
            // JNZ addr
            // if NZ, PC = addr
            if(!state->flags.zero){
                JMP(orderedOperands, state);
			}else{
                state->pc += 3;
                state->cyclesCompleted += 10;
			}
            break;
        case 0xC3: 
            // JMP adr - JUMP
            JMP(orderedOperands, state);
            break;
        case 0xC4: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xC5: 
            // PUSH B
            // Push register pair BC onto the stack
            PUSH_RP(state->b, state->c, state);
            break;
        case 0xC6: 
            // ADI D8
            // Add Immediate to register A
            // A = A + D8
            // Flags: z,s,p,cy,ac
            addWithCheckAC(state->a, operands[0], state);
            state->a = addWithCheckCY(state->a, operands[0], state);
            checkStandardArithmeticFlags(state->a, state);
            state->pc += 2;
            state->cyclesCompleted += 7;
            break;
        case 0xC7:
            // RST 0
            RST(0, state);
            break;
        case 0xC8:
            // RZ
            // Return if Zero
            if(state->flags.zero){
                RET(state);
                state->cyclesCompleted += 1;
            }else{
                state->pc += 1;
                state->cyclesCompleted += 5;
            }
            break;
        case 0xC9: 
            // RET
            RET(state);
            break;
        case 0xCA: 
            // JZ addr
            // Jump to address if zero (flag set)
            // if Z, PC=addr
            if(state->flags.zero){
                JMP(orderedOperands, state);
            }else{
                state->pc += 3;
                state->cyclesCompleted += 10;
            }
            break;
        case 0xCB: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xCC: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xCD:
            CALL(orderedOperands, state);
            break;
        case 0xCE: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xCF:
            // RST 1
            logger("RST 1\n");
            RST(1, state);
            break;
        case 0xD0: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xD1: 
            // POP D
            POP_RP(&state->d, &state->e, state);
            break;
        case 0xD2: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xD3: 
            // OUT D8
            // Content of register A placed on 8-bit bi-directional data bus
            // for transmission to the port specified by D8
            // (data) = A
            //logger("OUT 'A==0x%02x' to data port 0x%02x\n", state->a, operands[0]);
            ;
            //char garbage[100];
            //scanf("%s", garbage);
            state->pc += 2;
            state->cyclesCompleted += 10;
            break;
        case 0xD4: 
            // CNC adr
            // Call address if No Carry
            if(state->flags.carry == 0){
                CALL(orderedOperands, state);
		    }else{
                state->pc += instructionSizes[opcode];
                state->cyclesCompleted += 11;
		    }
            break;
        case 0xD5: 
            // PUSH D
            // PUSH register pair D-E
            PUSH_RP(state->d, state->e, state);
            break;
        case 0xD6: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xD7:
            // RST 2
            logger("RST 2\n");
            RST(2, state);
            break;
        case 0xD8: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xD9: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xDA: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xDB: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xDC: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xDD: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xDE: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xDF:
            // RST 3
            RST(3, state);
            break;
        case 0xE0: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xE1: 
            // POP H
            // POP from stack into register pair HL
            POP_RP(&(state->h), &(state->l), state);
            break;
        case 0xE2: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xE3:
            // XTHL
            // Exchange stack top with H and L
            // L <-> memory[SP]
            // H <-> memory[SP+1]
            ;  // declaration after label workaround
            tempL = state->l;
            tempH = state->h;
            state->l = readMem(state->sp, state);
            state->h = readMem((state->sp)+1, state);
            writeMem(state->sp, tempL, state);
            writeMem((state->sp)+1, tempH, state);
            state->pc += 1;
            state-> cyclesCompleted += 18;
            break;
        case 0xE4: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xE5: 
            // PUSH H
            // Push register pair H-L onto the stack
            PUSH_RP(state->h, state->l, state);
            break;
        case 0xE6: 
            // ANI D8
            // AND register A with Immediate
            // A = A & D8
            //Flags: z,s,p,cy(reset),ac(reset)
            state->a = state->a & operands[0];
            checkStandardArithmeticFlags(state->a, state);
            state->flags.carry = 0;
            state->flags.auxiliaryCarry = 0;
            state->pc += 2;
            state->cyclesCompleted += 7;
            break;
        case 0xE7:
            // RST 4
            RST(4, state);
            break;
        case 0xE8: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xE9:
            // PCHL
            // Jump to address (HL) by moving (HL) to PC
            // PCH = H
            // PCL = L
            state->pc = getValueHL(state);
            state->cyclesCompleted += 5;
            break;
        case 0xEA: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xEB: 
            // XCHG
            // eXCHanGe HL with DE
            // H = D; D = H 
            // L = E; E = L
            ;  // label-declaration workaround
            tempH = state->h;
            tempL = state->l;
            
            state->h = state->d;
            state->l = state->e;
            state->d = tempH;
            state->e = tempL;
            
            state->pc += 1;
            state->cyclesCompleted += 4;
            break;
        case 0xEC: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xED: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xEE: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xEF:
            // RST 5
            RST(5, state);
            break;
        case 0xF0: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xF1: 
            // POP PSW
            // POP the Processor Status Word (and accumulator) off the stack
            // flags = memory[sp]; A = memory[sp+1]
            // sp = sp + 2
            POP_RP(&(state->a), (uint8_t*)&(state->flags), state);  // Treat flags as 8-bit uint to match function signature
            break;
        case 0xF2: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xF3: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xF4: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xF5: 
            // PUSH PSW
            // Push Processor Status Word (and accumulator) onto stack
            ;
            uint8_t flagsAsInt = *(uint8_t*)&(state->flags);  // Can't use ConditionCodes struct directly
            PUSH_RP(state->a, flagsAsInt, state);
            break;
        case 0xF6: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xF7:
            // RST 6
            RST(6, state);
            break;
        case 0xF8: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xF9: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xFA: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xFB: 
            // EI
            // Enable Interrupt
            state->interruptsEnabled = 1;
            state->pc += 1;
            state->cyclesCompleted += 4;
            break;
        case 0xFC: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xFD: 
            printInstructionInfo(opcode);
            state->pc += instructionSizes[opcode];
            break;
        case 0xFE: 
            // CPI D8
            // Compare Immediate
            // A - D8
            // Flags: z,s,p,cy,ac
            // Note: result should not be stored anywhere, this just affects flags
            /* System Manual explcitly says Carry is set if A < D8
               I am unsure if CPI is considered a subtraction-type
               instruction, i.e. does it use 2's complement arithmetic?
               It seems it does not, and I will assume for now that the way
               it handles the carry flag should not be applied to other
               subtraction-type instructions
            */
            if (state->a < operands[0]){
                state->flags.carry = 1;
            }else{
                state->flags.carry = 0;
            }
            result = addWithCheckAC(state->a, (-1)*operands[0], state);
            resultByte = (uint8_t)result;
            checkStandardArithmeticFlags(resultByte, state);
            state->pc += 2;
            state->cyclesCompleted += 7;
            break;
        case 0xFF:
            // RST 7
            RST(7, state);
            break;
	}
}

/**
* Initialize global variables.
* 
* I do this here because the initializations are about 1000 lines.
* I could do this elsewhere, but it would clutter the top of the file.
* Delegate initialization to a function so it can happily remain at the bottom of the file.
* 
* I suppose doing this in another file was also an option, but this is most convenient.
*
* @return void
*/
void initializeGlobals()
{
    char instructionsLocal[256][20] = {
        "NOP",
        "LXI B;D16",
        "STAX B",
        "INX B",
        "INR B",
        "DCR B",
        "MVI B; D8",
        "RLC",
        "-",
        "DAD B",
        "LDAX B",
        "DCX B",
        "INR C",
        "DCR C",
        "MVI C;D8",
        "RRC",
        "-",
        "LXI D;D16",
        "STAX D",
        "INX D",
        "INR D",
        "DCR D",
        "MVI D; D8",
        "RAL",
        "-",
        "DAD D",
        "LDAX D",
        "DCX D",
        "INR E",
        "DCR E",
        "MVI E;D8",
        "RAR",
        "RIM",
        "LXI H;D16",
        "SHLD adr",
        "INX H",
        "INR H",
        "DCR H",
        "MVI H;D8",
        "DAA",
        "-",
        "DAD H",
        "LHLD adr",
        "DCX H",
        "INR L",
        "DCR L",
        "MVI L; D8",
        "CMA",
        "SIM",
        "LXI SP; D16",
        "STA adr",
        "INX SP",
        "INR M",
        "DCR M",
        "MVI M;D8",
        "STC",
        "-",
        "DAD SP",
        "LDA adr",
        "DCX SP",
        "INR A",
        "DCR A",
        "MVI A;D8",
        "CMC",
        "MOV B;B",
        "MOV B;C",
        "MOV B;D",
        "MOV B;E",
        "MOV B;H",
        "MOV B;L",
        "MOV B;M",
        "MOV B;A",
        "MOV C;B",
        "MOV C;C",
        "MOV C;D",
        "MOV C;E",
        "MOV C;H",
        "MOV C;L",
        "MOV C;M",
        "MOV C;A",
        "MOV D;B",
        "MOV D;C",
        "MOV D;D",
        "MOV D;E",
        "MOV D;H",
        "MOV D;L",
        "MOV D;M",
        "MOV D;A",
        "MOV E;B",
        "MOV E;C",
        "MOV E;D",
        "MOV E;E",
        "MOV E;H",
        "MOV E;L",
        "MOV E;M",
        "MOV E;A",
        "MOV H;B",
        "MOV H;C",
        "MOV H;D",
        "MOV H;E",
        "MOV H;H",
        "MOV H;L",
        "MOV H;M",
        "MOV H;A",
        "MOV L;B",
        "MOV L;C",
        "MOV L;D",
        "MOV L;E",
        "MOV L;H",
        "MOV L;L",
        "MOV L;M",
        "MOV L;A",
        "MOV M;B",
        "MOV M;C",
        "MOV M;D",
        "MOV M;E",
        "MOV M;H",
        "MOV M;L",
        "HLT",
        "MOV M;A",
        "MOV A;B",
        "MOV A;C",
        "MOV A;D",
        "MOV A;E",
        "MOV A;H",
        "MOV A;L",
        "MOV A;M",
        "MOV A;A",
        "ADD B",
        "ADD C",
        "ADD D",
        "ADD E",
        "ADD H",
        "ADD L",
        "ADD M",
        "ADD A",
        "ADC B",
        "ADC C",
        "ADC D",
        "ADC E",
        "ADC H",
        "ADC L",
        "ADC M",
        "ADC A",
        "SUB B",
        "SUB C",
        "SUB D",
        "SUB E",
        "SUB H",
        "SUB L",
        "SUB M",
        "SUB A",
        "SBB B",
        "SBB C",
        "SBB D",
        "SBB E",
        "SBB H",
        "SBB L",
        "SBB M",
        "SBB A",
        "ANA B",
        "ANA C",
        "ANA D",
        "ANA E",
        "ANA H",
        "ANA L",
        "ANA M",
        "ANA A",
        "XRA B",
        "XRA C",
        "XRA D",
        "XRA E",
        "XRA H",
        "XRA L",
        "XRA M",
        "XRA A",
        "ORA B",
        "ORA C",
        "ORA D",
        "ORA E",
        "ORA H",
        "ORA L",
        "ORA M",
        "ORA A",
        "CMP B",
        "CMP C",
        "CMP D",
        "CMP E",
        "CMP H",
        "CMP L",
        "CMP M",
        "CMP A",
        "RNZ",
        "POP B",
        "JNZ adr",
        "JMP adr",
        "CNZ adr",
        "PUSH B",
        "ADI D8",
        "RST 0",
        "RZ",
        "RET",
        "JZ adr",
        "-",
        "CZ adr",
        "CALL adr",
        "ACI D8",
        "RST 1",
        "RNC",
        "POP D",
        "JNC adr",
        "OUT D8",
        "CNC adr",
        "PUSH D",
        "SUI D8",
        "RST 2",
        "RC",
        "-",
        "JC adr",
        "IN D8",
        "CC adr",
        "-",
        "SBI D8",
        "RST 3",
        "RPO",
        "POP H",
        "JPO adr",
        "XTHL",
        "CPO adr",
        "PUSH H",
        "ANI D8",
        "RST 4",
        "RPE",
        "PCHL",
        "JPE adr",
        "XCHG",
        "CPE adr",
        "-",
        "XRI D8",
        "RST 5",
        "RP",
        "POP PSW",
        "JP adr",
        "DI",
        "CP adr",
        "PUSH PSW",
        "ORI D8",
        "RST 6",
        "RM",
        "SPHL",
        "JM adr",
        "EI",
        "CM adr",
        "-",
        "CPI D8",
        "RST 7"
    };

    char instructionSizesLocal[256] = {
        1,
        3,
        1,
        1,
        1,
        1,
        2,
        1,
        0,
        1,
        1,
        1,
        1,
        1,
        2,
        1,
        0,
        3,
        1,
        1,
        1,
        1,
        2,
        1,
        0,
        1,
        1,
        1,
        1,
        1,
        2,
        1,
        1,
        3,
        3,
        1,
        1,
        1,
        2,
        1,
        0,
        1,
        3,
        1,
        1,
        1,
        2,
        1,
        1,
        3,
        3,
        1,
        1,
        1,
        2,
        1,
        0,
        1,
        3,
        1,
        1,
        1,
        2,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        3,
        3,
        3,
        1,
        2,
        1,
        1,
        1,
        3,
        0,
        3,
        3,
        2,
        1,
        1,
        1,
        3,
        2,
        3,
        1,
        2,
        1,
        1,
        0,
        3,
        2,
        3,
        0,
        2,
        1,
        1,
        1,
        3,
        1,
        3,
        1,
        2,
        1,
        1,
        1,
        3,
        1,
        3,
        0,
        2,
        1,
        1,
        1,
        3,
        1,
        3,
        1,
        2,
        1,
        1,
        1,
        3,
        1,
        3,
        0,
        2,
        1
    };

    char instructionFlagsLocal[256][20] = {
        "",
        "",
        "",
        "",
        "Z; S; P; AC",
        "Z; S; P; AC",
        "",
        "CY",
        "",
        "CY",
        "",
        "",
        "Z; S; P; AC",
        "Z; S; P; AC",
        "",
        "CY",
        "",
        "",
        "",
        "",
        "Z; S; P; AC",
        "Z; S; P; AC",
        "",
        "CY",
        "",
        "CY",
        "",
        "",
        "Z; S; P; AC",
        "Z; S; P; AC",
        "",
        "CY",
        "",
        "",
        "",
        "",
        "Z; S; P; AC",
        "Z; S; P; AC",
        "",
        "",
        "",
        "CY",
        "",
        "",
        "Z; S; P; AC",
        "Z; S; P; AC",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; AC",
        "Z; S; P; AC",
        "",
        "CY",
        "",
        "CY",
        "",
        "",
        "Z; S; P; AC",
        "Z; S; P; AC",
        "",
        "CY",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "Z; S; P; CY; AC",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; CY; AC",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; CY; AC",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; CY; AC",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; CY; AC",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; CY; AC",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; CY; AC",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; CY; AC",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "Z; S; P; CY; AC",
        ""
    };

    char instructionFunctionsLocal[256][100] = {
        "",
        "B <- byte 3; C <- byte 2",
        "(BC) <- A",
        "BC <- BC+1",
        "B <- B+1",
        "B <- B-1",
        "B <- byte 2",
        "A = A << 1; bit 0 = prev bit 7; CY = prev bit 7",
        "",
        "HL = HL + BC",
        "A <- (BC)",
        "BC = BC-1",
        "C <- C+1",
        "C <-C-1",
        "C <- byte 2",
        "A = A >> 1; bit 7 = prev bit 0; CY = prev bit 0",
        "",
        "D <- byte 3; E <- byte 2",
        "(DE) <- A",
        "DE <- DE + 1",
        "D <- D+1",
        "D <- D-1",
        "D <- byte 2",
        "A = A << 1; bit 0 = prev CY; CY = prev bit 7",
        "",
        "HL = HL + DE",
        "A <- (DE)",
        "DE = DE-1",
        "E <-E+1",
        "E <- E-1",
        "E <- byte 2",
        "A = A >> 1; bit 7 = prev bit 7; CY = prev bit 0",
        "special",
        "H <- byte 3; L <- byte 2",
        "(adr) <-L; (adr+1)<-H",
        "HL <- HL + 1",
        "H <- H+1",
        "H <- H-1",
        "H <- byte 2",
        "special",
        "",
        "HL = HL + HI",
        "L <- (adr); H<-(adr+1)",
        "HL = HL-1",
        "L <- L+1",
        "L <- L-1",
        "L <- byte 2",
        "A <- !A",
        "special",
        "SP.hi <- byte 3; SP.lo <- byte 2",
        "(adr) <- A",
        "SP = SP + 1",
        "(HL) <- (HL)+1",
        "(HL) <- (HL)-1",
        "(HL) <- byte 2",
        "CY = 1",
        "",
        "HL = HL + SP",
        "A <- (adr)",
        "SP = SP-1",
        "A <- A+1",
        "A <- A-1",
        "A <- byte 2",
        "CY=!CY",
        "B <- B",
        "B <- C",
        "B <- D",
        "B <- E",
        "B <- H",
        "B <- L",
        "B <- (HL)",
        "B <- A",
        "C <- B",
        "C <- C",
        "C <- D",
        "C <- E",
        "C <- H",
        "C <- L",
        "C <- (HL)",
        "C <- A",
        "D <- B",
        "D <- C",
        "D <- D",
        "D <- E",
        "D <- H",
        "D <- L",
        "D <- (HL)",
        "D <- A",
        "E <- B",
        "E <- C",
        "E <- D",
        "E <- E",
        "E <- H",
        "E <- L",
        "E <- (HL)",
        "E <- A",
        "H <- B",
        "H <- C",
        "H <- D",
        "H <- E",
        "H <- H",
        "H <- L",
        "H <- (HL)",
        "H <- A",
        "L <- B",
        "L <- C",
        "L <- D",
        "L <- E",
        "L <- H",
        "L <- L",
        "L <- (HL)",
        "L <- A",
        "(HL) <- B",
        "(HL) <- C",
        "(HL) <- D",
        "(HL) <- E",
        "(HL) <- H",
        "(HL) <- L",
        "special",
        "(HL) <- A",
        "A <- B",
        "A <- C",
        "A <- D",
        "A <- E",
        "A <- H",
        "A <- L",
        "A <- (HL)",
        "A <- A",
        "A <- A + B",
        "A <- A + C",
        "A <- A + D",
        "A <- A + E",
        "A <- A + H",
        "A <- A + L",
        "A <- A + (HL)",
        "A <- A + A",
        "A <- A + B + CY",
        "A <- A + C + CY",
        "A <- A + D + CY",
        "A <- A + E + CY",
        "A <- A + H + CY",
        "A <- A + L + CY",
        "A <- A + (HL) + CY",
        "A <- A + A + CY",
        "A <- A - B",
        "A <- A - C",
        "A <- A + D",
        "A <- A - E",
        "A <- A + H",
        "A <- A - L",
        "A <- A + (HL)",
        "A <- A - A",
        "A <- A - B - CY",
        "A <- A - C - CY",
        "A <- A - D - CY",
        "A <- A - E - CY",
        "A <- A - H - CY",
        "A <- A - L - CY",
        "A <- A - (HL) - CY",
        "A <- A - A - CY",
        "A <- A & B",
        "A <- A & C",
        "A <- A & D",
        "A <- A & E",
        "A <- A & H",
        "A <- A & L",
        "A <- A & (HL)",
        "A <- A & A",
        "A <- A ^ B",
        "A <- A ^ C",
        "A <- A ^ D",
        "A <- A ^ E",
        "A <- A ^ H",
        "A <- A ^ L",
        "A <- A ^ (HL)",
        "A <- A ^ A",
        "A <- A | B",
        "A <- A | C",
        "A <- A | D",
        "A <- A | E",
        "A <- A | H",
        "A <- A | L",
        "A <- A | (HL)",
        "A <- A | A",
        "A - B",
        "A - C",
        "A - D",
        "A - E",
        "A - H",
        "A - L",
        "A - (HL)",
        "A - A",
        "if NZ; RET",
        "C <- (sp); B <- (sp+1); sp <- sp+2",
        "if NZ; PC <- adr",
        "PC <= adr",
        "if NZ; CALL adr",
        "(sp-2)<-C; (sp-1)<-B; sp <- sp - 2",
        "A <- A + byte",
        "CALL $0",
        "if Z; RET",
        "PC.lo <- (sp); PC.hi<-(sp+1); SP <- SP+2",
        "if Z; PC <- adr",
        "",
        "if Z; CALL adr",
        "(SP-1)<-PC.hi;(SP-2)<-PC.lo;SP<-SP+2;PC=adr",
        "A <- A + data + CY",
        "CALL $8",
        "if NCY; RET",
        "E <- (sp); D <- (sp+1); sp <- sp+2",
        "if NCY; PC<-adr",
        "special",
        "if NCY; CALL adr",
        "(sp-2)<-E; (sp-1)<-D; sp <- sp - 2",
        "A <- A - data",
        "CALL $10",
        "if CY; RET",
        "",
        "if CY; PC<-adr",
        "special",
        "if CY; CALL adr",
        "",
        "A <- A - data - CY",
        "CALL $18",
        "if PO; RET",
        "L <- (sp); H <- (sp+1); sp <- sp+2",
        "if PO; PC <- adr",
        "L <-> (SP); H <-> (SP+1) ",
        "if PO; CALL adr",
        "(sp-2)<-L; (sp-1)<-H; sp <- sp - 2",
        "A <- A & data",
        "CALL $20",
        "if PE; RET",
        "PC.hi <- H; PC.lo <- L",
        "if PE; PC <- adr",
        "H <-> D; L <-> E",
        "if PE; CALL adr",
        "",
        "A <- A ^ data",
        "CALL $28",
        "if P; RET",
        "flags <- (sp); A <- (sp+1); sp <- sp+2",
        "if P=1 PC <- adr",
        "special",
        "if P; PC <- adr",
        "(sp-2)<-flags; (sp-1)<-A; sp <- sp - 2",
        "A <- A | data",
        "CALL $30",
        "if M; RET",
        "SP=HL",
        "if M; PC <- adr",
        "special",
        "if M; CALL adr",
        "",
        "A - data",
        "CALL $38"
    };

    memcpy(instructions, instructionsLocal, 256*20);
    memcpy(instructionSizes, instructionSizesLocal, 256);
    memcpy(instructionFlags, instructionFlagsLocal, 256*20);
    memcpy(instructionFunctions, instructionFunctionsLocal, 256*100);
}