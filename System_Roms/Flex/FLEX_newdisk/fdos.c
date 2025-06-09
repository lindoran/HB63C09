#include "tinytypes.h"
#include "fdos.h"

#define PUTCHR  0xCD18   // FLEX: Output a character to terminal or file
#define INBUF   0xCD1B   // FLEX: Input a line from keyboard into line buffer
#define CLASS   0xCD21   // FLEX: Classify character as alphanumeric or not
#define PCRLF   0xCD24   // FLEX: Output carriage return and line feed
#define NXTCH   0xCD27   // FLEX: Get next character from line buffer (skips spaces, handles EOL)
#define RSTRIO  0xCD2A   // FLEX: Restore IO vectors for OUTCH, INCH, and File IO addresses to defalut
#define OUTHEX  0xCD3C   // FLEX: Output an 8-bit value as hexadecimal

#define NL      0x0A     // new line
#define CR      0x0D     // carrage return

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

asm void fputChar(char c) {
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

asm void finBuffer(void) {
    asm {
        JSR INBUF       ; // Call the inbuf routine
        LDA #NL         ; // we need to do a new line as we just echoed return
        JSR PUTCHR      ; // lets put the new line in.
    }
}

// FLEX strings are very antiquated; what's worse, they don't behave like C strings.
// They use a 0x04 (EOT) character to terminate the string, and they do not use a null terminator.
// In order to use C strings with FLEX, we need to use a setup that allows us to make use of string
// literals, but also allows us to honor TTYSET parameters. -- This is the most basic way to do this.

// Building a framework to support FLEX strings as-is is difficult and requires a lot of space.
// Using C strings is the best solution as it allows the compiler to optimize the code better.
// It allows for more natural use of strings; the downside here is that we have to use a separate
// set of functions to handle binary to integer and hex conversion. As CMOC does not support
// type detection within parameters like C11, this is what makes printf so BIG in CMOC.

// print (string)

// Outputs a string literal to FLEX honoring TTYSET parameters.
// Does not support printf-style formatting, but does support
// C control characters such as \n. Just outputs the string as is with
// no trailing line feed or carriage return unless you pass \n; it mimics
// printf and passes CR as well.

// Absolutely not thread safe, don't call from an interrupt while running.
// This just runs until it's hit the null so it's not memory safe either.
// If you pass any old character buffer it's just going to run until it hits a null.

asm void print (const char *s) {
    asm {
        LDX  2,s      ; load the address of the string to output
    p1: 
        LDA  ,x+      ; load the first character of the string
        CMPA #NL      ; if its a new line
        BNE  p2       ; we have to print carrage return if not we skip and go on
        LDA  #CR      ; some retro terminals really hate it the other way round
        JSR  PUTCHR   ; print the carrage return
        LDA  #NL      ; stage the new line
    p2: JSR  PUTCHR   ; call the putchr routine to output the character
        BNE  p1       ; the character is not null, we keep going

    }
}


// classify if character is a number or a letter and update system Last
// terminator character. (this specifically changes the dos enviornment)
// if a file is open or being read this could interfear with that.

asm bool fchrClass(char ch) {
    asm {
        CLRB        ; start with b as false ie not a letter / number
        LDA 3,s     ; get the lsb from 16 bit value from the stack
        JSR CLASS   ; determine if a letter or a number,if so carry is cleared 
        BCS cl      ; if c flag is set (this also updates the last terminator in the dos enviorntment)
        INCB        ; return true if its a character 
    cl:
    }
}

// pcrlf - Outputs a carriage return and line feed to the terminal.

asm void pcrlf(void) {
    asm {
        JSR PCRLF       // Call the pcrlf routine to output a CR/LF
    }
}

// fgetNext - Fetches the next character from the FLEX line buffer using the NXTCH routine.
//
// The NXTCH routine works as follows:
// - The character in the "Current Character" location is copied to "Previous Character".
// - The character pointed to by the Line Buffer Pointer is fetched from the Line Buffer and saved in "Current Character".
// - Multiple spaces are skipped, so a sequence of spaces is treated as a single space.
// - The Line Buffer Pointer is advanced to the next character, unless the fetched character is a RETURN (0x0D) or the TTYSET End-of-Line character.
//   - If RETURN or EOL is encountered, repeated calls to NXTCH will continue to return the same character.
// - NXTCH cannot cross into the next command in the buffer.
// - On exit, NXTCH calls the CLASS routine to classify the character (alphanumeric or not).
// - The fetched character is returned in register A (and B for CMOC).
// - The carry flag is clear if the character is alphanumeric, set otherwise.
// - Registers B and X are preserved.
//
// This function is typically used to sequentially read and parse command-line or file input from the FLEX system buffer.

asm char fgetNext (void) {
    asm {
        JSR NXTCH       ; get next character
        TFR A,B         ; place in B where cmoc expects return
    }
}

// Restore IO vectors (for completeness).
asm void frestoreIO (void) {
    asm {
        JSR RSTRIO      ; restore io vectors
    }
}


// foutHex (var) - Outputs the contents of an 8 bit variable as hexadecimal digits.
// void type is used to minimize casting in this case, generally this expects a 
// pointer to char or uint8_t, but can be used with any 8 bit variable. if used 
// with a larger variable, only the first byte will be output as hex. 


asm void foutHex(const void* var) {
    asm {
        LDX 2,s        // Load the address of the variable to output
        JSR OUTHEX     // Call the outhex routine
    }


}



