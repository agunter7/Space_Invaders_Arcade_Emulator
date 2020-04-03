/***********************************************************************************
* Emulates an Intel 8080 CPU's internal state and instruction execution
* @Author: Andrew Gunter
***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Global variable definitions and function prototypes
char instructions[256][20];
char instructionSizes[256];
char instructionFlags[256][20];
char instructionFunctions[256][100];
void initializeGlobals();

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

/**
* Main function
*/
int main(int argc, char **argv)
{
    uint8_t *romBuffer;  // Buffer for storing bytes read from Space Invaders ROM
    initializeGlobals();

    // Open Space Invaders ROM file and store contents in a buffer
    FILE *invadersFile = fopen(argv[1], "rb");  // binary file read-only
    if(invadersFile == NULL){
        printf("Failed to open Space Invaders ROM.");
        return -1;
    }
    romBuffer = getRomBuffer(invadersFile);
    
    runCodeFromBuffer(romBuffer);

    free(romBuffer);
    return 0;
}

void runCodeFromBuffer(uint8_t *romBuffer)
{
    State8080 state = {
        memory = malloc(2^16);  // Intel 8080 uses 16-bit byte-addressable memory
        .flags = 0;
        .a = 0;    
        .b = 0;    
        .c = 0;    
        .d = 0;    
        .e = 0;    
        .h = 0;    
        .l = 0;    
        .sp;
        .pc = 0;
    };  // Intel 8080 CPU state

    // Place ROM buffer data into CPU memory
    memcpy(state.memory, romBuffer, 0x2000);

    // Get next instruction and execute
    uint8_t operation = 0;
    uint8_t operands[2] = {0, 0};
    unsigned integer instructionSize = 0;
    while(state.pc < 0x2000){  // Keep within bounds of ROM data for 8080 memory map
        // Reset operands, 0xff chosen as it will likely standout as a reset value more than 0x00 would
        operands[0] = 0xff;
        operands[1] = 0xff;

        // Get next operation
        operation = romBuffer[pc];

        // Get operands depending on instruction size
        instructionSize = instructionSizes[operation];  // Array is ordered based on opcode
        numOperands = instructionSize-1;
        int lastOperandAddress = pc+instructionSize-1;
        int operandNum;
        for(int operandAddress = pc+1; operandAddress <= lastOperandAddress; operandAddress++){
              operandNum = operandAddress-(pc+1);
              operands[operandNum] = romBuffer[operandAddress];
		}

        executeInstruction(operation, operands);        
	}
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
 * Given an opcode and operands, perform the resulting state changes of the 8080 CPU
 */
void executeInstruction(uint8_t opcode, uint8_t *operands)
{

}

/**
* Initialize global variables.
* 
* I do this here because the initializations are about 1000 lines.
* I could do this elsewhere, but it would clutter the top of the file.
* Delegate initialization to a function so it can happily remain at the bottom of the file.
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
        "A = A &gt;&gt; 1; bit 7 = prev bit 0; CY = prev bit 0",
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
        "A = A &gt;&gt; 1; bit 7 = prev bit 7; CY = prev bit 0",
        "special",
        "H <- byte 3; L <- byte 2",
        "(adr) <-L; (adr+1)<-H",
        "HL <- HL + 1",
        "H <- H+1",
        "H <- H-1",
        "L <- byte 2",
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
        "(HL) <- C",
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
        "A <- A &amp; B",
        "A <- A &amp; C",
        "A <- A &amp; D",
        "A <- A &amp; E",
        "A <- A &amp; H",
        "A <- A &amp; L",
        "A <- A &amp; (HL)",
        "A <- A &amp; A",
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
        "L <-&gt; (SP); H <-&gt; (SP+1) ",
        "if PO; CALL adr",
        "(sp-2)<-L; (sp-1)<-H; sp <- sp - 2",
        "A <- A &amp; data",
        "CALL $20",
        "if PE; RET",
        "PC.hi <- H; PC.lo <- L",
        "if PE; PC <- adr",
        "H <-&gt; D; L <-&gt; E",
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