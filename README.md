# intel_8080_emulator
A Windows emulator for the Intel 8080 processor written in C

# Build
The project uses a makefile to compile the C source files. 
In a windows environment, this makefile will not be useable by default. 
In order to use the makefile, you will need: 
A) Git for Windows [https://gitforwindows.org/]
B) mingw-w64 (minimum GNU for Windows 64-bit) [http://mingw-w64.org/doku.php/download/mingw-builds]
C) The GNU Make tool for 64-bit mingw (minimum GNU for Windows) [https://gist.github.com/evanwill/0207876c3243bbb6863e65ec5dc3f058] [https://sourceforge.net/projects/ezwinports/files/]

1) Install Git for Windows. This should be straightforward.
2) Install mingw-w64 to the default Program Files path. Choose the latest version. "Architecture == x86_64" (64-bit). "threads == win32".
3) Add the 'bin' folder from your mingw-w64 install to your systems PATH environment variable. (There may be two bin folders in your install, try both)
4) Download the appropriate version of GNU make (At time of writing this would be "make-4.1-2-without-guile-w32-bin.zip")
5) Extract the contents of the zip folder.
6) Copy all contents from the extraction to your "Git\mingw64\" folder inside your Git for Windows folder path (NOT the mingw-w64 install performed at step 2)
7) In step 6, do not overwrite/replace any existing files

# Intel 8080 Memory Map
ROM    
    $0000-$07ff:    invaders.h    
    $0800-$0fff:    invaders.g    
    $1000-$17ff:    invaders.f    
    $1800-$1fff:    invaders.e    

    RAM    
    $2000-$23ff:    work RAM    
    $2400-$3fff:    video RAM    

    $4000-:     RAM mirror