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
char getCharNoEcho();      // wait and return a character from the console without echo
char getChar();            // wait and return a character from the console with echo
bool getTerminalState();   // return 1 if a character is available
void putChar(char c);      // output a character to the console

// control flow
asm exitMON();            // exit to the monitor
asm exitFLEX();           // exit to the flex system (this is not part of the console package)

// disk io
uint8_t readSector(uint8_t track, uint8_t sector, uint16_t* buffer) ; // read a sector from the disk
uint8_t writeSector(uint8_t track, uint8_t sector, uint16_t* buffer); // write a sector to the diskc
bool verifySector(uint8_t* error);                                    // verify the last sector written       
uint8_t rtzDrive(uint16_t* drive);                                    // reset the drive
uint8_t setDrive(uint16_t* drive);                                    // set the current drive  
bool chkDrive(uint16_t* drive, uint8_t* error);                       // check the current drive ready status
bool quickCheckDrive(uint16_t* drive, uint8_t* error);                // check the current drive ready status without waiting        
asm initDrive();                                                     // initialize the drive hardware 
asm warmDrive();                                                     // warm start the drive hardware 
uint8_t seekDrive(uint8_t track, uint8_t sector);                     // seek to the specified track and sector


#endif /* fbios_h */