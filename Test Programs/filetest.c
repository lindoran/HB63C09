// This is to test the file system

#include <cmoc.h>

// ASSIST09 SYSTEM CALLS USED
#define OUTCH   1         // sends a character to the terminal
#define MONITR  8         // soft start ASSIST09
#pragma limit 0x9E2A      // start DISSASEMBLER reserved mem starts here

#pragma org 0x4000         // start assembling here. (this is the bottom of the 24K fixed block)

// IO range for memory-mapped devices (A000-A00F)
#define IO_BASE_ADDRESS 0xA000
#define START_WRITE_ADDRESS 0x0179

unsigned char* SELDISK   = (unsigned char*)(IO_BASE_ADDRESS + 0x02);  // Select disk
unsigned char* SELTRACK  = (unsigned char*)(IO_BASE_ADDRESS + 0x03);  // Select track
unsigned char* SELSECT   = (unsigned char*)(IO_BASE_ADDRESS + 0x04);  // Select sector
unsigned char* WRITESECT = (unsigned char*)(IO_BASE_ADDRESS + 0x05);  // Write Sector

unsigned char* ERRDISK   = (unsigned char*)(IO_BASE_ADDRESS + 0x02);  // Disk Error Register
unsigned char* READSECT  = (unsigned char*)(IO_BASE_ADDRESS + 0x03);  // Read Sector
unsigned char* SDMOUNT   = (unsigned char*)(IO_BASE_ADDRESS + 0x04);  // Mount Volume

unsigned char* SECTOR_DATA = START_WRITE_ADDRESS;   // address to start writeing at

// call the monitor to soft-start...
asm void softStart(void) {
    asm {
        swi
        fcb     MONITR      // system call to ASSIST09 Soft start to system
    }
}


int main (void) {
     //setConsoleOutHook(newOutputRoutine);            // This reddirects the console output.

     *SELDISK = 0;
     *SELTRACK = 0;
     *SELTRACK = 0; // this is a 16 bit value we must write it twice.
     *SELSECT =0;

     for (short ioByte = 0; ioByte <= 511; ioByte++ ) {
         *SECTOR_DATA = *READSECT;
         SECTOR_DATA++;

     }
     softStart();

     return (0);
}


/*
// lets redirect printf... ASSIST09 has a built in character output
asm void newOutputRoutine(void) {
    asm {
        pshs    x,b         // save registers swi handler uses them
        swi
        fcb     OUTCH       // system call to ASSIST09 Sends ch in A Reg to Screen
        puls     b,x        // put them back
    }
}
*/


