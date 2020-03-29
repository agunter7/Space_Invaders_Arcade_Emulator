CC=gcc
CFLAGS=-I.
#DEPS =   # Dependencies go here when needed

disassemblerMake: src/disassembler.c
	$(CC) -o bin/disassembler src/disassembler.c

# -g enables debug, -og enables only the optimizations that do not interfere with debugging
debug: src/disassembler.c
	$(CC) -g -og src/disassembler.c

clean:
	rm output.txt
	rm assembly.txt
	rm g.exe
