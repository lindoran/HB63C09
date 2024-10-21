// const.h
// constant defs for HB63C09M

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


#ifndef CONST_H
#define CONST_H

// AVR DATABUS - NOT USED - JUST FOR REFRENCE -
const uint8_t D0      = 0; // PA0 (pin 40)
const uint8_t D1      = 1; // PA1 (pin 39)
const uint8_t D2      = 2; // PA2 (pin 38)
const uint8_t D3      = 3; // PA3 (pin 37)
const uint8_t D4      = 4; // PA4 (pin 36)
const uint8_t D5      = 5; // PA5 (pin 35)
const uint8_t D6      = 6; // PA6 (pin 34)
const uint8_t D7      = 7; // PA7 (pin 33)

// PORT B
const uint8_t RES_    = 0; // PB0 (pin 1) - 6309 ~reset line
const uint8_t HALT_   = 1; // PB1 (pin 2) - 6309 ~Halt line
const uint8_t R_W     = 2; // PB2 (pin 3) - 6309 read/~write line
const uint8_t IRQ_    = 3; // PB3 (pin 4) - 6309 ~irq
const uint8_t SPISS_  = 4; // PB4 (pin 5) - sd SPI
const uint8_t SPIMOSI = 5; // PB5 (pin 6) - sd SPI
const uint8_t SPIMISO = 6; // PB6 (pin 7) - sd SPI
const uint8_t SPISCK  = 7; // PB7 (pin 8) - sd SPI

// PORT C
const uint8_t SCL_PC0 = 0; // PC0 (pin 22) - i2c signals
const uint8_t SDA_PC1 = 1; // PC1 (pin 23) - i2c signals
const uint8_t MCUA0   = 2; // PC2 (pin 24) - 6309 A0
const uint8_t MCUA1   = 3; // PC3 (pin 25) - 6309 A1
const uint8_t MCUA2   = 4; // PC4 (pin 26) - 6309 A2
const uint8_t MCUA3   = 5; // PC5 (pin 27) - 6309 A3
const uint8_t MCUA4   = 6; // PC6 (pin 28) - 6309 A4
const uint8_t MCUA5   = 7; // PC7 (pin 29) - 6309 A5

// PORT D
const uint8_t RX      = 0; // PD0 (pin 14) - RX PIN
const uint8_t TX      = 1; // PD1 (pin 15) - TX PIN
const uint8_t WR_     = 2; // PD2 (pin 16) - RAM WR_ strobe
const uint8_t RD_     = 3; // PD3 (pin 17) - RAM RD_ strobe
const uint8_t BCLK    = 4; // PD4 (pin 18) - bank address clock pin
const uint8_t XSIN_   = 5; // PD5 (pin 19) - bus transceiver inhibit bar line
const uint8_t IOREQ_  = 6; // PD6 (pin 20) - io request bar line
const uint8_t IOGNT_  = 7; // PD7 (pin 21) - io grant bar line

// IO control addressing
const uint8_t NIBBLE_BITS  =   6; // Number of bits in the nibble
const uint8_t ADDRESS_MASK =  63; // Un-shifted nibble mask as it appears in the curOp variable
const uint8_t NIBBLE_MASK  = 252; // Shifted nibble mask as it appears on the C port

// Pulls the I/O address off PORTC as a 6-bit nibble
#define A_NIBBLE ((uint8_t)((PINC & NIBBLE_MASK) >> 2))

// MAX CHARACTERS IN A FILENAME WITH NULL  (petitFS uses this legnth)
#define MAX_FN_LENGTH 13  
#define MAX_PT_LENGTH ((MAX_FN_LENGTH *2)+2)

// ASCII ESC for vCMOS
#define ESC_KEY 27


// Define default bootstrap values
const uint16_t DEFAULT_BIOS_START = 0xC000;
const uint16_t DEFAULT_BIOS_SIZE = 0x4000;
const char DEFAULT_BIOS_NAME[MAX_FN_LENGTH] = "BIOS.BIN";

// Define EEPROM addresses for each variable
// Define EEPROM addresses for each variable
//  0 BIOS START ADDRESS (2 BYTES)
//  4 BIOS SIZE (2 BYTES)
//  8 BIOS NAME (13 BYTES)
// 21 BIOS PATH (13 BYTES)
 
const int BIOS_START_ADDR = 0;
const int BIOS_SIZE_ADDR = sizeof(uint16_t);                  // normally 2
const int BIOS_NAME_ADDR = BIOS_SIZE_ADDR + sizeof(uint16_t); // normally 4
const int BIOS_PATH_ADDR = BIOS_NAME_ADDR + MAX_FN_LENGTH;    // Assuming 13 bytes for biosName
const int CHECKSUM_ADDR = BIOS_PATH_ADDR + MAX_FN_LENGTH;     // Assuming 13 bytes for biosPath




#endif /* VCMOS_H */
