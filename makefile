CC=gcc
CFLAGS=-I.
#DEPS =   # Dependencies go here when needed

disassemblerMake: disassembler.o
	$(CC) -o bin/disassembler disassembler.o 