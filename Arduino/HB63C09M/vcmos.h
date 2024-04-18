// vcmos.h

#ifndef VCMOS_H
#define VCMOS_H

// Define the maximum number of variables
#define NUM_VARIABLES 3

// Define the maximum filename characters for variable value
#define MAX_FN_LENGTH 11

// Define maximum input length 
#define MAX_INPUT_LENGTH 40 

extern const uint16_t DEFAULT_BIOS_START;
extern const uint16_t DEFAULT_BIOS_SIZE;

extern uint16_t biosStart;                   // start of the system rom in memory (this will eventually be stored in EEPROM)
extern uint16_t biosSize;                    // this is the size to load to memory before reset. (this will eventually be stored in EEPROM)
extern char     biosName[MAX_FN_LENGTH];     // this is the filename in the root of the sd card to load.

extern void updateEEPROM(uint16_t newStart, uint16_t newSize, const char* newName);



// Function prototypes
uint8_t processCommand(const String& command);
String readSerialLine();

#endif /* VCMOS_H */
