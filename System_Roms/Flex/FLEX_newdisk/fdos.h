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

asm fputChar(char c);

// Inputs a line from the keyboard into the Line Buffer.
asm finBuf(void);

// outputs a string literal to flex honnoring TTYSET parameters.
// does not support printf style formatting, but does support
// c control characters such as \n and \r. does no formatting, 
// just outputs the string as is with no trailing line feed or carrage 

asm print(const char* str);

// outputs a string to the terminal to flex honnoring TTYSET parameters.
// passes a lf/cr to the end of the string, all the same as the print function.

asm println(const char* str);

// pcrlf - Outputs a carriage return and line feed to the terminal.
asm pcrlf(void);

// var contains the address of the variable to output as hex.
asm foutHex(const void* var);



#endif /* FDOS_H */