/* this is simple primitive operations for FLEX 9 for cmoc.
 * This is a simple set of operations to allow the user to
 * interface with the system io and console routines for flex.
 * 
 * this is free software and is released under the GPLv3 license.
 * see the file LICENSE in the main repository for more details.
 *
 * Copyright (C) 2025 David Collins
 */


#include "tinytypes.h"


// Console base addresses 0xD3E0 each entry is 2 bytes and requires
// indirect addressing to access the entry point eg JSR [INCHNE]
// this is a list of 16 bit address containng the entry point 
// of the subroutine.

#define INCHNE       0xD3E5   // input character no echo routine
#define MONITOR      0xD3F3   // monitor entry point
#define TINIT        0xD3F5   // initialize terminal
#define STATUS       0xD3F7   // terminal status routine
#define OUTCH        0xD3F9   // output character routine
#define INCH         0xD3FB   // input character with echo routine

#define FLEXRESET    0xCD03   // FLEX system reset entry

// IO base addresses 0xDE00 each entry is 3 bytes and requires
// direct addressing to access the entry point eg JSR READ,
// this is a jump table so the branch happens 2x and returns at the
// end of the routine. the define is the address of the entry point
// jump.

#define READ         0xDE00   // read sector routine
#define WRITE        0xDE03   // write sector routine
#define VERIFY       0xDE06   // verify last sector written
#define RESTORE      0xDE09   // restore (reset) drive
#define DRIVE        0xDE0C   // set current drive
#define CHKRDY       0xDE0F   // check drive ready status
#define QUICK        0xDE12   // quick check drive ready status
#define INIT         0xDE15   // initialize drive
#define WARM         0xDE18   // warm start drive
#define SEEK         0xDE1B   // seek to specified track and sector




// console io
// (console io uses a table of addresses, we use inderect addressing)

// wait and return a character from the console without echo
asm char getCharNoEcho(void) {
    asm {
            JSR [INCHNE]  // ; call the input character no echo routine 
            TFR A,B       // ; store the character in b where cmoc expects the return value
    } 
}

// wait and return a character from the console with echo
asm char getChar(void) {
    asm {
            JSR [INCH]  // ; call the input character routine
            TFR A,B     // ; store the character in b where cmoc expects the return value
    }
}

// return 'true' if a character is available
asm bool getTerminalState(void) {
    asm { 
            CLRB          ; asume there is no character waiting 'false'
            JSR [STATUS]  ; call the status routine
            BEQ ENDSTATE  ; if no character is waiting, branch to endstate  
            INCB          ; character wating set to 'true' 
        ENDSTATE:
    }

}

// output a character to the console
// note that we are pulling from 3,S this is because cmoc converts char into uint16_t 
// when it pushes to the stack, so we need to pull from the lsb of the 16 bit value 
// stored at 2,S.  OR we could have pulled 2,s into D and then Transfered B to A, but this is simpler.
asm void outChar(char c) {
    asm {
            LDA 3,S       ; load the character to output into A.
            JSR [OUTCH]   ; call the output character routine
    }
}

// control flow

// exit to the monitor
asm void exitMON(void) {
    asm {
            JMP [MONITOR]  ; jump to the monitor address
    }
}

// exit to the flex system (this is not part of the console package)
asm void exitFLEX(void) {
    asm{
           JMP FLEXRESET  ; jump to the flex reset address
    }

}

// disk io
// (disk io uses a jump table, we use direct addressing)


// read a sector from the disk
asm uint8_t readSector(uint8_t track, uint8_t sector, void* buffer) {
    asm {
           LDA 3,S    ; load the track number 
           LDB 5,S    ; load the sector number
           LDX 6,S    ; load the buffer address
           JSR READ   ; call the read routine 
           
           ;; error code is already in B, which is conviently the return value location
           
    }
   
    
}

// write a sector to the disk
asm uint8_t writeSector(uint8_t track, uint8_t sector, void* buffer) {
    asm {
           LDA 3,S    ; load the track number 
           LDB 5,S    ; load the sector number
           LDX 6,S    ; load the buffer address
           JSR WRITE      ; call the write routine 
           
           ;; error code is already in B, which is conviently the return value location
    }
   
}

// verify, returns zero if no error, non zero if error exists

asm uint8_t verifySector(void) {
    asm {
            JSR VERIFY    ; call the verify routine
            
            ;; error code is already in B, which is conviently the return value location
    
    }
}

// RTZ the drive: variable 'drive' is the FCB for the drive to reset -- this is nebulus 
// RESTORE looks at drive + 3 to find the drive number, so you can point to an actual FCB 
// or just a dummy variable that is 4 bytes long and the drive number is in the 4th byte
// This function is the same on all the below that use the drive number.

asm uint8_t rtzDrive(void* drive) {

    asm {
            LDX 2,S        ; load the drive address
            JSR RESTORE    ; call the restore routine 
            
            ;; error code is already in B, which is conviently the return value location
    }
   
}

// set the current drive
asm uint8_t setDrive(void* drive) {
    asm {
            LDX 2,S        ; load the drive address
            JSR DRIVE      ; call the set drive routine 
          
            ;; error code is already in B, which is conviently the return value location
    } 

}

// check the current drive ready status
asm uint8_t chkDrive(void* drive) {
    asm {
            LDX 2,S        ; load the drive address
            JSR CHKRDY     ; call the check drive routine 
         
            ;; error code is already in B, which is conviently the return value location

    }   
}

// check the current drive ready status without waiting
asm uint8_t quickCheckDrive(void* drive) {
    asm {
            LDX 2,S        ; load the drive address
            JSR QUICK      ; call the quick check drive routine 
           
            ;; error code is already in B, which is conviently the return value location
    }
}

// initialize the drive hardware from cold
asm void initDrive(void) {
    asm {
            JSR INIT       ; call the initialize drive routine 
    }       
}

// warm start the drive hardware
asm void warmDrive(void) {
    asm {
            JSR WARM       ; call the warm start drive routine 
    }
    
}

// seek to the specified track and sector
asm uint8_t seekDrive(uint8_t track, uint8_t sector) {
    asm {
            LDA 3,S    ; load the track number 
            LDB 5,S    ; load the sector number
            JSR SEEK       ; call the seek routine 

            ;; error code is already in B, which is conviently the return value location
    }
    
}          
