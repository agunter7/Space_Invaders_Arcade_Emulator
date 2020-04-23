CC=gcc
# Miscellaneous compiler flags
GENERAL_FLAGS=-Wall
# specifies directories for header files
INCLUDE_PATHS=-I"C:\Program Files\mingw_dev_lib\SDL2-2.0.12\x86_64-w64-mingw32\include\SDL2"
# specifies directories for lib files
LIBRARY_PATHS=-L"C:\Program Files\mingw_dev_lib\SDL2-2.0.12\x86_64-w64-mingw32\lib"
# specifies the libraries being linked against
LINKER_FLAGS=-lmingw32 -lSDL2main -lSDL2
# Source code file names
SOURCES_EMULATOR=src/shell8080.c src/instructions.c src/helpers.c src/arcadeMachine.c src/arcadeEnvironment.c
#SOURCES_EMULATOR=src/arcadeMachine.c src/arcadeEnvironment.c
SOURCES_DISASSEMBLER=src/disassembler.c
# Final executable name
EXE_NAME_EMU=bin/emulatorShell
EXE_NAME_DISASSEMBLER=bin/disassembler

all: $(SOURCES_DISASSEMBLER) $(SOURCES_EMULATOR)
	$(CC) $(INCLUDE_PATHS) $(SOURCES_DISASSEMBLER) $(LIBRARY_PATHS) $(GENERAL_FLAGS) $(LINKER_FLAGS) -o $(EXE_NAME_DISASSEMBLER)
	$(CC) $(INCLUDE_PATHS) $(SOURCES_EMULATOR) $(LIBRARY_PATHS) $(GENERAL_FLAGS) $(LINKER_FLAGS) -o $(EXE_NAME_EMU)

# -g enables debug, -og enables only the optimizations that do not interfere with debugging
debug: src/disassembler.c src/emulatorShell.c src/instructions.c src/helpers.c
	$(CC) -g -og src/disassembler.c
	$(CC) -g -og src/emulatorShell.c src/instructions.c src/helpers.c

clean:
	rm output.txt
	rm assembly.txt
	rm g.exe
