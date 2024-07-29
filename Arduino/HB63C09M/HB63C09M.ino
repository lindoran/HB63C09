
// This is very rough - be advised - at any time this could be broken.
// I will implement a version control mechanic when we get ready to release REV4


/*
HB63C09 Sketch is (C) David Collins under the terms of the GPL 3.0 (see repo)
https://github.com/lindoran/HB63C09

Attribution: 

IOS/Z80-MBC Code for Floppy emulation is (C) Fabio Defabis under the GPL 3.0
In line call outs within the code specify where this is the case.
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

***Thank you*** 

Grant Searle, Jeff Tranter, Fabio Defabis, and Brad Rodreguez for their 
inspiring designs for without which none of this would be possibe. 

Thank you to PCBWay.Com for sponsoring the physical prototypes which were used
to develop this system see their website at http://pcbway.com

*/

#include "const.h"
#include <stdint.h>
#include <Wire.h>
#include <EEPROM.h>
#include "PetitFS.h"
#include "blockcopy.h"
#include "vbios.h"


//RAM Write accsess time - best to not mess with this
//at 20Mhz a single bit flip is 50ns at 16 it is a bit closer to 60, by introducing this short delay
//we asure the ram chip has time to respond (min accsess time of our RAM is 55ns) regardless of clock
//speed.
#define DO_TWICE_NOP() \
do { \
    __asm__ __volatile__ ("nop\n\t"); \
    __asm__ __volatile__ ("nop\n\t"); \
} while(0)


// set up READ_MODE and WRITE_MODE enum for the data direction function.
enum PinMode {
    READ_MODE,
    WRITE_MODE
};

// Forward definging these here for ease of understanding as they use the above constants 

void ddr_a_nibble(enum PinMode mode); // forward def, see code below main block. ex: ddr_a_nibble(READ_MODE);
void write_a_nibble(uint16_t data);   // forward def, see code block below ex: write_a_nibble(0xFFFE); 


// File name Defines for IOS

#define   M6X09DISK     "DSxNyy.DSK"  // Generic 6x09 disk name (from DS0N00.DSK to DS9N99.DSK)

// Global System variables  -- see const.h for readabiblity constants

uint8_t  busData = 0;                 // data for current step through the loop
uint8_t  bankReg = 0;                 // last value set in bank register
uint8_t  lastOp = 0;                  // last operation run (MSB = read / write bit, remaining bits are address)
uint8_t  curOp = 0;                   // curent operation run (MSB = read /write bit, remaining bits are addres)
uint16_t ioByteCnt;                   // Exchanged bytes counter durring an I/O operation
uint8_t  tempByte;                    // temorary byte storage
FRESULT  errCodeSD;                   // Temporary variable to store error codes from the PetitFS
FILINFO  fno;                         // current derectory file info 
DIR      dir;                         // current directory 
DIR      lastDir;                     // temp directory storage, the last entered directory 
char     filePath[MAX_PT_LENGTH];     // filePath space for the current calculated path 
char     curPath[MAX_FN_LENGTH];      // this is the current path name.
uint8_t  numReadBytes;                // Number of read bytes after a readSD() call
uint8_t  loaderReg = 0;               // loader register.
uint16_t loaderAddr= 0;               // this is the loader current address.
uint16_t biosStart;                   // start of the system rom in memory (this will eventually be stored in EEPROM)
uint16_t biosSize;                    // this is the size to load to memory before reset. (this will eventually be stored in EEPROM)
char     biosName[MAX_FN_LENGTH];     // this is the filename in the root of the sd card to load.
uint8_t  storedChecksum;              // variables for verifying EEPROM Contents
uint8_t  calculatedChecksum;          //  ''


const char *  fileNameSD;             // Pointer to the string with the currently used file name

FATFS    filesysSD;                   // Filesystem object (PetitFS library)
uint8_t  bufferSD[32];                // I/O buffer for SD disk operations (store a "segment" of a SD sector).
char  diskName[MAX_FN_LENGTH] = M6X09DISK;    // String used for virtual disk file name
uint16_t trackSel;                    // Store the current track number [0..511]
uint8_t  sectSel;                     // Store the current sector number [0..31]
uint8_t  diskErr = 19;                // SELDISK, SELSECT, SELTRACK, WRITESECT, READSECT or SDMOUNT resulting 
                                      //  error code
uint8_t  numWriBytes;                 // Number of written bytes after a writeSD() call
uint8_t  diskSet = 0;                 // Current "Disk Set"  -- TODO*** NEED TO BUILD SUPPORT FOR MULTIPLE SETS ***

// constants
const byte    maxDiskNum   = 99;          // Max number of virtual disks


void setup() {
busTstate();               // This tri-states the MCU bus -- it is pulled low by external pulldowns 

//outputs
bitSet(DDRD, IOGNT_); // these two lines set the grant signal 
bitSet(PORTD, IOGNT_);
bitSet(DDRB, RES_); // seting these to outputs simply sets them low
bitSet(DDRB, HALT_);
bitSet(DDRD, BCLK); // set up the bank clock pin to output

// 63C09 System is in tri-state

bankReg = 0;        // bank register is reset along with the 63C09 by the avr we need to update the stored value
Serial.begin(115200);  // set up uart

// Stageing state
bitSet(DDRD, XSIN_);   // inhibit the bus tranceiver (tri-state bioreq_)
bitSet(DDRD, IOREQ_);   // set tranceiver to enable output, bioreq_ = 0
bitSet(DDRB, R_W);     // set R_W LINE pin to an output
ddr_a_nibble(WRITE_MODE);  // set the nibble writer to write mode.

while(!Serial); // waiting for USB Serial to load.
// check for configuration 
RAMWrite(42,0xFFFF);    //write the meaning of life.
// its ram?
if (RAMRead(0xFFFF) == 42) {
  Serial.println(F("Staging From RAM..."));
  Serial.println(F("Bootstrap Code loading at 0xFFC0...") );  
  loaderAddr = blockcopy_blks[0].start;

  for (unsigned int j = 0; j < blockcopy_blks[0].len; ++j) {
    RAMWrite(blockcopy_blks[0].data[j],loaderAddr);
    loaderAddr++;
  }
  Serial.println(F("Setting Reset Vector..."));
  // set the reset vector to the begining of the loader
  RAMWrite(0xFF, 0xFFFE);
  RAMWrite(0xC0, 0xFFFF);  
  
  // mount the SD Card to initiaize HB63C09 - IOS Floppy emulation 
  // see Attribuition at top
  Serial.print("IOS: Attempting to mount SD Card");
  if (mountSD(&filesysSD))
   // Error mounting. Try again
   {
     errCodeSD = mountSD(&filesysSD);
     errCodeSD = mountSD(&filesysSD);
     if (errCodeSD)
     // Error again. Repeat until error disappears (or the user forces a reset)
     do
     {
       printErrSD(0, errCodeSD, NULL);
       waitKey();                                // Wait a key to repeat
       mountSD(&filesysSD);                      // New double try
       errCodeSD = mountSD(&filesysSD);
     }
     while (errCodeSD);
     Serial.println(F("IOS: SD Card found!")) ;
   }
  else Serial.println(F("...OK!"));
  // Check EEPROM CONTENTS
  Serial.println();  
  Serial.println(F("Current vBIOS settings:"));
  // Read from EEPROM
  readEEPROM();

  // Calculate checksum
  calculatedChecksum = calculateChecksum(biosStart, biosSize, biosName);

  if (storedChecksum == calculatedChecksum) {
    
    // Print each variable with its label
    showVariables(); // show current EEPROM Settings
    Serial.println();
    
    Serial.println(F("press <ESC> for vBIOS"));
    Serial.println();
    if (waitForEscape(3000)) {
      showLeadIn(); // show the vBios Leadin on serial out
      change_dir("/"); // set to root directory, this sets up the defalut state.
      vbiosStart(); // call the interpreter
      
    }   


  } else {
      // Data is invalid or EEPROM is not initialized, update with default values
      Serial.println(F("Data in EEPROM is invalid or uninitialized. starting vBIOS..."));
      // Update with default values
      // updateEEPROM(DEFAULT_BIOS_START, DEFAULT_BIOS_SIZE, DEFAULT_BIOS_NAME);
      Serial.println();
      showLeadIn(); // show the vBios Leadin 
      change_dir("/"); // set to root directory, this sets up the defalut state.
      
      vbiosStart(); // call the interpereter
  }
  
  buildFilePath(curPath, biosName);
  Serial.printf("IOS: Mounting %s volume...",filePath);
  diskErr = openSD(filePath);       // open the bios volume
  if (diskErr) {
    printErrSD(1,diskErr,biosName); // print error message
    Serial.println(F(" ... Halt!"));
    while(1);  // halt.
  }
  // seek to 0 just in case 
  diskErr = seekSD(0);
   if (diskErr) {
    printErrSD(4,diskErr,biosName); // print error message
    Serial.println(F(" ... Halt!"));
    while(1);  // halt.
  }
} else {  // its ROM...
  Serial.println(F("\nStaging from ROM..."));

}

Serial.println();
Serial.println(F("Switching Bus Mastering to HC63C09..."));
// run state 
write_a_nibble(0);      
ddr_a_nibble(READ_MODE); // address bus should be tri-stated
bitClear(DDRD, IOREQ_);
bitClear(PORTB, R_W);
bitClear(DDRB, R_W);
bitClear(DDRD, XSIN_);


Serial.println(F("HC63C09 is now on the bus..."));

// flush the RX buffer to clear spurius inputs due to dongle power up
// this avoids issues displaying the incomming prompt text.
 while (Serial.available() > 0) 
  {
    Serial.read();
  }


// Lets burn this candle! -- System coming out of reset state
// these two have external pull ups- setting the lines on the arduino to inputs effectively tri-states the link between the two chips

bitClear(DDRB, RES_);
bitClear(DDRB, HALT_);

}


// Main loop

void loop(){
  // checking for IOREQ_
  if (!(bitRead(PIND, IOREQ_))) { 
      if (!(bitRead(PIND, XSIN_))) {
        busIO();  // end early for expansion bus IO
        lastOp = 255;    // this just makes sure the bytecount logic is reset cleanly
      }
      else {
       curOp = (bitRead(PINB, R_W) << NIBBLE_BITS) | A_NIBBLE;  // read R_W and the address bus
       if (!(lastOp == curOp)) ioByteCnt = 0;   // do we need to reset the multi-op counter?
       
       /*index of operations: 
        *-------------------- 
        *
        *  WRITE OPERATIONS: 
        *   0xA000    - NULL       - Legacy 6850 UART Control register. May use later (future use)
        *   0xA001    - TXSERIAL   - 6850 Wrapper: Send a byte to TX Buffer (default buffer size is 64bytes)
        *   0xA002    - SELDISK*   - Select Disk Number ie: 0..99
        *   0xA003    - SELTRACK*  - Select Disk Track  ie: 0..511 (two bytes in sequence)
        *   0xA004    - SELSECT*   - Select Disk Sector ie: 0..31  
        *   0xA005    - WRITESECT* - Write a Sector to the Disk (512 bytes in sequence) 
        *   ...
        *   0xA03F    - LOADERR    - This is the loader register, it is unlocked by writing 255 after boot up.
        *   0xA03F    - SETBANK    - Write the low nibble (3 bits) on the data bus to the bank register
        *  
        *  READ OPERATIONS:  
        *   0xA000    - UARTSTAT   - 6850 Wrapper: Send data back as if it were a 6850 Status regeister.(SEE ACCEPTIONS IN CASE)
        *   0xA001    - RXSERIAL   - 6850 Wrapper: Read a byte from the RX buffer (defalut buffer size is 64bytes)
        *   0xA002    - ERRDISK*   - Read out the last disk error
        *   0xA003    - READSECT*   - Read a sector from the disk (512 bytes in sequence) 
        *   0xA004    - SDMOUNT*    - Mount the installed volume (output error code as read value)
        *   ...
        *   0xA03E    - LOADER     - This is the loader port its typically locked to the user.
        *   0xA03F    - RDBANK     - Read the last selected bank value 
        *   
        *    * - Part of IOS/Z80-MBC Code see attribuition at top.
       */
       
       // bus write
       if (!(bitRead(curOp, NIBBLE_BITS))) { // R_W value
           
           busData = busRead();    // read the waiting data from the bus.
           
           switch (curOp & ADDRESS_MASK) { // address nibble
             case 0x00:
               // NULL
               //This is normally the UART control register for the 6850, since it is internally
               //configured in the AVR we don't need to store this information. but we need to 
               //leave this for legacy support of older code. 
            
              break;
             
             case 0x01:
              // send a byte to the terminal.
              // TX SERIAL 
              
              Serial.write(busData);

              break;
              
             // begin IOS Emulation code for write, see Attribution at the top
             case 0x02:
              // DISK EMULATION
              // SELDISK - select the emulated disk number (binary). 100 disks are supported [0..99]:
              //
              //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
              //                            ---------------------------------------------------------
              //                             D7 D6 D5 D4 D3 D2 D1 D0    DISK number (binary) [0..99]
              //
              //
              // Opens the "disk file" correspondig to the selected disk number, doing some checks.
              // A "disk file" is a binary file that emulates a disk using a LBA-like logical sector number.
              // Every "disk file" must have a dimension of 8388608 bytes, corresponding to 16384 LBA-like logical sectors
              //  (each sector is 512 bytes long), correspinding to 512 tracks of 32 sectors each (see SELTRACK and
              //  SELSECT Opcodes).
              // Errors are stored into "errDisk" (see ERRDISK Opcode).
              //
              //
              // ...........................................................................................
              //
              // "Disk file" filename convention:
              //
              // Every "disk file" must follow the sintax "DSsNnn.DSK" where
              //
              //    "s" is the "disk set" and must be in the [0..9] range (always one numeric ASCII character)
              //    "nn" is the "disk number" and must be in the [00..99] range (always two numeric ASCII characters)
              //
              // ...........................................................................................
              //
              //
              // NOTE 1: The maximum disks number may be lower due the limitations of the used OS (e.g. CP/M 2.2 supports
              //         a maximum of 16 disks)
              // NOTE 2: Because SELDISK opens the "disk file" used for disk emulation, before using WRITESECT or READSECT
              //         a SELDISK must be performed at first.

              if (busData <= maxDiskNum)             // Valid disk number
              // Set the name of the file to open as virtual disk, and open it
              {
                diskName[2] = diskSet + 48;         // Set the current Disk Set
                diskName[4] = (busData / 10) + 48;   // Set the disk number
                diskName[5] = busData - ((busData / 10) * 10) + 48;
                diskErr = openSD(diskName);         // Open the "disk file" corresponding to the given disk number
              }
              else diskErr = 16;                    // Illegal disk number
              break;
              
             case 0x03:
              // DISK EMULATION
              // SELTRACK - select the emulated track number (word split in 2 bytes in sequence: DATA 0 and DATA 1):
              //
              //                I/O DATA 0:  D7 D6 D5 D4 D3 D2 D1 D0
              //                            ---------------------------------------------------------
              //                             D7 D6 D5 D4 D3 D2 D1 D0    Track number (binary) MSB [0..1]
              //
              //                I/O DATA 1:  D7 D6 D5 D4 D3 D2 D1 D0
              //                            ---------------------------------------------------------
              //                             D7 D6 D5 D4 D3 D2 D1 D0    Track number (binary) LSB [0..255]
              //
              //
              // Stores the selected track number into "trackSel" for "disk file" access.
              // A "disk file" is a binary file that emulates a disk using a LBA-like logical sector number.
              // The SELTRACK and SELSECT operations convert the legacy track/sector address into a LBA-like logical 
              //  sector number used to set the logical sector address inside the "disk file".
              // A control is performed on both current sector and track number for valid values. 
              // Errors are stored into "diskErr" (see ERRDISK Opcode).
              //
              //
              // NOTE 1: Allowed track numbers are in the range [0..511] (512 tracks)
              // NOTE 2: Before a WRITESECT or READSECT operation at least a SELSECT or a SELTRAK operation
              //         must be performed
              // NOTE 3: Big endian numbering so MSB is written first. (this differs from the Z80MBC and the V20MBC)

  
              if (!ioByteCnt)
              // MSB
              {
                trackSel = ((word)busData) << 8;
              }
              else
              // LSB
              {
                trackSel |= busData;

                if ((trackSel < 512) && (sectSel < 32))
                // Sector and track numbers valid
                {
                  diskErr = 0;                      // No errors
                }
                else
                // Sector or track invalid number
                {
                  if (sectSel < 32) diskErr = 17;   // Illegal track number
                  else diskErr = 18;                // Illegal sector number
                }
                
              }
              ioByteCnt++;
              break;
              
             case 0x04:
              // DISK EMULATION
              // SELSECT - select the emulated sector number (binary):
              //
              //                  I/O DATA:  D7 D6 D5 D4 D3 D2 D1 D0
              //                            ---------------------------------------------------------
              //                             D7 D6 D5 D4 D3 D2 D1 D0    Sector number (binary) [0..31]
              //
              //
              // Stores the selected sector number into "sectSel" for "disk file" access.
              // A "disk file" is a binary file that emulates a disk using a LBA-like logical sector number.
              // The SELTRACK and SELSECT operations convert the legacy track/sector address into a LBA-like logical 
              //  sector number used to set the logical sector address inside the "disk file".
              // A control is performed on both current sector and track number for valid values. 
              // Errors are stored into "diskErr" (see ERRDISK Opcode).
              //
              //
              // NOTE 1: Allowed sector numbers are in the range [0..31] (32 sectors)
              // NOTE 2: Before a WRITESECT or READSECT operation at least a SELSECT or a SELTRAK operation
              //         must be performed
  
              sectSel = busData;
              if ((trackSel < 512) && (sectSel < 32))
              // Sector and track numbers valid
              {
                diskErr = 0;                        // No errors
              }
              else
              // Sector or track invalid number
              {
                if (sectSel < 32) diskErr = 17;     // Illegal track number
                else diskErr = 18;                  // Illegal sector number
              }
              break;

             case 0x05:
              // DISK EMULATION
              // WRITESECT - write 512 data bytes sequentially into the current emulated disk/track/sector:
              //
              //                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
              //                            ---------------------------------------------------------
              //                             D7 D6 D5 D4 D3 D2 D1 D0    First Data byte
              //
              //                      |               |
              //                      |               |
              //                      |               |                 <510 Data Bytes>
              //                      |               |
              //
              //
              //               I/O DATA 511: D7 D6 D5 D4 D3 D2 D1 D0
              //                            ---------------------------------------------------------
              //                             D7 D6 D5 D4 D3 D2 D1 D0    512th Data byte (Last byte)
              //
              //
              // Writes the current sector (512 bytes) of the current track/sector, one data byte each call. 
              // All the 512 calls must be always performed sequentially to have a WRITESECT operation correctly done. 
              // If an error occurs during the WRITESECT operation, all subsequent write data will be ignored and
              //  the write finalization will not be done.
              // If an error occurs calling any DISK EMULATION Opcode (SDMOUNT excluded) immediately before the WRITESECT 
              //  Opcode call, all the write data will be ignored and the WRITESECT operation will not be performed.
              // Errors are stored into "diskErr" (see ERRDISK Opcode).
              //
              // NOTE 1: Before a WRITESECT operation at least a SELTRACK or a SELSECT must be always performed
              // NOTE 2: Remember to open the right "disk file" at first using the SELDISK Opcode
              // NOTE 3: The write finalization on SD "disk file" is executed only on the 512th data byte exchange, so be 
              //         sure that exactly 512 data bytes are exchanged.
  
              if (!ioByteCnt)
              // First byte of 512, so set the right file pointer to the current emulated track/sector first
              {
                if ((trackSel < 512) && (sectSel < 32) && (!diskErr))
                // Sector and track numbers valid and no previous error; set the LBA-like logical sector
                {
                diskErr = seekSD((trackSel << 5) | sectSel);  // Set the starting point inside the "disk file"
                                                              //  generating a 14 bit "disk file" LBA-like 
                                                              //  logical sector address created as TTTTTTTTTSSSSS
                }
              }
            
  
              if (!diskErr)
              // No previous error (e.g. selecting disk, track or sector)
              {
                tempByte = ioByteCnt % 32;          // [0..31]
                bufferSD[tempByte] = busData;        // Store current exchanged data byte in the buffer array
                if (tempByte == 31)
                // Buffer full. Write all the buffer content (32 bytes) into the "disk file"
                {
                  diskErr = writeSD(bufferSD, &numWriBytes);
                  if (numWriBytes < 32) diskErr = 19; // Reached an unexpected EOF
                  if (ioByteCnt >= 511)
                  // Finalize write operation and check result (if no previous error occurred)
                  {
                    if (!diskErr) diskErr = writeSD(NULL, &numWriBytes);
                   
                  }
                }
              }
              ioByteCnt++;                          // Increment the counter of the exchanged data bytes
              break;

             case 0x3E:
              // LOADERR
               switch(busData) {
                case 0x01:
                  //reset the system
                  if (loaderReg == 0xFF) {  
                  busIO();
                  bitSet(DDRB, RES_);
                  bitClear(DDRB, RES_);  // system will reset at the end of the IO request.
                  loaderReg = 0;
                  }
                  else loaderReg = 0;
                  break;
                
                case 0x02:
                  //halt the system
                  if (loaderReg == 0xFF) {
                    busIO();
                    bitSet(DDRB, HALT_);
                    Serial.println(F("\n System Halted."));
                    while(0);  // infinate loop.
                  } 
                  else {
                    loaderReg = 0;

                  } 
                  break;

                case 0xff:
                  //unlock the register
                  loaderReg = 0xFF;
                  break;
                default:
                  //any other value set to zero (locked)
                  loaderReg = 0;
                  break;
              }
              break;
               
             case 0x3F:
              // SETBANK
              // write value on the bus to the bank adress latch
              bankReg = busData;
              bitSet(PORTD, BCLK);
              bitClear(PORTD, BCLK);
              break; 
              
             default:
              break;
              // should never jump to here - this can never be true.
          } // endcase write 
       } // endif write  
       // bus read
       else {
            busData = 0;  // flush busdata from last operation 
            switch (curOp & ADDRESS_MASK) { // address nibble
              
              case 0x00:
               // UARTSTAT 
               //This is the UART status register it is simalar to the 6850 (however has a built in buffer TX & RX) 
               // bit 0 = Receive Data register full (this is set if there is data waiting)
               // bit 1 = transmit data empty (this is set if TX buffer has space)
               // bit 2 = depriciated
               // bit 3 = depriciated
               // bit 4 = depricaited
               // bit 5 = buffer over/underrun - not yet implimented 
               // bit 6 = depricated 
               // bit 7 = irq  - not yet implimented
               
               // buffers are manually set to 128bytes, testing has shown that there is little chance of an underun / overflow 
               // set at this level. since hardware flow control is not used, the other status bits are not set.  
               // /DCD, /CTS and FE are depreciated.

               // TODO - write code for bits 5 and 7 which will interupt the CPU if the buffer has under/over run
               
               if (Serial.available() > 0) bitSet(busData, 0);
               if (Serial.availableForWrite() > 0) bitSet(busData, 1);
              
               break; // end of read uart control register
              
              case 0x01:
               //RXSERIAL
               //this is reading a byte from the terminal. so we need to write the next byte in
               //the uart to the bus.

               busData = Serial.read();
                 // TODO: -- Needs to 
               break; // end of read uart 
              
              case 0x02:
               // DISK EMULATION
               // ERRDISK - read the error code after a SELDISK, SELSECT, SELTRACK, WRITESECT, READSECT 
               //           or SDMOUNT operation
               //
               //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
               //                            ---------------------------------------------------------
               //                             D7 D6 D5 D4 D3 D2 D1 D0    DISK error code (binary)
               //
               //
               // Error codes table:
               //
               //    error code    | description
               // ---------------------------------------------------------------------------------------------------
               //        0         |  No error
               //        1         |  DISK_ERR: the function failed due to a hard error in the disk function, 
               //                  |   a wrong FAT structure or an internal error
               //        2         |  NOT_READY: the storage device could not be initialized due to a hard error or 
               //                  |   no medium
               //        3         |  NO_FILE: could not find the file
               //        4         |  NOT_OPENED: the file has not been opened
               //        5         |  NOT_ENABLED: the volume has not been mounted
               //        6         |  NO_FILESYSTEM: there is no valid FAT partition on the drive
               //       16         |  Illegal disk number
               //       17         |  Illegal track number
               //       18         |  Illegal sector number
               //       19         |  Reached an unexpected EOF
               //
               //
               //
               //
               // NOTE 1: ERRDISK code is referred to the previous SELDISK, SELSECT, SELTRACK, WRITESECT or READSECT
               //         operation
               // NOTE 2: Error codes from 0 to 6 come from the PetitFS library implementation
               // NOTE 3: ERRDISK must not be used to read the resulting error code after a SDMOUNT operation 
               //         (see the SDMOUNT Opcode)
               
               busData = diskErr;
               break;
               
              case 0x03:
               // DISK EMULATION
               // READSECT - read 512 data bytes sequentially from the current emulated disk/track/sector:
               //
               //                 I/O DATA:   D7 D6 D5 D4 D3 D2 D1 D0
               //                            ---------------------------------------------------------
               //                 I/O DATA 0  D7 D6 D5 D4 D3 D2 D1 D0    First Data byte
               //
               //                      |               |
               //                      |               |
               //                      |               |                 <510 Data Bytes>
               //                      |               |
               //
               //               I/O DATA 127  D7 D6 D5 D4 D3 D2 D1 D0
               //                            ---------------------------------------------------------
               //                             D7 D6 D5 D4 D3 D2 D1 D0    512th Data byte (Last byte)
               //
               //
               // Reads the current sector (512 bytes) of the current track/sector, one data byte each call. 
               // All the 512 calls must be always performed sequentially to have a READSECT operation correctly done. 
               // If an error occurs during the READSECT operation, all subsequent read data will be = 0.
               // If an error occurs calling any DISK EMULATION Opcode (SDMOUNT excluded) immediately before the READSECT 
               //  Opcode call, all the read data will be will be = 0 and the READSECT operation will not be performed.
               // Errors are stored into "diskErr" (see ERRDISK Opcode).
               //
               // NOTE 1: Before a READSECT operation at least a SELTRACK or a SELSECT must be always performed
               // NOTE 2: Remember to open the right "disk file" at first using the SELDISK Opcode
  
               if (!ioByteCnt)
               // First byte of 512, so set the right file pointer to the current emulated track/sector first
               {
                 if ((trackSel < 512) && (sectSel < 32) && (!diskErr))
                 // Sector and track numbers valid and no previous error; set the LBA-like logical sector
                 {
                  diskErr = seekSD((trackSel << 5) | sectSel);  // Set the starting point inside the "disk file"
                                                              //  generating a 14 bit "disk file" LBA-like 
                                                              //  logical sector address created as TTTTTTTTTSSSSS
                 }
               }
               if (!diskErr)
               // No previous error (e.g. selecting disk, track or sector)
               {
                 tempByte = ioByteCnt % 32;        // [0..31]
                 if (!tempByte)
                 // Read 32 bytes of the current sector on SD in the buffer (every 32 calls, starting with the first)
                 {
                   diskErr = readSD(bufferSD, &numReadBytes); 
                   if (numReadBytes < 32) diskErr = 19;    // Reached an unexpected EOF
                 }
                 if (!diskErr) busData = bufferSD[tempByte];// If no errors, exchange current data byte with the CPU
               }
               
               ioByteCnt++;                        // Increment the counter of the exchanged data bytes
               break;

              case 0x04:
               // DISK EMULATION
               // SDMOUNT - mount a volume on SD, returning an error code (binary):
               //
               //                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
               //                            ---------------------------------------------------------
               //                             D7 D6 D5 D4 D3 D2 D1 D0    error code (binary)
               //
               //
               //
               // NOTE 1: This Opcode is "normally" not used. Only needed if using a virtual disk from a custom program
               //         loaded with iLoad or with the Autoboot mode (e.g. ViDiT). Can be used to handle SD hot-swapping
               // NOTE 2: For error codes explanation see ERRDISK Opcode
               // NOTE 3: Only for this disk Opcode, the resulting error is read as a data byte without using the 
               //         ERRDISK Opcode
  
               busData = mountSD(&filesysSD);
               break;  

              case 0x3E:
                // LOADER
                // This will load the system in a "ROMLESS" memory configuration 
                if (loaderReg == 255) {        // check to see if the loader is unlocked
                  switch(ioByteCnt) {
                    case 0x00:
                      // send the start address MSB
                      busData = highByte(biosStart);

                    break; // will this break
                    case 0x01:
                      // send the start address LSB
                      busData = lowByte(biosStart);

                    break;
                    case 0x02:
                      // send the number of bytes to read MSB
                      busData = (biosSize >> 8) & 0xFF;

                    case 0x03:
                      // send the number of bytes to read LSB
                      busData = biosSize & 0xFF;

                    break;
                    
                    default:
                      //send the data                          
                      if (!diskErr)
                      // No previous error (e.g. fs issue)
                      {
                        tempByte = (ioByteCnt-4) % 32;        // [0..31]
                        if (!tempByte)
                        // Read 32 bytes of the current sector on SD in the buffer (every 32 calls, starting with the first)
                        {
                          diskErr = readSD(bufferSD, &numReadBytes); 
                          if (numReadBytes < 32) diskErr = 19;    // Reached an unexpected EOF
                        }
                        if (!diskErr) busData = bufferSD[tempByte];// If no errors, exchange current data byte with the CPU
                      }
                    break;  
                  }
                  ioByteCnt++; 
                  // clean up after loader if done.
                  if  ((ioByteCnt > (biosSize+4)) && ( loaderReg = 255) ) { 
                    // we are done loading the data and must reset or the computer will hang.
                    loaderReg = 0; // re-lock the loader
                    lastOp = 255;  // reset last op (this value makes certain a new operation is initiated.)
                    busIO();       // send the cpu back to run mode
                    bitSet(DDRB, RES_);
                    bitClear(DDRB, RES_); // reset
                  }
                  
                } 
                break;
                
              case 0x3F:
               //RDBANK 
               //this reads out last value stored in bank register.
               busData = bankReg;
               break;
               
              default:
               break;
               // should never jump here, this can never be true.
            } // endcase read
            busWrite(busData);  // data is on the bus
       } // inner else read
       lastOp = curOp;
       busIO();  // end the current Io Request as data is ready to be read or written;
      } // outer else R_W
      
  
     } // io request
     

} // end - on to next loop

// tri-state the bus
void busTstate(void) {
    DDRA = 0x00;            // input
    PORTA = 0x00;           // atmega bus is in tri-state
}

//read a bite, asumes bus is configured to read.
uint8_t busRead(void) {
  return(PINA); 
}

//write a byte, leaves the bus in write, must be tri-stated with busTstate()

void busWrite(uint8_t data) {
  DDRA = 0xFF;     // bus is output
  PORTA = data; // data is on the bus
}

//end the current io request
void busIO(void) {
      noInterrupts();  // this next bit is very timing sensitive.
      // send bgnt_
      bitClear(PORTD, IOGNT_);    // this ends the io request - cpu is free running from this point 
      bitSet(PORTD, IOGNT_);      // the current cycle is ended - we need to restore this as the address bus is building
      busTstate(); // this just makes sure we are ready to read on the next go-through.
      interrupts();  // back to it
}

// this is for configuring the address bus pins.

void ddr_a_nibble(enum PinMode mode) {
    if (mode == READ_MODE) {
        DDRC &= ~NIBBLE_MASK;
    } else if (mode == WRITE_MODE) {
        DDRC |= NIBBLE_MASK;
    }
}

void write_a_nibble(uint16_t data) {
    // Drop the bits beyond the first 6
    uint8_t trimmed_data = (uint8_t)(data & ADDRESS_MASK);

    // Clear the current value on PORTC
    PORTC &= ~NIBBLE_MASK;

    // Write the new value to PORTC
    PORTC |= (trimmed_data << 2);
    DO_TWICE_NOP();
}


//write to RAM in staging mode
void RAMWrite(uint8_t ioData,uint16_t addr) {

    write_a_nibble(addr);  // sets the address bus to the address to write to.
  
    bitSet(PORTB, R_W);    // set R_W HIGH to WRITE from the AVR to the bus (this is inverse from the CPU perspective) 
    busWrite(ioData);      // places the data on the bus          
    bitSet(DDRD, WR_);     // clock the write pin
            
    // we waste some time as at 20Mhz the cycle time is less than the actual needed hold time for the RAM chip. 
    DO_TWICE_NOP();
    
    bitClear(DDRD,WR_);    
    busTstate();           // tri-state dataport to avoid bus contention 
}

//Read from ram in staging mode
uint8_t RAMRead(uint16_t addr) {
    
    write_a_nibble(addr);   // sets the address bus to the address to read from 
    
    uint8_t ioData = 0;
    
    bitClear(PORTB, R_W);   // set the AVR to READ from the bus
    bitSet(DDRD, RD_);
    
    // we waste some time as at 20Mhz the cycle time is less than the actual needed hold time for the RAM chip
    DO_TWICE_NOP();
    
    ioData = busRead();
    bitClear(DDRD, RD_);
    busTstate();
    return(ioData); 
}

void updateEEPROM(void) {
    // Update BIOS parameters in EEPROM
    EEPROM.put(BIOS_START_ADDR, biosStart);
    EEPROM.put(BIOS_SIZE_ADDR, biosSize);
    // blank space to update eeprom
    for (int i = 0; i < MAX_FN_LENGTH; i++) {
        EEPROM.write(BIOS_NAME_ADDR + i, 0);
    }
    for (int i = 0; i < strlen(biosName); i++) {
        EEPROM.write(BIOS_NAME_ADDR + i, biosName[i]);
    }
    // blank space to update eeprm
    for (int i = 0; i < MAX_FN_LENGTH; i++) {
        EEPROM.write(BIOS_PATH_ADDR + i, 0);
    }
    for (int i = 0; i < MAX_FN_LENGTH; i++) {
        EEPROM.write(BIOS_PATH_ADDR + i, curPath[i]);
    }
    // Update checksum
    uint8_t newChecksum = calculateChecksum(biosStart,biosSize,biosName);
    EEPROM.write(CHECKSUM_ADDR, newChecksum);
 
}

void readEEPROM(void) {
  EEPROM.get(BIOS_START_ADDR, biosStart);
  EEPROM.get(BIOS_SIZE_ADDR, biosSize);
  for (int i = 0; i < sizeof(biosName); i++) {
     biosName[i] = EEPROM.read(BIOS_NAME_ADDR + i);
  }
  biosName[sizeof(biosName) - 1] = '\0';  // ensure null termination 
  for (int i = 0; i < sizeof(curPath); i++) {
    curPath[i] = EEPROM.read(BIOS_PATH_ADDR + i);
  }
  curPath[sizeof(curPath) - 1] = '\0'; // ensure null termination
  EEPROM.get(CHECKSUM_ADDR, storedChecksum);
}

bool waitForEscape(int timeoutMillis) {
    unsigned long startTime = millis();
    while (millis() - startTime < timeoutMillis) {
        if (Serial.available() > 0) {
            int incomingByte = Serial.read();
            if (incomingByte == ESC_KEY) {
                return true; // Escape key pressed
            }
        }
    }
    return false; // Timeout reached, escape key not pressed
}

uint8_t calculateChecksum(uint16_t start, uint16_t size, const char* name) {
    uint8_t checksum = 0;
    checksum ^= (start & 0xFF) ^ (start >> 8); // Add bytes of start
    checksum ^= (size & 0xFF) ^ (size >> 8);   // Add bytes of size
    for (int i = 0; i < 11; i++) {             // Add bytes of name
        checksum ^= name[i];
    }
    
    // Take one's complement of the checksum
    checksum = ~checksum;
    
    return checksum;
}

void vbiosStart (void) {
while(true) {
        // if a comand is waiting...
        if(Serial.available()) {  
          // see vbios.cpp / vbios.h for detals of processing commands
          if(processCommand(readSerialLine())) {
            Serial.println();
            break;
          } else {
            Serial.println();
            Serial.print(F("> "));
          }
        } else { 
            delay(10);  // prevent wait blocking 
        }
  
      }
}

void showLeadIn(void) {
    Serial.println();
    Serial.println(F("Welcome to HB63C09M vBIOS!"));
    Serial.println(F(" (C) 2023 - D. Collins (Z80Dad)"));
    Serial.println();
    Serial.println(F("Type '?' for Commands "));
    Serial.println();
    Serial.print(F("> "));
}
// IOS SD Card routines for Z80-MBC2 floppy emulation See *** Attribution at top ***
// ------------------------------------------------------------------------------

// SD Disk routines (FAT16 and FAT32 filesystems supported) using the PetitFS library.
// For more info about PetitFS see here: http://elm-chan.org/fsw/ff/00index_p.html

// ------------------------------------------------------------------------------

void waitKey()
// Wait a key to continue
{
  while (Serial.available() > 0) Serial.read();   // Flush serial RX buffer
  Serial.println(F("IOS: Check SD and press a key to repeat\r\n"));
  while(Serial.available() < 1);
}


byte mountSD(FATFS* fatFs)
// Mount a volume on SD: 
// *  "fatFs" is a pointer to a FATFS object (PetitFS library)
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
{
  return pf_mount(fatFs);
}

// ------------------------------------------------------------------------------

byte openSD(const char* fileName)
// Open an existing file on SD:
// *  "fileName" is the pointer to the string holding the file name (8.3 format)
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
{
  return pf_open(fileName);
}

// ------------------------------------------------------------------------------

byte readSD(void* buffSD, byte* numReadBytes)
// Read one "segment" (32 bytes) starting from the current sector (512 bytes) of the opened file on SD:
// *  "BuffSD" is the pointer to the segment buffer;
// *  "numReadBytes" is the pointer to the variables that store the number of read bytes;
//     if < 32 (including = 0) an EOF was reached).
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
//
// NOTE1: Each SD sector (512 bytes) is divided into 16 segments (32 bytes each); to read a sector you need to
//        to call readSD() 16 times consecutively
//
// NOTE2: Past current sector boundary, the next sector will be pointed. So to read a whole file it is sufficient 
//        call readSD() consecutively until EOF is reached
{
  UINT  numBytes;
  byte  errcode;
  errcode = pf_read(buffSD, 32, &numBytes);
  *numReadBytes = (byte) numBytes;
  return errcode;
}

// ------------------------------------------------------------------------------

byte writeSD(void* buffSD, byte* numWrittenBytes)
// Write one "segment" (32 bytes) starting from the current sector (512 bytes) of the opened file on SD:
// *  "BuffSD" is the pointer to the segment buffer;
// *  "numWrittenBytes" is the pointer to the variables that store the number of written bytes;
//     if < 32 (including = 0) an EOF was reached.
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
//
// NOTE1: Each SD sector (512 bytes) is divided into 16 segments (32 bytes each); to write a sector you need to
//        to call writeSD() 16 times consecutively
//
// NOTE2: Past current sector boundary, the next sector will be pointed. So to write a whole file it is sufficient 
//        call writeSD() consecutively until EOF is reached
//
// NOTE3: To finalize the current write operation a writeSD(NULL, &numWrittenBytes) must be called as last action
{
  UINT  numBytes;
  byte  errcode;
  if (buffSD != NULL)
  {
    errcode = pf_write(buffSD, 32, &numBytes);
  }
  else
  {
    errcode = pf_write(0, 0, &numBytes);
  }
  *numWrittenBytes = (byte) numBytes;
  return errcode;
}

// ------------------------------------------------------------------------------

byte seekSD(word sectNum)
// Set the pointer of the current sector for the current opened file on SD:
// *  "sectNum" is the sector number to set. First sector is 0.
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
//
// NOTE: "secNum" is in the range [0..16383], and the sector addressing is continuos inside a "disk file";
//       16383 = (512 * 32) - 1, where 512 is the number of emulated tracks, 32 is the number of emulated sectors
//
{
  byte i;
  return pf_lseek(((unsigned long) sectNum) << 9);
}

// ------------------------------------------------------------------------------

void printErrSD(byte opType, byte errCode, const char* fileName)
// Print the error occurred during a SD I/O operation:
//  * "OpType" is the operation that generated the error (0 = mount, 1= open, 2 = read,
//     3 = write, 4 = seek);
//  * "errCode" is the error code from the PetitFS library (0 = no error);
//  * "fileName" is the pointer to the file name or NULL (no file name)
//
// ........................................................................
//
// Errors legend (from PetitFS library) for the implemented operations:
//
// ------------------
// mountSD():
// ------------------
// NOT_READY
//     The storage device could not be initialized due to a hard error or no medium.
// DISK_ERR
//     An error occured in the disk read function.
// NO_FILESYSTEM
//     There is no valid FAT partition on the drive.
//
// ------------------
// openSD():
// ------------------
// NO_FILE
//     Could not find the file.
// DISK_ERR
//     The function failed due to a hard error in the disk function, a wrong FAT structure or an internal error.
// NOT_ENABLED
//     The volume has not been mounted.
//
// ------------------
// readSD() and writeSD():
// ------------------
// DISK_ERR
//     The function failed due to a hard error in the disk function, a wrong FAT structure or an internal error.
// NOT_OPENED
//     The file has not been opened.
// NOT_ENABLED
//     The volume has not been mounted.
// 
// ------------------
// seekSD():
// ------------------
// DISK_ERR
//     The function failed due to an error in the disk function, a wrong FAT structure or an internal error.
// NOT_OPENED
//     The file has not been opened.
//
// ........................................................................
{
  if (errCode)
  {
    Serial.print(F("\r\nIOS: SD error "));
    Serial.print(errCode);
    Serial.print(" (");
    switch (errCode)
    // See PetitFS implementation for the codes
    {
      case 1: Serial.print(F("DISK_ERR")); break;
      case 2: Serial.print(F("NOT_READY")); break;
      case 3: Serial.print(F("NO_FILE")); break;
      case 4: Serial.print(F("NOT_OPENED")); break;
      case 5: Serial.print(F("NOT_ENABLED")); break;
      case 6: Serial.print(F("NO_FILESYSTEM")); break;
      default: Serial.print(F("UNKNOWN")); 
    }
    Serial.print(F(" on "));
    switch (opType)
    {
      case 0: Serial.print(F("MOUNT")); break;
      case 1: Serial.print(F("OPEN")); break;
      case 2: Serial.print(F("READ")); break;
      case 3: Serial.print(F("WRITE")); break;
      case 4: Serial.print(F("SEEK")); break;
      default: Serial.print(F("UNKNOWN"));
    }
    Serial.print(F(" operation"));
    if (fileName)
    // Not a NULL pointer, so print file name too
    {
      Serial.print(F(" - File: "));
      Serial.print(fileName);
    }
    Serial.println(")");
  }
}
