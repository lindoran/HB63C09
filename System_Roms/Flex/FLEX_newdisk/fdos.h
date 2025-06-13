#ifndef FDOS_H
#define FDOS_H
// fdos.h
// This file is part of the FLEX operating system project.
// It provides definitions and functions for handling FLEX strings
// and interfacing with the FLEX operating system.

// This contains addtional routines not provided by bflex, such as
// the very basic driver access routines, these routines are higher
// level and provide more functionality than that of the bios routines.


#include "tinytypes.h"

//some functions have usage that requires returning large amounts of data
//this structure allows us to have a psudo register for returning these values on the
//last operation.  this is not exclusive and is only for some of the operations where
//it is usefull.

//updated by: 
//fgetNext()  --  carry
//fgetHex()   --  b_reg, x_reg, and carry 
//fgetDec()   --  b_reg, x_reg, and carry 

typedef struct {
    uint8_t  reg_b;      //accumulator b (lsb of d)
    bool     carry;      //CC register cary bit
    uint16_t reg_x;      //index register x
} flex_lastop_t;            

extern flex_lastop_t flex_lastop;

// Outputs a character to the terminal or file.

void fputChar(char c);

// Inputs a line from the keyboard into the Line Buffer.
void finBuffer(void);

// outputs a string literal to flex honnoring TTYSET parameters.
// does not support printf style formatting, but does support
// c control characters such as \n and \r. does no formatting, 
// just outputs the string as is with no trailing line feed or carrage 

void print(const char* str);


// pcrlf - Outputs a carriage return and line feed to the terminal.
void pcrlf(void);

// get a character from the input buffer and update flex dos space.
char fgetNext(void);

// restore io vectors
void frestoreIO (void);

// &var contains the address of the variable to output as unsigned decimal
// spaces contains TRUE or FALSE weather or not to substitute trailing zeros
// for spaces. 
void foutDecimal(const void* var, bool spaces);

// &var contains the address of the variable to output as hex.
void foutHex(const void* var);

// returns a 16 bit value from the linebuffer, expects the pointer to be pointing at the start 
// of a 16 bit number in hex (up to 4 digits), returns more information in the psudoregister.
// see fdos.c for more info.
uint16_t fgetHex(void);


// &var contatins the address of the variable to output as hex.
void foutLongHex(const void* var);

// returns a 16 bit vaue from the linebuffer, expects the pointer to be pointing at the start 
// of a 16 bit number in decimal (up to 16 bits of presision 0..65535) Returns more information
// in the psudoregister. 
// see fdos.c for more info.

uint16_t fgetDec(void);

// sets a string literal in the line buffer and terminates properly for use with flex.
void setLineBuffer(const char* s);

// get a token from the linebuffer, or next token.
void getLineToken(char* dst);

#endif /* FDOS_H */