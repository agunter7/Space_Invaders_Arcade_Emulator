CC=gcc
CFLAGS=-I.
#DEPS =   # Dependencies go here when needed

disassemblerMake: src/disassembler.c
	$(CC) -o bin/disassembler src/disassembler.c