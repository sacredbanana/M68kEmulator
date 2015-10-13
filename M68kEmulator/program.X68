*       Addind.x68
*       Write the byte value $aa to locations $2000 to $3000

*       Definitions used  in this code.
FIRST   EQU     $2000           First Location.
BYTES   EQU     $1001           Number of locations to be changed.
VALUE   EQU     $AA             Value to be written to memory.

        ORG     $1000           Start of code location.

*       Main program starts here.
        MOVEA.L #FIRST,A0       Pointer to first location in memory.
        MOVE.W  #BYTES,D0       Set up the counter.
NEXT    MOVE.B  #VALUE,(A0)     Write the value to current memory location.
        ADDA.L  #1,A0           Increment memory pointer.
        SUB.W   #1,D0           Decrement counter.
        BNE     NEXT            Loop back if not finished.
        STOP    #$2700          ...else stop.

        END     $1000           End of program.

*~Font name~Courier New~
*~Font size~10~
*~Tab type~1~
*~Tab size~4~
