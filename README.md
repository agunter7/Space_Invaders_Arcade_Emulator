# intel_8080_emulator
A Windows emulator for the Intel 8080 processor written in C

# Build
The project uses a makefile to compile the C source files. 
In a windows environment, this makefile will not be useable by default. 
In order to use the makefile, you will need: 

A) Git for Windows [https://gitforwindows.org/]

B) mingw-w64 (minimum GNU for Windows 64-bit) [http://mingw-w64.org/doku.php/download/mingw-builds]

C) The GNU Make tool for 64-bit mingw (minimum GNU for Windows) [https://gist.github.com/evanwill/0207876c3243bbb6863e65ec5dc3f058] [https://sourceforge.net/projects/ezwinports/files/]

D) SDL2 (Simple DirectMedia Layer 2) 64-bit Developer Library source files (for mingw) [https://libsdl.org/download-2.0.php]


1) Install Git for Windows. This should be straightforward.
2) Install mingw-w64 to the default Program Files path. Choose the latest version. "Architecture == x86_64" (64-bit). "threads == win32".
3) Add the 'bin' folder from your mingw-w64 install to your systems PATH environment variable. (There may be two bin folders in your install, try both)
4) Download the appropriate version of GNU make (At time of writing this would be "make-4.1-2-without-guile-w32-bin.zip")
5) Extract the contents of the zip folder.
6) Copy all contents from the extraction to your "Git\mingw64\" folder inside your Git for Windows folder path (NOT the mingw-w64 install performed at step 2)
7) In step 6, do not overwrite/replace any existing files
8) Download the SDL2 Dev Library folder to the path "C:\Program Files\mingw_dev_lib\SDL2-2.0.12" (or any folder of your choosing if you edit the makefile)

# Usage
8080 Emulator - Use run.sh

8080 Disassembler - Use disassemble.sh

# Resources
1) http://computerarcheology.com/Arcade/SpaceInvaders/
2) http://www.emulator101.com/welcome.html
3) http://www.nj7p.info/Manuals/PDFs/Intel/9800153B.pdf
4) https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf

# Assumptions
1) Clock speed is 2 MHz (or 0.5μs clock period). The 8080 is capable of up to 3.125 MHz, but this was seemingly not 
the case for the Space Invaders Arcade Machine

2) One clock period marks a CPU "state". A single instruction constitutes multiple states throughout its execution.
This is based off of information from the 8080 programmer's manual.

3) The I/O ports for Space Invaders are separate rather than joined. Documentation from Computer Archeology and Emulator
101 is quite unclear on this. It could be, for example, that Read Port 2 and Write Port 2 are the same physical port.
This emulator assumes Read Port 2 and Write Port 2 to be two separate physical entities with their own I/O addresses in
the 8080's I/O memory.
