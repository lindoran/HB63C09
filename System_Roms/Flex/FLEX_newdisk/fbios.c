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
char getCharNoEcho() {
    char c; // hold the value of the character to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.
    
    asm {
            JSR [INCHNE]  ; call the input character no echo routine 
            STA :c        ; store the character in c
    }

    return c;  
}

// wait and return a character from the console with echo
char getChar() {
    char c; // hold the value of the character to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.
    
    asm {
            JSR [INCH]  ; call the input character routine
            STA :c      ; store the character in c
    }

    return c;  
}

// return 'true' if a character is available
bool getTerminalState() {
    bool state = true; // hold the value of the state to return, we clean if a character is not waiting
 
    // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
    // have not quite worked how to do that.

    asm {
            JSR [STATUS]  ; call the status routine
            BNE ENDSTATE  ; if a character is waiting, branch to endstate  
            CLR :state    ; no character wating clear to 'false' 
        ENDSTATE:
    }
    return state;
}

// output a character to the console
void putChar(char c) {
    asm {
            LDA :c        ; load the character to output
            JSR [OUTCH]   ; call the output character routine
    }


}

// control flow

// exit to the monitor
asm exitMON() {
    asm {
            JMP [MONITOR]  ; jump to the monitor address
    }
}

// exit to the flex system (this is not part of the console package)
asm exitFLEX() {
    asm{
           JMP FLEXRESET  ; jump to the flex reset address
    }

}

// disk io
// (disk io uses a jump table, we use direct addressing)


// read a sector from the disk
uint8_t readSector(uint8_t track, uint8_t sector, uint16_t* buffer) {
    uint8_t error = 0; // hold the value of the error to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.

    asm {
           LDA :track     ; load the track number 
           LDB :sector    ; load the sector number
           LDX :buffer    ; load the buffer address
           JSR READ       ; call the read routine 
           STB :error     ; store the error code
    }
   
    return error; // return the error code
    
}

// write a sector to the disk
uint8_t writeSector(uint8_t track, uint8_t sector, uint16_t* buffer) {
    uint8_t error = 0; // hold the value of the error to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.

    asm {
           LDA :track     ; load the track number 
           LDB :sector    ; load the sector number
           LDX :buffer    ; load the buffer address
           JSR WRITE      ; call the write routine 
           STB :error     ; store the error code
    }
   
    return error; // return the error code
}

// verify the last sector written true = verified, false = not verified error code returned in variable 'error'

bool verifySector(uint8_t* error) {
    bool state = true; // hold the value of state of the verify to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.
    asm {
            JSR VERIFY    ; call the verify routine
            BEQ ENDSTATE  ; if no error, branch to ENDVERIFY  
            CLR :state    ; if an error exists clear to 'false'
        ENDVERIFY:
            STB :error    ; store the error code to variable 'error'
    
    }
    return state; // return the state of the verify
}

// RTZ the drive: variable 'drive' is the FCB for the drive to reset -- this is nebulus 
// RESTORE looks at drive + 3 to find the drive number, so you can point to an actual FCB 
// or just a dummy variable that is 4 bytes long and the drive number is in the 4th byte
// This function is the same on all the below that use the drive number.

uint8_t rtzDrive(uint16_t* drive) {
    uint8_t error = 0; // hold the value of the error to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.

    asm {
            LDX :drive     ; load the drive address
            JSR RESTORE    ; call the restore routine 
            STB :error     ; store the error code
    }
   
    return error; // return the error code
}

// set the current drive
uint8_t setDrive(uint16_t* drive) {
    uint8_t error = 0; // hold the value of the error to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.

    asm {
            LDX :drive     ; load the drive address
            JSR DRIVE      ; call the set drive routine 
            STB :error     ; store the error code
    } 
   
    return error; // return the error code
}

// check the current drive ready status
bool chkDrive(uint16_t* drive, uint8_t* error) {
    bool state = true; // hold the value of the state to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.
    asm {
            LDX :drive     ; load the drive address
            JSR CHKRDY     ; call the check drive routine 
            BEQ ENDCHECK   ; if no error, branch to ENDCHECK
            CLR :state     ; if an error exists clear to 'false'
        ENDCHECK:
            STB :error     ; store the error code to variable 'error'   

    }   
    return state; // return the state of the check
}

// check the current drive ready status without waiting
bool quickCheckDrive(uint16_t* drive, uint8_t* error) {
    bool state = true; // hold the value of the state to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.
    asm {
            LDX :drive     ; load the drive address
            JSR QUICK      ; call the quick check drive routine 
            BEQ ENDQUICK   ; if no error, branch to ENDQUICK
            CLR :state     ; if an error exists clear to 'false'
        ENDQUICK:
            STB :error     ; store the error code to variable 'error'   

    }

    
    return state; // return the state of the check
}

// initialize the drive hardware from cold
asm initDrive() {
    asm {
            JSR INIT       ; call the initialize drive routine 
    }       
}

// warm start the drive hardware
asm warmDrive() {
    asm {
            JSR WARM       ; call the warm start drive routine 
    }
    
}

// seek to the specified track and sector
uint8_t seekDrive(uint8_t track, uint8_t sector) {
    uint8_t error = 0; // hold the value of the error to return
 
        // this could be reworked to save the variable, by putting a in the return value locaiton on the stack.
        // have not quite worked how to do that.
    asm {
            LDA :track     ; load the track number 
            LDB :sector    ; load the sector number
            JSR SEEK       ; call the seek routine 
            STB :error     ; store the error code
    }
    
    return error; // return the error code

}          
