#include "tinytypes.h"
#include "fdos.h"

#define PUTCHR  0xCD18   // putchr routine as defined in the advanced FLEX programmers guide
#define INBUF   0xCD1B   // inbuf routine as defined in the advanced FLEX programmers guide
#define PSTRNG  0xCD1E   // pstring routine as defined in the advanced FLEX programmers guide
#define OUTHEX  0xCD3C   // outhex routine as defined in the advanced FLEX programmers guide
#define PCRLF   0xCD24   // pcrlf routine as defined in the advanced FLEX programmers guide

// fputChar (character to output) - Outputs a character to the terminal or file.
// Outputs a character using the FLEX PUTCHR routine ($CD18).
// - The character is passed in register A.
// - Honors all TTYSET parameters (e.g., line length, escape key).
// - If the "Special I/O Flag" is zero, manages column count and newlines.
// - If using ACIA, checks for TTYSET Escape Character and pauses output if typed.
// - If "Output Switch" is non-zero, sends the character via OUTCH2.
// - If "File Output Address" is non-zero, writes the character to the specified file (via FCB).
// - Otherwise, sends the character to the terminal using OUTCH (which can be redirected).
// - Registers X and B are preserved.
// note that we are pulling from 3,S this is because cmoc converts char into uint16_t 
// when it pushes to the stack, so we need to pull from the lsb of the 16 bit value 
// stored at 2,S.  OR we could have pulled 2,s into D and then Transfered B to A, but this is simpler.

asm fputChar(char c) {
    asm {
        LDA 3,S         // Load the character to output into register A
        JSR PUTCHR      // Call the putchr routine
    }            
}

// Calls the FLEX INBUFF routine ($CD1B) to input a line from the keyboard into the Line Buffer.
// - Handles TTYSET Backspace and Delete characters.
// - Ignores all control characters except RETURN and LINE FEED.
// - RETURN is stored at the end of the buffer; LINE FEED is stored as a space but echoed as CR/LF.
// - Maximum 128 characters (including RETURN) are accepted; excess input is discarded.
// - On exit, the Line Buffer Pointer points to the start of the buffer.
// Caution: Using INBUFF will overwrite the command line buffer, which may disrupt DOS command processing.

asm finBuf(void) {
    asm {
        JSR INBUF       ; // Call the inbuf routine
    }
}

// flex strings are very antiquated, whats worse they don't behave like C strings.
// they use a 04 (EOT) character to terminate the string, and they do not use a null terminator.
// in order to use c strings with flex, we need to use a set up that allows us to make use of string
// literals, but also allows us to honor TTYSET parameters. -- this is the most basic way to do this.

// buiding a framework to support flex strings as is is difficult and requires a lot of space.  
// using c strings is the best solution as it allows the compiler to optimize the code better 
// it allows for more natural use of strings the down side here is that we have to use a seprate 
// set of functions to handle binary to integer and hex converstion.  as cmoc doese not support 
// type detection within parameters like C11 this is what makes printf so BIG in CMOC.    

// outputs a string literal to flex honnoring TTYSET parameters.
// does not support printf style formatting, but does support
// c control characters such as \n and \r. does no formatting, 
// just outputs the string as is with no trailing line feed or carrage 
// absolutly not thread safe, don't call from an interrupt while running.
// this just runs untill its hit the null so its not memory safe either. 
// if you pass any old character buffer its just going to run until it hits a null.

asm print (const char *s) {
    asm {
        LDX 2,s      ; load the address of the string to output
    @loop:
        LDA ,x+      ; load the first character of the string
        JSR PUTCHR   ; call the putchr routine to output the character
        BNE @loop    ; the character is not null, we keep going
        
    }
}

// outputs a string to the terminal to flex honnoring TTYSET parameters.
// passes a lf/cr to the end of the string, all the same as the print function.

asm println(const char *s) {
    asm {
        LDX 2,s        ; // Load the address of the string to output
    @loop:
        LDA ,x+        ; // Load the next character from the string
        JSR PUTCHR
        BNE @loop      ; // If the character is not null, continue looping
        JSR PCRLF      ; // Call the pcrlf routine to output a CR/LF
    }

}

// pcrlf - Outputs a carriage return and line feed to the terminal.

asm pcrlf(void) {
    asm {
        JSR PCRLF       // Call the pcrlf routine to output a CR/LF
    }
}

// foutHex (var) - Outputs the contents of an 8 bit variable as hexadecimal digits.
// void type is used to minimize casting in this case, generally this expects a 
// pointer to char or uint8_t, but can be used with any 8 bit variable. if used 
// with a larger variable, only the first byte will be output as hex. 


asm foutHex(const void* var) {
    asm {
        LDX 2,s        // Load the address of the variable to output
        JSR OUTHEX      // Call the outhex routine
    }


}



