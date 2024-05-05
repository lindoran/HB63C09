// vcmos.h

#ifndef VCMOS_H
#define VCMOS_H
#include "const.h"
#include "PetitFS.h"


// Define the maximum number of variables
#define NUM_VARIABLES 3

// ///#define MAX_FILES_PER_LINE 4    

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

// external functions 
extern void updateEEPROM(uint16_t newStart, uint16_t newSize, const char* newName);
extern void printErrSD(byte opType, byte errCode, const char* fileName);


// Function prototypes
uint8_t processCommand(const String& command);
String readSerialLine();
void showVariables();
FRESULT scan_files(void);
FRESULT change_dir(const char *path);

#endif /* VCMOS_H */
