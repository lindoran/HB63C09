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
char getCharNoEcho(void);     // wait and return a character from the console without echo
char getChar(void);           // wait and return a character from the console with echo
bool getTerminalState(void);  // return 1 if a character is available
void outChar(char c);      // output a character to the console

// control flow
void exitMON(void);            // exit to the monitor
void exitFLEX(void);           // exit to the flex system (this is not part of the console package)

// disk io
uint8_t readSector(uint8_t track, uint8_t sector, void* buffer) ; // read a sector from the disk
uint8_t writeSector(uint8_t track, uint8_t sector, void* buffer); // write a sector to the diskc
uint8_t verifySector(void);                                               // verify the last sector written       
uint8_t rtzDrive(void* drive);                                    // reset the drive
uint8_t setDrive(void* drive);                                    // set the current drive  
uint8_t chkDrive(void* drive);                       // check the current drive ready status
uint8_t quickCheckDrive(void* drive);                // check the current drive ready status without waiting        
void initDrive(void);                                                     // initialize the drive hardware 
void warmDrive(void);                                                     // warm start the drive hardware 
uint8_t seekDrive(uint8_t track, uint8_t sector);                     // seek to the specified track and sector


#endif /* fbios_h */