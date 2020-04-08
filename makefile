CC=gcc
CFLAGS=-I.
#DEPS =   # Dependencies go here when needed

all: src/disassembler.c src/emulatorShell.c src/instructions.c src/helpers.c
	$(CC) -o bin/disassembler src/disassembler.c
	$(CC) -o bin/emulatorShell src/emulatorShell.c src/instructions.c src/helpers.c

# -g enables debug, -og enables only the optimizations that do not interfere with debugging
debug: src/disassembler.c src/emulatorShell.c src/instructions.c src/helpers.c
	$(CC) -g -og src/disassembler.c
	$(CC) -g -og src/emulatorShell.c src/instructions.c src/helpers.c

clean:
	rm output.txt
	rm assembly.txt
	rm g.exe
