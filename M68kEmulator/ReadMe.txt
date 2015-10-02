========================================================================
    CONSOLE APPLICATION : M68kEmulator by Cameron Armstrong 2015
========================================================================

This is an emulator of the Motorola 68000 series of microprocessors.

How to compile in the command line in Mac/Linux:
Navigate to the project directory and run: g++ M68kEmulator.cpp Memory.cpp CPUCore.cpp ProgramLoader.cpp -std=c++11 -o M68kEmulator

The program will open and execute a file in its directory called program.S68
This is a Motorola S-Record file. The sample one provided was assembled with the EASy68K assembler. You may use this file or create your 
own program by writing assembly code in an assembler and saving the S68 file as program.S68

The program will save a complete memory dump when finished called core_dump.txt

Current recognised instructions:

ADD
ADDA
ADDI
ADDQ
Bcc
BRA
BSR
CLR
CMP
JMP
LEA
MOVE
MOVEQ
NOP
RTS
TRAP