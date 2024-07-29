// vcmos.h
//PetitFS licence:
/* 
Attribution: 

IOS/Z80-MBC Code for Floppy emulation is (C) Fabio Defabis under the GPL 3.0
In line call outs within the code specify where this is the case. (in HB63C09M main sketch)
Details for the Z80-MBC2 project are: 
https://github.com/SuperFabius/Z80-MBC2

SD library from: https://github.com/greiman/PetitFS (based on 
PetitFS: http://elm-chan.org/fsw/ff/00index_p.html)

PetitFS licence:
/-----------------------------------------------------------------------------/
/  Petit FatFs - FAT file system module  R0.03                  (C)ChaN, 2014
/-----------------------------------------------------------------------------/
/ Petit FatFs module is a generic FAT file system module for small embedded
/ systems. This is a free software that opened for education, research and
/ commercial developments under license policy of following trems.
/
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * The Petit FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------/
*/


#ifndef VCMOS_H
#define VCMOS_H
#include "const.h"
#include "PetitFS.h"



#define NUM_VARIABLES 4                      // Define the maximum number of variables
#define VAR_SYMBOL_LENGTH 10                 // max symbol character size for variable symbols
#define VAR_FORMAT_LENGTH 5                  // max format character length for variable formater 

// Define maximum input length 
#define MAX_INPUT_LENGTH 40 

// external globals
extern uint16_t biosStart;                   // start of the system rom in memory (this will eventually be stored in EEPROM)
extern uint16_t biosSize;                    // this is the size to load to memory before reset. (this will eventually be stored in EEPROM)
extern char     biosName[MAX_FN_LENGTH];     // this is the filename in the root of the sd card to load.
extern FRESULT  errCodeSD;                   // Temporary variable to store error codes from the PetitFS
extern FILINFO  fno;                         // current directory file info
extern DIR      dir;                         // current directory 
extern DIR      lastDir;                     // temp directory storage, the last entered directory
extern char     filePath[MAX_PT_LENGTH];      // filePath space for the current calculated path 
extern char     curPath[MAX_FN_LENGTH];      // this is the current path name.


// external functions 
extern void updateEEPROM(void);
extern void printErrSD(byte opType, byte errCode, const char* fileName);


// Function prototypes
uint8_t processCommand(const String& command);
String readSerialLine();
void showVariables();
FRESULT scan_files(void);
FRESULT change_dir(const char *path);
void buildFilePath(const char* directory, const char* filename);

#endif /* VCMOS_H */
