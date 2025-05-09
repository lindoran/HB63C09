/*
HB63C09 Sketch is (C) David Collins under the terms of the GPL 3.0 (see repo)
https://github.com/lindoran/HB63C09

Attribution: 
The Z80-MBC2 project was hevally refrenced for the creation of this board and firmware
Disk routines from that project can be found in older revisions of the code.  These 
had to be re-written but I would like to state planely that none of the work here
would have been done without the help of that project (and many others, but especially 
Fabios work) as it was instrimental in the creation of everthing you see here.

https://github.com/SuperFabius/Z80-MBC2

***Thank you*** 

Grant Searle, Jeff Tranter, Fabio Defabis, and Brad Rodreguez for their 
inspiring designs for without which none of this would be possibe. 

Thank you to PCBWay.Com for sponsoring the physical prototypes which were used
to develop this system see their website at http://pcbway.com

WARNING!!! 
-- Falure to care for open collector (ie non-push pull) on this board can cause damage!
   Take care, and verify on the schematics when driving a line high, you can cause a 
   short with just software.

   Most of the signals are only ever driven low by setting an input to an output.
   make sure you understand how the board works before modifying the code, which
   you do at your own risk!

*/

#include "const.h"  // many constants are defined here.
#include <stdint.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SD.h>
#include "blockcopy.h"  // this is the bootstrap machine code



//RAM Write accsess time - best to not mess with this
//at 20Mhz a single bit flip is 50ns at 16Mhz it is a bit closer to 60, by introducing this short delay
//we asure the ram chip has time to respond (min accsess time of our RAM is 55ns) regardless of clock
//speed.

#define F_CPU 20000000UL // may not be needed but we need to be sure board is 20Mhz

  

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

#define   M6X09DISK     "DS0N00.DSK"  // Generic 6x09 disk name (from DS0N00.DSK to DS9N99.DSK)
#define   M6X09FLPY     "FLPY00.DSK"  // Generic Floppy Name (from FLPY00.DSK to FLPY03.DSK)

// Global System variables  -- see const.h for readabiblity constants

uint8_t  busData = 0;                 // data for current step through the loop
uint8_t  bankReg = 0;                 // last value set in bank register
uint8_t  lastOp = 0;                  // last operation run (MSB = read / write bit, remaining bits are address)
uint8_t  curOp = 0;                   // curent operation run (MSB = read /write bit, remaining bits are addres)
uint16_t ioByteCnt;                   // Exchanged bytes counter durring an I/O operation
uint8_t  tempByte;                    // temorary byte storage

File     imgFile;                     // working file for current file
uint8_t  numReadBytes;                // Number of read bytes after a readSD() call
uint8_t  loaderReg = 0;               // loader register.
uint16_t loaderAddr= 0;               // this is the loader current address.
uint16_t biosStart;                   // start of the system rom in memory (this will eventually be stored in EEPROM)
uint16_t biosSize;                    // this is the size to load to memory before reset. (this will eventually be stored in EEPROM)
char     biosName[MAX_FN_LENGTH];     // this is the filename in the root of the sd card to load.
uint8_t  storedChecksum;              // variables for verifying EEPROM Contents
uint8_t  calculatedChecksum;          //  ''
uint8_t  ustatus = 0;                 // this is the current 6850 Wrapper status register for the current poling cycle.
uint8_t  rxIntEn = 0;                 // enable interupts on RDRF
uint8_t  txIntEn = 0;                 // enable interupts on TDRE
uint8_t  tmIntEn = 0;                 // enable timer interupts
uint16_t curTime = 0;                 // holding variable for timer
uint8_t  sTickTime = 10;              // default systick is 10 ms
unsigned long  uSStamp = micros();    // holds the last microsecond time stamp
uint8_t  tStatus = 0;                 // timer Status Register
uint8_t  tControl = 0;                // Timer Control register.
uint8_t  intRegister = 0;             // this register contains the status of interupt generation, this is used only by the interupt handler

uint8_t  diskSet = 0;                 // Current "Disk Set"  -- since subdirectories all disk sets are 0, each system rom will have it's own disk sets.

int16_t  diskByte = 0;                // signed int for error detection. (will return -1 on reads, we mask down to uint8_t to transfer the byte).
uint8_t  flpCmd = 0;                      // command register for soft sectord floppy controler.
uint8_t  drvNumb = 0;                     // current drive number soft sectored floppy
uint8_t  flpErr = 0;                      // error code for soft sector floppy  
uint8_t  flpTrack = 0;                    // floppy track 0 - 255
uint8_t  flpSector = 0;                   // floppy sector 0 - 254 (from flex this is 1-255)
char     flpName[MAX_FN_LENGTH]
          = M6X09FLPY;          
uint8_t  errReg = 0;


// constants
const byte maxDiskNum = 99;           // Max number of virtual disks


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
  Serial.println();
  Serial.println(F("Welcome to the vBIOS Staging Environment..."));
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
    
  // mount the SD Card to initiaize HB63C09 - vBios Floppy emulation 
  // see Attribuition at top
  Serial.println(F("vBIOS: Attempting to mount SD Card"));
  if (!SD.begin()) {
    do {
       waitKey();
       }
       while(!SD.begin());
       
  }

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
    
    Serial.println(F("press <ESC> for vBIOS settings"));
    Serial.println();
    if (waitForEscape(3000)) {

      Serial.println(F("Escape pressed! Enter BIOS configuration manually."));

      Serial.print(F("Enter BIOS start address (hex, default 0x"));
      Serial.print(DEFAULT_BIOS_START, HEX);
      Serial.print(F("): "));
      biosStart = readHexInput(DEFAULT_BIOS_START);
      Serial.println();

      Serial.print(F("Enter BIOS size (hex, default 0x"));
      Serial.print(DEFAULT_BIOS_SIZE, HEX);
      Serial.print(F("): "));
      biosSize = readHexInput(DEFAULT_BIOS_SIZE);
      Serial.println();

      Serial.print(F("Enter BIOS filename (default "));
      Serial.print(DEFAULT_BIOS_NAME);
      Serial.print(F("): "));
      readFilenameInput(biosName, MAX_FN_LENGTH, DEFAULT_BIOS_NAME);
      Serial.println();
      
      Serial.println(F("Configuration complete."));
      updateEEPROM(); // Save what was entered


    }   


  } else {
    
      // Data is invalid or EEPROM is not initialized, update with default values
      Serial.println(F("Data in EEPROM is invalid or uninitialized, loading defaults."));
      Serial.println();
      
      //Set defaults, C000, 4000, and ASSIST09.BIN see const.h
      
      biosStart = DEFAULT_BIOS_START;
      biosSize  = DEFAULT_BIOS_SIZE;
     
      strncpy(biosName, DEFAULT_BIOS_NAME, MAX_FN_LENGTH);
      biosName[MAX_FN_LENGTH - 1] = '\0'; // extra safe!
      
      updateEEPROM(); // rebuild the eeprom and the checksum
  }
  
 
  Serial.printf("vBIOS: Mounting %s volume...",biosName);
  imgFile = SD.open(biosName, FILE_READ);
  imgFile.seek(0);
  if (!imgFile) {
    Serial.println(F("Can not find System ROM File on SD ... Halt!"));
    while(1);  // halt.
  }
 
}else {
  // we couldn't find the ram chip! 
  // something is wrong
  Serial.println(F("RAM issue detected at 0xFFFF Could not read back Sanity check!"));
  Serial.println(F("Halt!"));
 
  // place debug code here system is in tri-state with the 
  // controller in control of the top of the address space
  
  
  while(1); 
}

Serial.println();
Serial.println(F("Switching Bus Mastering to HD63C09..."));
// run state 
write_a_nibble(0);      
ddr_a_nibble(READ_MODE); // address bus should be tri-stated
bitClear(DDRD, IOREQ_);
bitClear(PORTB, R_W);
bitClear(DDRB, R_W);
bitClear(DDRD, XSIN_);


Serial.println(F("HD63C09 is now on the bus..."));

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
  
  // set UARTSTAT Register.
  ustatus = 0;  // clear status register 

  // serial interupts, the AVR has 64 byte fifo buffer, making the buffering transparent 
  // when the user uses it in 6850 wrapper mode. for most use cases, these buffers are 
  // large enough to operate without flow control when using fully interupt controlled 
  // circular buffers of large enough size. such as defined in Dominic's BBC Basic port.
  
  //RDRF 
  if (Serial.available() > 0) {
    bitSet(ustatus, 0);
    // set an interupt if the recieve data register has data and interupts are enabled
    if(rxIntEn) {
      bitSet(intRegister, 0); // (interupt is from RDRF)
      bitSet(ustatus,7); // status interupt bit
    }
    else bitClear(intRegister,0);
  }

  //TDRE
  if (Serial.availableForWrite() > 0) {
      bitSet(ustatus, 1);
      // set an interupt if the Transmit data register has space and can send data
      if(txIntEn) {
        bitSet(intRegister, 1); // (interupt is from TRDE)
        bitSet(ustatus,7); // status interupt bit. 
      }
      else bitClear(intRegister,1);
  }
  //systic simulator -- this is Fabio's method for building a interval timer. 
  //Quick simple and efective. this was lifted almsot verbatem from Z80-MBC Code.
  //See atribution. 
  
  if(tmIntEn) {
    if ((micros() - uSStamp) > ((long unsigned) (sTickTime) * 1000)) {
      bitSet(intRegister, 2);   // interupt set the status bit
      bitSet(tStatus, 0);
      uSStamp = micros();       // seed the systick timer
    }
  } else {        // just clear the int status bit
    bitClear(tStatus, 0);
  }
  
  //interupt handler 
  intHandler();  // checks to see if the intRegister is clear '0' and if so clears the interupt.
  
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
        *   0xA000    - UARTCNT    - Legacy 6850 UART Control register.
        *   0xA001    - TXSERIAL   - 6850 Wrapper: Send a byte to TX Buffer (default buffer size is 64bytes)
        *   0xA006    - FLPSEL     - Select Soft Sectored Floppy
        *   0xA007    - FLPTRK     - Select Floppy Track
        *   0xA008    - FLPSEC     - Select Floppy Sector
        *   0xA009    - FLPWRI     - Write a 256 Byte sector
        *   ...
        *   0xA03B    - TIMRCNT    - Timer Control Register
        *   0xA03B    - SETTICK*   - Set the system Tick Timer in msec, default is 10 msec.  
        *   0xA03E    - LOADERR    - This is the loader register, it is unlocked by writing 255 after boot up.
        *   0xA03F    - SETBANK    - Write the low nibble (3 bits) on the data bus to the bank register
        *  
        *  READ OPERATIONS:  
        *   0xA000    - UARTSTAT   - 6850 Wrapper: Send data back as if it were a 6850 Status regeister.(SEE ACCEPTIONS IN CASE)
        *   0xA001    - RXSERIAL   - 6850 Wrapper: Read a byte from the RX buffer (defalut buffer size is 64bytes)
        *   0xA006    - DRVREG     - Returns the selected Drive number
        *   0xA007    - TRKREG     - Returns the selected Track Number
        *   0xA008    - SECREG     - Returns the selected Sector Number (will be offset by -1 in flex)
        *   0xA009    - FLPREA     - Read a 256 byte sector
        *   0xA00A    - FLPSTA     - Read the Controler Error Codes. 
        *   ...
        *   0xA03B    - TIMRSTA    - Timer Status Register
        *   0xA03C    - SAMPTICK    - Sample timer while running
        *   0xA03D    - SYSTAT     - Read System status register  
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
              // store the control req register bits for use by the wrapper   
              txIntEn = (busData & 0x20) && !(busData & 0x40); // CR5 = 1 and CR6 = 0
              rxIntEn = (busData & 0x80);                      // CR7 = 1

              break;
             
             case 0x01:
              // send a byte to the terminal.
              // TX SERIAL 
              
              Serial.write(busData);
              bitClear(intRegister, 1);
              intHandler();

              break;
               
             case 0x06:
             /* NEW SOFT SECTOR FLOPPY COMMANDS 
              * FLPSEL - Select the floppy drive
              * 
              * Has to be 0 - 3, each is a image file in the directory of the boot rom 
              * calld FLPY00.DSK - FLPY03.DSK.  They are all 80 track 20 sector's.
              * 
              * will default to the last disk selected if it errors.  error codes are listed
              * below in the read status and mimic the WD floppy controler.
              * 
              */
              flpErr = 0;
              if (busData <= 3)                            // Valid disk number
               // Set the name of the file to open as virtual disk, and open it
               {
                 drvNumb = busData;
                 imgFile.close();                          // asure that any file left open accidentally is closed. 
                 
                 flpName[4] = (busData / 10) + 48;         // Set the disk number
                 flpName[5] = busData - ((busData / 10) * 10) + 48;
                 
                 imgFile = SD.open(flpName,FILE_WRITE);    // Open the "disk file" corresponding to the given disk number
                
                 if (!imgFile) {
                  
                  bitSet(flpErr,7);           // disk not ready
                
                 }
                 
               }
               else {
                
                bitSet(flpErr,4) ;                     // Disk does not exist.
               
               }
              
              break; // internal case
           
             case 0x07:
             /*NEW SOFT SECTOR FOPPLY COMMANDS
              *FLPTRK - Select the floppy track
              *
              *Has to be 0 - 79, if this value is greater it will return a seek error.
              * 
              */
              flpErr = 0;  // flush flpErr
              if (busData <= 79) {        // valid track number 0-79
                flpTrack = busData;   
               }
               else { 
                
                bitSet(flpErr,4);       // seek error
        
               }
            
              break;

             case 0x08:
             /* NEW SOFT SECTOR FLOPPY COMMANDS 
              * FLPSEC - Select the floppy Sector
              * 
              * has to be a value 0-19, keep in mind flex tracks this as 1-20 so you will have to
              * agment the value in your drivers by 1 to correct the sector numbering so that it
              * matches.
              * 
              *if the value is greater it will pass an error
              */
               flpErr = 0;  // flush flpErr
               if (busData <= 19) {  // valid track number 0-19 ie (1-20 inside flex)
                flpSector = busData;               
               }
               else {
                
              
                bitSet(flpErr,4);    // seek error
                
               }
               
               break;
              
              case 0x09:
              /*
               * FLPWRI - Write the selected sector (soft sector floppy emulation)
               *
               * Usage: 
               *  You must write to 0xA009 256 times in a row for the operation to complete. 
               *  Not writing all the data will cause the SD card to not properly update,
               *  which could result in data corruption. Disable interrupts on the host CPU side 
               *  to avoid causing a reset in the multi-op logic. 
               *
               *
               * Design Notes:
               *  - This routine writes one byte at a time directly to the SD card file.
               *  - Every 32 bytes, a flush() is called to commit data, minimizing internal SD buffering.
               *  - No RAM buffering is used — this saves memory on the ATmega32 (only a few bytes used total).
               *  - This approach is slower than bulk writes, but still orders of magnitude faster than a real floppy drive.
               *  - Priority is reliability and minimal RAM use, not maximum write speed.
               *  - Flushing every 32 bytes reduces SD card wear and ensures data integrity in case of power loss.
               *
               * Floppy seek emulation is maintained by calculating the correct file offset
               * based on track and sector numbers (logical LBA formula).
               */


               if (!ioByteCnt) {
                // First byte of transfer, calculate offset and seek.
                if ((flpTrack < 80) && (flpSector < 20) && (!flpErr)) {
                  if (!(imgFile.seek((flpTrack * 20UL + flpSector) * 256UL))) {
                    bitSet(flpErr, 4); // seek error
                   
                  }
                  
                }
               }

               if (!flpErr) {
                tempByte = ioByteCnt % 32; // 0..31 it should complete 8 flushes in 256 bytes, and flush on the last byte.
                imgFile.write(busData);
                if (tempByte == 31) {
                  imgFile.flush();   // ensure data actually written, keep internal buffer small.
                }

               if (ioByteCnt >= 255) {
                if (tempByte < 31) {
                 
                  bitSet(flpErr,2); // unexpected end of write operation. 
                 
                }
                  curOp = 255; // mark operation finished, resets multi-op cleanly
                }
               }

              ioByteCnt++; // count bytes transferred
              break;

             
             case 0x3B:  
             /* TIMRCNT --  
              * TIMER CONTROL REGISTER (TCR):  
              * Controls the timer operation, including starting, stopping, resetting, and enabling interrupts.  
              *  
              *               I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0  
              *                          ---------------------------------------------------------  
              *                          
              *                                       ...            D1 - D7 are future use, write 0's  
              *                            
              *                            X  X  X  X  X  X  X  0    Timer interrupts disabled  
              *                            X  X  X  X  X  X  X  1    Timer interrupts enabled  
              *   
              *  
              *                        
              *  
              * DESCRIPTION:  
              * - Bit 0: Controls interupts, (1) enables interputs or (0) disables interupts   
              */

              // Set the control register to the bus data
              tControl = busData;

              // Enable or disable timer interrupts
              tmIntEn = bitRead(tControl, 0);

             break;
            
           
             case 0x3C:
             // Set Sys tick timer 
             // SETTICK - set the Systick timer time (milliseconds)
             //
             //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
             //                            ---------------------------------------------------------
             //                             D7 D6 D5 D4 D3 D2 D1 D0    Systick time (binary) [1..255]
             //
             // Set/change the time (millisecond) used for the Systick timer.
             // At reset time the default value is 10ms.
             // See SETIRQ and SYSIRQ Opcodes for more info.
             //
             // NOTE: If the time is 0 milliseconds the set operation is ignored.
  
             if (busData >0 ) sTickTime = busData;
             
             break;
             
             case 0x3E:
              // LOADERR --
              /*
               *   This performs some low level operations (high risk!!) understand how these work before using
               *  +---------------+---------------------------------------------------------+
               *  | write byte:   |   Operation:                                            |
               *  +---------------+---------------------------------------------------------+
               *  |     01        |  Resets only the 6309, this will not reset the AVR      |
               *  +---------------+---------------------------------------------------------+
               *  |     02        |  halts the 6309, remains halted until next sys_reset    |
               *  +---------------+---------------------------------------------------------+
               *  |               |  Unlocks the Loader enviornment for the above, or to    |
               *  |     FF        | stage the computer. (see read operation 0x03E)          |
               *  +---------------+---------------------------------------------------------+
               * 
               */
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
               // bit 7 = irq
               
               // buffers are manually set to 64 bytes, testing has shown that there is little chance of an underun / overflow 
               // set at this level. /DCD, /CTS and FE are depreciated.

               // TODO - write code for bits 5 

               busData = ustatus;  // output the current status of the register to the bus
              
               break; // end of read uart control register
              
              case 0x01:
               //RXSERIAL
               //this is reading a byte from the terminal. so we need to write the next byte in
               //the uart to the bus.

               busData = Serial.read();
               bitClear(intRegister, 0); // clear the interupt bit
               intHandler();
               
               break; // end of read uart 

             case 0x06:
              /*NEW SOFT SECTOR FLOPPY ROUTINES
               *DRVREG - Floppy Register 
               *
               *Returns the currently selected drive
               *
               */
               
   
              busData = drvNumb;

              break; 
              
               
             case 0x07:
              /* NEW SOFT SECTOR FLOPPY ROUTINES
               * TRKREG - Track Register
               * 
               * Returns the currently selected track
               */
              busData = flpTrack;
              
              break; 

             case 0x08:
              /* NEW SOFT SECTOR FLOPPY ROUTINES 
               * SECREG - Sector Register
               * 
               * Returns the currently selected sector
               */
              busData = flpSector;

              break;
             
               
            case 0x09:
             /*
              * FLPREA - Read a sector from the floppy image
              *
              * Reads exactly 256 bytes sequentially without interruption.
              * Host-side interrupts must be disabled during this operation to prevent
              * partial reads, which would cause errors and data corruption.
              *
              * Operation:
              *  - On the first byte, calculate the logical offset from track and sector, then seek.
              *  - Read one byte at a time from the SD file.
              *  - If a read error (EOF or other) occurs, set the appropriate error bit.
              *  - Continue until 256 bytes have been read, then signal the operation complete.
              */

                // seek to proper location in file selected by last seek
                if (!ioByteCnt) {
                // First byte of transfer, calculate offset and seek.
                 if ((flpTrack < 80) && (flpSector < 20) && (!flpErr)) {
                   if (!(imgFile.seek((flpTrack * 20UL + flpSector) * 256UL))) {
                     bitSet(flpErr, 4); // seek error
                     
                   }
                  
                 }
                }
                
                // check for errors from seek and continue if none
                if (!flpErr) {

                    //read with enough resolution to see the errors from valid bits 
                    //diskByte is type int16_t

                    diskByte = imgFile.read();
                    busData = (uint8_t) lowByte(diskByte); // strip the MSB 

                    // check for errors 
                    if (diskByte == -1) {
                          // we hit a EOF or there is an error need to return error code 
                          bitSet(flpErr, 3);  // CRC error -- we reached an unexpected end to data  
                        
                    }
                          
                }
                if (ioByteCnt >= 255) {
                  curOp = 255; // reset last op this value makes certain that the multi io logic resets cleanly.
                }
              
                ioByteCnt++;                          // Increment the counter of the exchanged data bytes
                 
            break;
              

            case 0x0A:
             /*NEW SOFT SECTOR FLOPPY ROUTINES
              * FLPSTA - Status register, returns a WD like drive error code           
              * 
              * BIT         Status      
              *  7         Not Ready   
              *  6            0  
              *  5            0
              *  4  Seek Error / Not Found
              *  3         CRC Error 
              *  2        Lost Data 
              *  1            0     
              *  0            0
              */
               busData = flpErr;

             break;
            

              case 0x3B:
               /* TIMRSTA
                * TIMER STATUS REGISTER (TSR):  
                * Shows the timer operation, for interupts, reads / writes and run status  
                *  
                *               I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0  
                *                          ---------------------------------------------------------  
                *                          
                *                                     ....             bits D1 - D7 are Future use
                *                          
                *                            X  X  X  X  X  X  X  0    Timer interrupt cleared (high)  
                *                            X  X  X  X  X  X  X  1    Timer interrupt active (low)  
                *  
                *                                     
                *  
                *                           
                * DESCRIPTION:  
                * - Bit 0: shows when an interupt is active, resets interupt flag when read and clears interupt
                *
                */
              
                busData = tStatus;
                bitClear(tStatus, 0); // clear interupt status bit, when checked
                bitClear(intRegister, 2); // clear the interupt
                intHandler();

              break;

              
              case 0x3C:
              // SAMPTICK
              // sample the systick timer, may be useful for retry counter, psudonumber generation etc... 
              // will be a number from 0 - sTickTime. 
              
              busData = (unsigned byte) (micros() - uSStamp) / 1000;  
               
               
              break;
              case 0x3D:
               /* SYSTAT
                * System status register
                * Shows the status of the internal register sets
                * 
                *               I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0  
                *                          ---------------------------------------------------------  
                *                          
                *                                     ....             bits D3 - D7 are Future use
                * 

                *                            X  X  X  X  X  0  X  X    TX interupt unset          
                *                            X  X  X  X  X  1  X  X    TX interupt set          
                *                            X  X  X  X  X  X  0  X    RX interupt unset 
                *                            X  X  X  X  X  X  1  X    RX interupt set
                *                            X  X  X  X  X  X  X  0    Timer interrupt unset  
                *                            X  X  X  X  X  X  X  1    Timer interrupts set  
                * 
                *  
                *                                     
                */
                
                busData = 0;   // reset bus data
                if (txIntEn) bitSet(busData,2);
                if (rxIntEn) bitSet(busData,1);
                if (tmIntEn) bitSet(busData,0);

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
                      busData = highByte(biosSize+1);

                      break;

                    case 0x03:
                      // send the number of bytes to read LSB
                      busData = lowByte(biosSize+1);

                    break;
                    
                    default:
                      // send the data                          
                      // No previous error from this point would have crashed at boot time.

                      // read with enough resolution to see the errors from valid bits diskByte
                      // is type int16_t. 
                      
                      //   0xFFFF = -1  = ERROR!
                      //   0x00XX = (an 8 bit value)

                      diskByte = imgFile.read();   

                      // check for EOF or errors
                      if (diskByte != -1)
                      
                          // if none mask off the diskByte to keep just the LSB (stored data)
                           
                          busData = (uint8_t) lowByte(diskByte);
                          
                      else {
                            // we hit the EOF or its an error.
                            // we are done loading the data and must reset or the computer will hang.
                            loaderReg = 0; // re-lock the loader
                            curOp = 255;   // reset last op (this value makes certain a new operation is initiated.)
                            imgFile.close();  // close the file so we can open the floppy image.
                            imgFile = SD.open(flpName,FILE_WRITE); // open floppy zero for random access. 
                            
                            Serial.println(F("LOADER: Finished bootstrap"));
                            bitSet(DDRB, RES_);
                            bitClear(DDRB, RES_); // reset
                          }
                     
                     break; 
                    } 
                  }
              ioByteCnt++;    
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




// handle interupts
void intHandler(void) {
  if (intRegister == 0) bitClear(DDRB, IRQ_); // if int Register is clear then clear the interupt
  else bitSet(DDRB, IRQ_); // else set the interupt.
  
}
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
      cli();  // we need to disable the interupt register for 2 timing sensitive bit flips 
      // send bgnt_
      bitClear(PORTD, IOGNT_);    // this ends the io request - cpu is free running from this point 
      bitSet(PORTD, IOGNT_);      // the current cycle is ended - we need to restore this as the address bus is building
      busTstate(); // this just makes sure we are ready to read on the next go-through.
      sei();  // back to it
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
    for (int i = 0; i < MAX_FN_LENGTH; i++) {               
        EEPROM.write(BIOS_NAME_ADDR + i, biosName[i]);
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

// Function to show the EEPROM variables
void showVariables() {
    for (int i = 0; i < NUM_VARIABLES; i++) {
        Serial.printf(" %-7s: ", variables[i].name);
        
        // Cast the pointer to the appropriate type before dereferencing
        if (strcmp(variables[i].formatSpecifier, "%s") == 0) {
            // If the format specifier is "%s", cast to char* and print as a string
            Serial.printf(variables[i].formatSpecifier, reinterpret_cast<char*>(variables[i].ptr));
        } else {
            // Otherwise, assume it's an integer and cast to uint16_t*
            Serial.printf(variables[i].formatSpecifier, *reinterpret_cast<uint16_t*>(variables[i].ptr));
        }
        
        Serial.println();
    }
}

void readFilenameInput(char* dest, size_t maxLength, const char* defaultName) {
  char buffer[MAX_FN_LENGTH];
  size_t pos = 0;

  while (true) {
    if (Serial.available()) {
      char c = Serial.read();

      if (c == '\r' || c == '\n') {
        break; // Done typing
      } else if (c == 8 || c == 127) {
        if (pos > 0) {
          pos--;
          Serial.print("\b \b");
        }
      } else if (isPrintable(c) && pos < (maxLength - 1)) {
        c = toupper(c);
        buffer[pos++] = c;
        Serial.print(c);
      }
      // Ignore anything else
    }
  }

  buffer[pos] = '\0'; // Null terminate

  if (pos == 0) {
    // Nothing entered, use default
    Serial.print(defaultName);
    strncpy(dest, defaultName, maxLength);
  } else {
    strncpy(dest, buffer, maxLength);
  }

  dest[maxLength - 1] = '\0'; // extra safe
}


uint16_t readHexInput(uint16_t defaultValue) {
  char buffer[10];
  size_t pos = 0;

  while (true) {
    if (Serial.available()) {
      char c = Serial.read();

      if (c == '\r' || c == '\n') {
        break; // Done typing
      } else if (c == 8 || c == 127) {
        if (pos > 0) {
          pos--;
          Serial.print("\b \b");
        }
      } else if (isHexadecimalDigit(c) && pos < (sizeof(buffer) - 1)) {
        c = toupper(c);
        buffer[pos++] = c;
        Serial.print(c);
      } else if ((c == 'x' || c == 'X') && pos == 1 && buffer[0] == '0') {
        buffer[pos++] = c;
        Serial.print(c);
      }
      // Ignore anything else
    }
  }

  buffer[pos] = '\0'; // Null terminate

  if (pos == 0) {
    // Nothing entered, use default
    Serial.print(defaultValue, HEX);
    return defaultValue;
  } else {
    // Parse typed input
    return (uint16_t) strtol(buffer, NULL, 16);
  }
}


void waitKey()
// Wait a key to continue
{
  while (Serial.available() > 0) Serial.read();   // Flush serial RX buffer
  Serial.println(F("vBIOS: Check SD and press a key to repeat\r\n"));
  while(Serial.available() < 1);
}
