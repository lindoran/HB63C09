#ifndef fbios_h
#define fbios_h


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



// console io
asm char getCharNoEcho();     // wait and return a character from the console without echo
asm char getChar();           // wait and return a character from the console with echo
asm bool getTerminalState();  // return 1 if a character is available
asm outChar(char c);      // output a character to the console

// control flow
asm exitMON();            // exit to the monitor
asm exitFLEX();           // exit to the flex system (this is not part of the console package)

// disk io
asm uint8_t readSector(uint8_t track, uint8_t sector, uint16_t* buffer) ; // read a sector from the disk
asm uint8_t writeSector(uint8_t track, uint8_t sector, uint16_t* buffer); // write a sector to the diskc
asm uint8_t verifySector();                                               // verify the last sector written       
asm uint8_t rtzDrive(uint16_t* drive);                                    // reset the drive
asm uint8_t setDrive(uint16_t* drive);                                    // set the current drive  
asm uint8_t chkDrive(uint16_t* drive);                       // check the current drive ready status
asm uint8_t quickCheckDrive(uint16_t* drive);                // check the current drive ready status without waiting        
asm initDrive();                                                     // initialize the drive hardware 
asm warmDrive();                                                     // warm start the drive hardware 
asm uint8_t seekDrive(uint8_t track, uint8_t sector);                     // seek to the specified track and sector


#endif /* fbios_h */