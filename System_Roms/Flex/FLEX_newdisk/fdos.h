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

// var contains the address of the variable to output as hex.
void foutHex(const void* var);



#endif /* FDOS_H */